package main

import (
	"bytes"
	"encoding/json"
	"os"
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
	"github.com/elastos/Elastos.ELA.SideChain/spv"

	"github.com/elastos/Elastos.ELA.Utility/elalog"
	"github.com/elastos/Elastos.ELA.Utility/http/jsonrpc"
	"github.com/elastos/Elastos.ELA.Utility/http/restful"
	"github.com/elastos/Elastos.ELA.Utility/http/util"
	"github.com/elastos/Elastos.ELA.Utility/signal"
)

const (
	printStateInterval = 10 * time.Second
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

	if loadConfigErr != nil {
		eladlog.Fatalf("load config file failed %s", loadConfigErr)
		os.Exit(-1)
	}

	// listen interrupt signals.
	interrupt := signal.NewInterrupt()

	eladlog.Info("1. BlockChain init")
	idChainStore, err := bc.NewChainStore(activeNetParams.GenesisBlock)
	if err != nil {
		eladlog.Fatalf("open chain store failed, %s", err)
		os.Exit(1)
	}
	defer idChainStore.Close()

	chainCfg := blockchain.Config{
		ChainParams: activeNetParams,
		ChainStore:  idChainStore.ChainStore,
	}

	mempoolCfg := mempool.Config{
		ChainParams: activeNetParams,
		ChainStore:  idChainStore.ChainStore,
	}
	txFeeHelper := mempool.NewFeeHelper(&mempoolCfg)
	mempoolCfg.FeeHelper = txFeeHelper
	chainCfg.GetTxFee = txFeeHelper.GetTxFee

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
		DataDir:        "./data_spv",
		Magic:          activeNetParams.SpvParams.Magic,
		DefaultPort:    activeNetParams.SpvParams.DefaultPort,
		SeedList:       activeNetParams.SpvParams.SeedList,
		Foundation:     activeNetParams.SpvParams.Foundation,
		GenesisAddress: genesisAddress,
		TxStore:        spv.NewTxStore(idChainStore.ChainStore),
	}
	spvService, err := spv.NewService(&spvCfg)
	if err != nil {
		eladlog.Fatalf("SPV module initialize failed, %s", err)
		os.Exit(1)
	}
	mempoolCfg.SpvService = spvService

	defer spvService.Stop()
	spvService.Start()

	txValidator := mp.NewValidator(&mempoolCfg)
	mempoolCfg.Validator = txValidator
	chainCfg.CheckTxSanity = txValidator.CheckTransactionSanity
	chainCfg.CheckTxContext = txValidator.CheckTransactionContext

	chain, err := blockchain.New(&chainCfg)
	if err != nil {
		eladlog.Fatalf("BlockChain initialize failed, %s", err)
		os.Exit(1)
	}
	chainCfg.Validator = blockchain.NewValidator(chain)

	txPool := mempool.New(&mempoolCfg)

	eladlog.Info("3. Start the P2P networks")
	server, err := server.New(chain, txPool, activeNetParams)
	if err != nil {
		eladlog.Fatalf("initialize P2P networks failed, %s", err)
		os.Exit(1)
	}
	defer server.Stop()
	server.Start()

	eladlog.Info("4. --Initialize pow service")
	powCfg := pow.Config{
		ChainParams:               activeNetParams,
		MinerAddr:                 cfg.MinerAddr,
		MinerInfo:                 cfg.MinerInfo,
		Server:                    server,
		Chain:                     chain,
		TxMemPool:                 txPool,
		TxFeeHelper:               txFeeHelper,
		CreateCoinBaseTx:          pow.CreateCoinBaseTx,
		GenerateBlock:             pow.GenerateBlock,
		GenerateBlockTransactions: pow.GenerateBlockTransactions,
	}

	powService := pow.NewService(&powCfg)
	if cfg.Mining {
		eladlog.Info("Start POW Services")
		go powService.Start()
	}

	eladlog.Info("5. --Start the RPC service")
	service := sv.NewHttpService(&service.Config{
		Server:                      server,
		Chain:                       chain,
		TxMemPool:                   txPool,
		PowService:                  powService,
		SetLogLevel:                 setLogLevel,
		GetBlockInfo:                service.GetBlockInfo,
		GetTransactionInfo:          sv.GetTransactionInfo,
		GetTransactionInfoFromBytes: sv.GetTransactionInfoFromBytes,
		GetTransaction:              service.GetTransaction,
		GetPayloadInfo:              sv.GetPayloadInfo,
		GetPayload:                  service.GetPayload,
	}, idChainStore)

	rpcServer := newJsonRpcServer(cfg.HttpJsonPort, service)
	defer rpcServer.Stop()
	go func() {
		if err := rpcServer.Start(); err != nil {
			eladlog.Errorf("Start HttpJsonRpc server failed, %s", err.Error())
		}
	}()

	restServer := newRESTfulServer(cfg.HttpRestPort, service.HttpService)
	defer restServer.Stop()
	go func() {
		if err := restServer.Start(); err != nil {
			restlog.Errorf("Start HttpRESTful server failed, %s", err.Error())
		}
	}()

	if cfg.MonitorState {
		go printSyncState(idChainStore.ChainStore, server)
	}

	<-interrupt.C
}

func newJsonRpcServer(port uint16, service *sv.HttpServiceExtend) *jsonrpc.Server {
	s := jsonrpc.NewServer(&jsonrpc.Config{ServePort: port})

	s.RegisterAction("setloglevel", service.SetLogLevel, "level")
	s.RegisterAction("getblock", service.GetBlockByHash, "blockhash", "verbosity")
	s.RegisterAction("getcurrentheight", service.GetBlockHeight)
	s.RegisterAction("getblockhash", service.GetBlockHash, "height")
	s.RegisterAction("getconnectioncount", service.GetConnectionCount)
	s.RegisterAction("getrawmempool", service.GetTransactionPool)
	s.RegisterAction("getrawtransaction", service.GetRawTransaction, "txid", "verbose")
	s.RegisterAction("getneighbors", service.GetNeighbors)
	s.RegisterAction("getnodestate", service.GetNodeState)
	s.RegisterAction("sendtransactioninfo", service.SendTransactionInfo)
	s.RegisterAction("sendrawtransaction", service.SendRawTransaction, "data")
	s.RegisterAction("getbestblockhash", service.GetBestBlockHash)
	s.RegisterAction("getblockcount", service.GetBlockCount)
	s.RegisterAction("getblockbyheight", service.GetBlockByHeight, "height")
	s.RegisterAction("getdestroyedtransactions", service.GetDestroyedTransactionsByHeight, "height")
	s.RegisterAction("getexistdeposittransactions", service.GetExistDepositTransactions)
	s.RegisterAction("gettransactioninfo", service.GetTransactionInfoByHash)
	s.RegisterAction("submitsideauxblock", service.SubmitAuxBlock, "blockhash", "auxpow")
	s.RegisterAction("createauxblock", service.CreateAuxBlock, "paytoaddress")
	s.RegisterAction("togglemining", service.ToggleMining, "mining")
	s.RegisterAction("discretemining", service.DiscreteMining, "count")
	s.RegisterAction("getidentificationtxbyidandpath", service.GetIdentificationTxByIdAndPath, "id", "path")

	return s
}

func newRESTfulServer(port uint16, service *service.HttpService) *restful.Server {
	var (
		s = restful.NewServer(&restful.Config{ServePort: port})

		sendRawTransaction = func(data []byte) (interface{}, error) {
			var params = util.Params{}
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
		ApiGetBlockByHash      = "/api/v1/block/details/hash/:blockhash/:verbosity"
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
