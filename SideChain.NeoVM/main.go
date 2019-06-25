package main

import (
	"bytes"
	"os"
	"path/filepath"
	"runtime"
	"runtime/debug"
	"strconv"
	"time"

	"github.com/elastos/Elastos.ELA.SideChain/mempool"
	"github.com/elastos/Elastos.ELA.SideChain/pow"
	"github.com/elastos/Elastos.ELA.SideChain/service"
	"github.com/elastos/Elastos.ELA.SideChain/spv"
	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/events"
	"github.com/elastos/Elastos.ELA.SideChain/server"
	sw "github.com/elastos/Elastos.ELA.SideChain/service/websocket"

	"github.com/elastos/Elastos.ELA/utils/http/jsonrpc"
	"github.com/elastos/Elastos.ELA/utils/signal"
	"github.com/elastos/Elastos.ELA/utils/elalog"
	"github.com/elastos/Elastos.ELA/common"

	mp "github.com/elastos/Elastos.ELA.SideChain.NeoVM/mempool"
	sv "github.com/elastos/Elastos.ELA.SideChain.NeoVM/service"
	nc "github.com/elastos/Elastos.ELA.SideChain.NeoVM/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/store"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/event"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/datatype"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/service/websocket"
	ns "github.com/elastos/Elastos.ELA.SideChain.NeoVM/p2p/server"
	npow "github.com/elastos/Elastos.ELA.SideChain.NeoVM/pow"
)

const (
	printStateInterval = 10 * time.Second

	DataPath = "elastos_neo"
	DataDir  = "data"
	ChainDir = "chain"
	SpvDir   = "spv"
)

var (
	// Build version generated when build program.
	Version string

	// The go source code version at build.
	GoVersion string
)

func main() {
	// Use all processor cores.
	runtime.GOMAXPROCS(runtime.NumCPU())
	// Block and transaction processing can cause bursty allocations.  This
	// limits the garbage collector from excessively overallocating during
	// bursts.  This value was arrived at with the help of profiling live
	// usage.
	debug.SetGCPercent(10)

	eladlog.Infof("Node version: %s", Version)
	eladlog.Info(GoVersion)

	// listen interrupt signals.
	interrupt := signal.NewInterrupt()

	eladlog.Info("1. BlockChain init")
	chainStore, err := blockchain.NewChainStore(filepath.Join(DataPath, DataDir, ChainDir), activeNetParams.GenesisBlock)
	if err != nil {
		eladlog.Fatalf("open chain store failed, %s", err)
		os.Exit(1)
	}
	defer chainStore.Close()

	eladlog.Info("2. SPV module init")
	genesisHash := activeNetParams.GenesisBlock.Hash()
	programHash, err := mempool.GenesisToProgramHash(&genesisHash)
	if err != nil {
		eladlog.Fatalf("Genesis block hash to programHash failed, %s", err)
		os.Exit(1)
	}

	genesisAddress, err := programHash.ToAddress()
	if err != nil {
		eladlog.Fatalf("Genesis program hash to address failed, %s", err)
		os.Exit(1)
	}
	serviceCfg := spv.Config{
		DataDir:        filepath.Join(DataPath, DataDir, SpvDir),
		ChainParams:    spvNetParams,
		PermanentPeers: cfg.SPVPermanentPeers,
		GenesisAddress: genesisAddress,
	}
	spvService, err := spv.NewService(&serviceCfg)
	if err != nil {
		eladlog.Fatalf("SPV module initialize failed, %s", err)
		os.Exit(1)
	}

	defer spvService.Stop()
	spvService.Start()

	mempoolCfg := mempool.Config{
		ChainParams: activeNetParams,
		ChainStore:  chainStore,
		SpvService:  spvService,
	}
	txFeeHelper := mempool.NewFeeHelper(&mempoolCfg)
	mempoolCfg.FeeHelper = txFeeHelper

	txValidator := mp.NewValidator(&mempoolCfg)
	mempoolCfg.Validator = txValidator

	ledgerStore, err := store.NewLedgerStore(chainStore)
	if err != nil {
		eladlog.Fatalf("init DefaultLedgerStore failed, %s", err)
		os.Exit(1)
	}

	chainCfg := blockchain.Config{
		ChainParams: activeNetParams,
		ChainStore:  chainStore,
		GetTxFee:  txFeeHelper.GetTxFee,
		CheckTxSanity: txValidator.CheckTransactionSanity,
		CheckTxContext:txValidator.CheckTransactionContext,
		GetHeader: ledgerStore.GetHeader,
		GetBlock: ledgerStore.GetBlock,
	}

	chain, err := nc.NewBlockChain(&chainCfg, txValidator, ledgerStore)
	if err != nil {
		eladlog.Fatalf("BlockChain initialize failed, %s", err)
		os.Exit(1)
	}
	nc.DefaultChain = chain

	flag, err := ledgerStore.Get([]byte(store.AccountPersisFlag))
	if err != nil {
		batch := ledgerStore.NewBatch()
		ledgerStore.PersisAccount(batch, activeNetParams.GenesisBlock)
		batch.Commit()

		flag = []byte{1}
		ledgerStore.Put([]byte(store.AccountPersisFlag), flag)
	}

	sv.Store = ledgerStore
	sv.Table = store.NewCacheCodeTable(nc.NewDBCache(ledgerStore))

	txPool := mempool.New(&mempoolCfg)
	chainCfg.Validator = blockchain.NewValidator(chain.BlockChain, spvService)
	eladlog.Info("3. Start the P2P networks")
	server, err := ns.New(&server.Config{
		DataDir:        filepath.Join(DataPath, DataDir),
		Chain:          chain.BlockChain,
		TxMemPool:      txPool,
		ChainParams:    activeNetParams,
		PermanentPeers: cfg.PermanentPeers,})
	if err != nil {
		eladlog.Fatalf("initialize P2P networks failed, %s", err)
		os.Exit(1)
	}
	defer server.Stop()
	server.Start()

	eladlog.Info("4. --Initialize pow service")
	powCfg := &pow.Config{
		ChainParams:               activeNetParams,
		MinerAddr:                 cfg.PayToAddr,
		MinerInfo:                 cfg.MinerInfo,
		Server:                    server,
		Chain:                     chain.BlockChain,
		TxMemPool:                 txPool,
		TxFeeHelper:               txFeeHelper,
		CreateCoinBaseTx:          pow.CreateCoinBaseTx,
		GenerateBlockTransactions: pow.GenerateBlockTransactions,
		Validator:                 txValidator,
		GenerateBlock:             npow.GenerateBlock,
	}

	powService := pow.NewService(powCfg)
	if cfg.EnableMining {
		eladlog.Info("Start POW Services")
		go powService.Start()
	}

	eladlog.Info("5. --Start the RPC service")
	httpCfg := &sv.Config{
		Config: &service.Config{
			Server:                      server,
			Chain:                       chain.BlockChain,
			Store:                       ledgerStore.ChainStore,
			GenesisAddress:              genesisAddress,
			TxMemPool:                   txPool,
			PowService:                  powService,
			SpvService:                  spvService,
			SetLogLevel:                 setLogLevel,
			GetBlockInfo:                service.GetBlockInfo,
			GetTransactionInfo:          sv.GetTransactionInfo,
			GetTransactionInfoFromBytes: sv.GetTransactionInfoFromBytes,
			GetTransaction:              service.GetTransaction,
			GetPayloadInfo:              sv.GetPayloadInfo,
			GetPayload:                  service.GetPayload,
		},
		Compile:  Version,
		NodePort: cfg.NodePort,
		RPCPort:  cfg.RPCPort,
		RestPort: cfg.RESTPort,
		WSPort:   cfg.WSPort,
	}

	service := sv.NewHttpService(httpCfg, mempoolCfg.ChainParams.ElaAssetId)

	if cfg.EnableRPC {
		rpcServer := newJsonRpcServer(cfg.RPCPort, service)
		defer rpcServer.Stop()
		go func() {
			if err := rpcServer.Start(); err != nil {
				eladlog.Errorf("Start HttpJsonRpc server failed, %s", err.Error())
			}
		}()
	}

	if cfg.EnableWS {
		socketServer := newWebSocketServer(cfg.WSPort, service.HttpService)
		defer socketServer.Server.Stop()
		go func() {
			if err := socketServer.Server.Start(); err != nil {
				sockLog.Errorf("Start HttpSocket server failed, %s", err.Error())
			}
		}()

		events.Subscribe(socketServer.OnEvent)
	}

	go printSyncState(ledgerStore.ChainStore, server)

	events.Subscribe(handleRunTimeEvents)

	<-interrupt.C
}

func newJsonRpcServer(port uint16, service *sv.HttpServiceExtend) *jsonrpc.Server {
	s := jsonrpc.NewServer(&jsonrpc.Config{
		ServePort: port,
		User:      cfg.RPCUser,
		Pass:      cfg.RPCPass,
		WhiteList: cfg.RPCWhiteList,
	})
	s.RegisterAction("setloglevel", service.SetLogLevel, "level")
	s.RegisterAction("getblock", service.GetBlockByHash, "blockhash", "verbosity")
	s.RegisterAction("getcurrentheight", service.GetBlockHeight)
	s.RegisterAction("getblockhash", service.GetBlockHash, "height")
	s.RegisterAction("getconnectioncount", service.GetConnectionCount)
	s.RegisterAction("getrawmempool", service.GetTransactionPool)
	s.RegisterAction("getrawtransaction", service.GetRawTransaction, "txid", "verbose")
	s.RegisterAction("getneighbors", service.GetNeighbors)
	s.RegisterAction("getnodestate", service.GetNodeState)
	s.RegisterAction("sendrechargetransaction", service.SendRechargeToSideChainTxByHash)
	s.RegisterAction("sendrawtransaction", service.SendRawTransaction, "data")
	s.RegisterAction("getbestblockhash", service.GetBestBlockHash)
	s.RegisterAction("getblockcount", service.GetBlockCount)
	s.RegisterAction("getblockbyheight", service.GetBlockByHeight, "height")
	s.RegisterAction("getwithdrawtransactionsbyheight", service.GetWithdrawTransactionsByHeight, "height")
	s.RegisterAction("getexistdeposittransactions", service.GetExistDepositTransactions)
	s.RegisterAction("getwithdrawtransaction", service.GetWithdrawTransactionByHash, "txid")
	s.RegisterAction("submitsideauxblock", service.SubmitAuxBlock, "blockhash", "auxpow")
	s.RegisterAction("createauxblock", service.CreateAuxBlock, "paytoaddress")
	s.RegisterAction("togglemining", service.ToggleMining, "mining")
	s.RegisterAction("discretemining", service.DiscreteMining, "count")
	s.RegisterAction("listunspent", service.ListUnspent, "addresses")
	s.RegisterAction("getreceivedbyaddress", service.GetReceivedByAddress, "address", "assetid")
	s.RegisterAction("getillegalevidencebyheight", service.GetIllegalEvidenceByHeight, "height")

	s.RegisterAction("invokescript", service.InvokeScript, "script", "returntype")
	s.RegisterAction("invokefunction", service.InvokeFunction, "scripthash", "operation", "params", "returntype")
	s.RegisterAction("getOpPrice", service.GetOpPrice, "op", "args")
	s.RegisterAction("getTransactionReceipt", service.GetTransactionReceipt, "txid")
	return s
}

func newWebSocketServer(port uint16, service *service.HttpService) *websocket.SocketServer {
	svrCfg := sw.Config{
		ServePort: port,
		Service:   service,
	}
	server := websocket.NewSocketServer(&svrCfg)
	return server
}

func printSyncState(db *blockchain.ChainStore, server server.Server) {
	logger := elalog.NewBackend(logWriter).Logger("STAT",
		elalog.LevelInfo)

	ticker := time.NewTicker(printStateInterval)
	defer ticker.Stop()

	for range ticker.C {
		var buf bytes.Buffer
		buf.WriteString("-> ")
		buf.WriteString(strconv.FormatUint(uint64(db.GetHeight()), 10))
		peers := server.ConnectedPeers()
		buf.WriteString(" [")
		for i, p := range peers {
			buf.WriteString(strconv.FormatUint(uint64(p.ToPeer().Height()), 10))
			buf.WriteString(" ")
			buf.WriteString(p.ToPeer().String())
			if i != len(peers)-1 {
				buf.WriteString(", ")
			}
		}
		buf.WriteString("]")
		logger.Info(buf.String())
	}
}

func handleRunTimeEvents(et *events.Event) {
	if et.Type == event.ETRunTimeNotify {
		notifyInfo(et.Data.(datatype.StackItem))
	} else if et.Type == event.ETRunTimeLog {
		data := et.Data.(datatype.StackItem)
		avmlog.Info("onRunTimeLog:", string(data.GetByteArray()))
	}
}

func notifyInfo(item datatype.StackItem) {
	switch item.(type) {
	case *datatype.Boolean:
		avmlog.Info("notifyInfo:", item.GetBoolean())
	case *datatype.Integer:
		avmlog.Info("notifyInfo:", item.GetBigInteger())
	case *datatype.ByteArray:
		avmlog.Info("notifyInfo:", common.BytesToHexString(item.GetByteArray()))
	case *datatype.GeneralInterface:
		interop := item.GetInterface()
		buf := bytes.NewBuffer([]byte{})
		interop.Serialize(buf)
		avmlog.Info(common.BytesToHexString(buf.Bytes()))
	case *datatype.Array:
		items := item.GetArray()
		if len(items) == 4 && string(items[0].GetByteArray()) == "transfer" {
			str := string(items[0].GetByteArray()) + ":\n from:"
			str += common.BytesToHexString(items[1].GetByteArray()) + " to:"
			str += common.BytesToHexString(items[2].GetByteArray()) + " value:"
			str += items[3].GetBigInteger().String()
			avmlog.Info("notifyInfo:", str)
			return
		}
		for i := 0; i < len(items); i++ {
			notifyInfo(items[i])
		}
	}
}
