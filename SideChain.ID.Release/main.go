package main

import (
	"bytes"
	"encoding/json"
	"os"
	"path/filepath"
	"runtime"
	"runtime/debug"
	"strconv"
	"time"

	bc "github.com/elastos/Elastos.ELA.SideChain.ID/blockchain"
	mp "github.com/elastos/Elastos.ELA.SideChain.ID/mempool"
	sv "github.com/elastos/Elastos.ELA.SideChain.ID/service"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/mempool"
	"github.com/elastos/Elastos.ELA.SideChain/pow"
	"github.com/elastos/Elastos.ELA.SideChain/server"
	"github.com/elastos/Elastos.ELA.SideChain/service"
	"github.com/elastos/Elastos.ELA.SideChain/service/websocket"
	"github.com/elastos/Elastos.ELA.SideChain/spv"
	"github.com/elastos/Elastos.ELA/utils/elalog"
	"github.com/elastos/Elastos.ELA/utils/http"
	"github.com/elastos/Elastos.ELA/utils/http/jsonrpc"
	"github.com/elastos/Elastos.ELA/utils/http/restful"
	"github.com/elastos/Elastos.ELA/utils/signal"
)

const (
	printStateInterval = time.Minute

	DataPath = "elastos_did"
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
	idChainStore, err := bc.NewChainStore(activeNetParams.GenesisBlock,
		filepath.Join(DataPath, DataDir, ChainDir))
	if err != nil {
		eladlog.Fatalf("open chain store failed, %s", err)
		os.Exit(1)
	}
	defer idChainStore.Close()

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

	spvCfg := spv.Config{
		DataDir:        filepath.Join(DataPath, DataDir, SpvDir),
		ChainParams:    spvNetParams,
		PermanentPeers: cfg.SPVPermanentPeers,
		GenesisAddress: genesisAddress,
	}
	spvService, err := spv.NewService(&spvCfg)
	if err != nil {
		eladlog.Fatalf("SPV module initialize failed, %s", err)
		os.Exit(1)
	}

	defer spvService.Stop()
	spvService.Start()

	mempoolCfg := mempool.Config{
		ChainParams: activeNetParams,
		ChainStore:  idChainStore.ChainStore,
		SpvService:  spvService,
	}
	txFeeHelper := mempool.NewFeeHelper(&mempoolCfg)
	mempoolCfg.FeeHelper = txFeeHelper
	txValidator := mp.NewValidator(&mempoolCfg, idChainStore)
	mempoolCfg.Validator = txValidator

	chainCfg := blockchain.Config{
		ChainParams:    activeNetParams,
		ChainStore:     idChainStore.ChainStore,
		GetTxFee:       txFeeHelper.GetTxFee,
		CheckTxSanity:  txValidator.CheckTransactionSanity,
		CheckTxContext: txValidator.CheckTransactionContext,
	}
	chain, err := blockchain.New(&chainCfg)
	if err != nil {
		eladlog.Fatalf("BlockChain initialize failed, %s", err)
		os.Exit(1)
	}
	chainCfg.Validator = blockchain.NewValidator(chain, spvService)

	txPool := mempool.New(&mempoolCfg)

	eladlog.Info("3. Start the P2P networks")
	server, err := server.New(&server.Config{
		DataDir:        filepath.Join(DataPath, DataDir),
		Chain:          chain,
		TxMemPool:      txPool,
		ChainParams:    activeNetParams,
		PermanentPeers: cfg.PermanentPeers,
	})
	if err != nil {
		eladlog.Fatalf("initialize P2P networks failed, %s", err)
		os.Exit(1)
	}
	defer server.Stop()
	server.Start()

	eladlog.Info("4. --Initialize pow service")
	powCfg := pow.Config{
		ChainParams:               activeNetParams,
		MinerAddr:                 cfg.PayToAddr,
		MinerInfo:                 cfg.MinerInfo,
		Server:                    server,
		Chain:                     chain,
		TxMemPool:                 txPool,
		TxFeeHelper:               txFeeHelper,
		Validator:                 txValidator,
		CreateCoinBaseTx:          pow.CreateCoinBaseTx,
		GenerateBlock:             pow.GenerateBlock,
		GenerateBlockTransactions: pow.GenerateBlockTransactions,
	}

	powService := pow.NewService(&powCfg)
	if cfg.EnableMining {
		eladlog.Info("Start POW Services")
		go powService.Start()
	}

	eladlog.Info("5. --Start the RPC service")
	serviceCfg := sv.Config{
		Config: service.Config{
			Server:                      server,
			Chain:                       chain,
			Store:                       idChainStore.ChainStore,
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
		Store:    idChainStore,
	}
	service := sv.NewHttpService(&serviceCfg)

	if cfg.EnableRPC {
		rpcServer := newRPCServer(cfg.RPCPort, service)
		defer rpcServer.Stop()
		go func() {
			if err := rpcServer.Start(); err != nil {
				eladlog.Errorf("Start JSON-PRC server failed, %s", err)
			}
		}()
	}

	if cfg.EnableREST {
		restServer := newRESTfulServer(cfg.RESTPort, service.HttpService)
		defer restServer.Stop()
		go func() {
			if err := restServer.Start(); err != nil {
				eladlog.Errorf("Start RESTful server failed, %s", err)
			}
		}()
	}

	if cfg.EnableWS {
		wsServer := newWebSocketServer(cfg.WSPort, service.HttpService, &serviceCfg.Config)
		defer wsServer.Stop()
		go func() {
			if err := wsServer.Start(); err != nil {
				eladlog.Errorf("Start WebSocket server failed, %s", err)
			}
		}()
	}

	go printSyncState(idChainStore.ChainStore, server)

	<-interrupt.C
}

func newRPCServer(port uint16, service *sv.HttpService) *jsonrpc.Server {
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
	s.RegisterAction("getreceivedbyaddress", service.GetReceivedByAddress, "address")
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
	s.RegisterAction("getidentificationtxbyidandpath", service.GetIdentificationTxByIdAndPath, "id", "path")
	s.RegisterAction("listunspent", service.ListUnspent, "addresses")
	s.RegisterAction("getillegalevidencebyheight", service.GetIllegalEvidenceByHeight, "height")
	s.RegisterAction("checkillegalevidence", service.CheckIllegalEvidence, "evidence")
	s.RegisterAction("resolvedid", service.ResolveDID, "id", "all")

	return s
}

func newRESTfulServer(port uint16, service *service.HttpService) *restful.Server {
	var (
		s = restful.NewServer(&restful.Config{ServePort: port})

		sendRawTransaction = func(data []byte) (interface{}, error) {
			var params = http.Params{}
			if err := json.Unmarshal(data, &params); err != nil {
				return nil, err
			}
			return service.SendRawTransaction(params)
		}
	)

	const (
		ApiGetConnectionCount  = "/api/v1/node/connectioncount"
		ApiGetBlockTxsByHeight = "/api/v1/block/transactions/height/:height"
		ApiGetBlockByHeight    = "/api/v1/block/details/height/:height"
		ApiGetBlockByHash      = "/api/v1/block/details/hash/:blockhash"
		ApiGetBlockHeight      = "/api/v1/block/height"
		ApiGetBlockHash        = "/api/v1/block/hash/:height"
		ApiGetTotalIssued      = "/api/v1/totalissued"
		ApiGetTransaction      = "/api/v1/transaction/:hash"
		ApiGetAsset            = "/api/v1/asset/:hash"
		ApiGetUTXOByAddr       = "/api/v1/asset/utxos/:addr"
		ApiGetUTXOByAsset      = "/api/v1/asset/utxo/:addr/:assetid"
		ApiGetBalanceByAddr    = "/api/v1/asset/balances/:addr"
		ApiGetBalanceByAsset   = "/api/v1/asset/balance/:addr/:assetid"
		ApiSendRawTransaction  = "/api/v1/transaction"
		ApiGetTransactionPool  = "/api/v1/transactionpool"
		ApiRestart             = "/api/v1/restart"
	)

	s.RegisterGetAction(ApiGetConnectionCount, service.GetConnectionCount)
	s.RegisterGetAction(ApiGetBlockTxsByHeight, service.GetTransactionsByHeight)
	s.RegisterGetAction(ApiGetBlockByHeight, service.GetBlockByHeight)
	s.RegisterGetAction(ApiGetBlockByHash, service.GetBlockByHash)
	s.RegisterGetAction(ApiGetBlockHeight, service.GetBlockHeight)
	s.RegisterGetAction(ApiGetBlockHash, service.GetBlockHash)
	s.RegisterGetAction(ApiGetTransactionPool, service.GetTransactionPool)
	s.RegisterGetAction(ApiGetTransaction, service.GetTransactionByHash)
	s.RegisterGetAction(ApiGetAsset, service.GetAssetByHash)
	s.RegisterGetAction(ApiGetUTXOByAddr, service.GetUnspendsByAddr)
	s.RegisterGetAction(ApiGetUTXOByAsset, service.GetUnspendsByAsset)
	s.RegisterGetAction(ApiGetBalanceByAddr, service.GetBalanceByAddr)
	s.RegisterGetAction(ApiGetBalanceByAsset, service.GetBalanceByAsset)

	s.RegisterPostAction(ApiSendRawTransaction, sendRawTransaction)

	return s
}

func newWebSocketServer(port uint16, service *service.HttpService, config *service.Config) *websocket.Server {
	svrCfg := websocket.Config{
		Flags:      2,
		ServePort:  port,
		Service:    service,
		ServiceCfg: config,
	}
	server := websocket.NewServer(&svrCfg)
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
