package main

import (
	"os"
	"runtime"

	bc "github.com/elastos/Elastos.ELA.SideChain.ID/blockchain"
	ic "github.com/elastos/Elastos.ELA.SideChain.ID/core"
	mp "github.com/elastos/Elastos.ELA.SideChain.ID/mempool"
	sv "github.com/elastos/Elastos.ELA.SideChain.ID/servers"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/mempool"
	"github.com/elastos/Elastos.ELA.SideChain/pow"
	"github.com/elastos/Elastos.ELA.SideChain/servers"
	"github.com/elastos/Elastos.ELA.SideChain/servers/httpjsonrpc"
	"github.com/elastos/Elastos.ELA.SideChain/servers/httpnodeinfo"
	"github.com/elastos/Elastos.ELA.SideChain/servers/httprestful"
	"github.com/elastos/Elastos.ELA.SideChain/spv"
	"github.com/elastos/Elastos.ELA.Utility/common"
)

const (
	DefaultMultiCoreNum = 4
)

func init() {
	var coreNum int
	if config.Parameters.MultiCoreNum > DefaultMultiCoreNum {
		coreNum = int(config.Parameters.MultiCoreNum)
	} else {
		coreNum = DefaultMultiCoreNum
	}

	eladlog.Debug("The Core number is ", coreNum)
	runtime.GOMAXPROCS(coreNum)
}

func main() {
	eladlog.Info("Node version: ", config.Version)

	ic.InitPayloadHelper()
	ic.InitTransactionHelper()

	params := config.Parameters

	foundation, err := common.Uint168FromAddress(config.Parameters.FoundationAddress)
	if err != nil {
		eladlog.Info("Please set correct foundation address in config file")
		os.Exit(-1)
	}

	eladlog.Info("1. BlockChain init")
	genesisBlock, err := blockchain.GenesisBlock()
	if err != nil {
		eladlog.Fatalf("Get genesis block failed, error %s", err)
		os.Exit(1)
	}

	idChainStore, err := bc.NewChainStore(genesisBlock)
	if err != nil {
		eladlog.Fatalf("open chain store failed, %s", err)
		os.Exit(1)
	}
	defer idChainStore.Close()

	chainCfg := blockchain.Config{
		FoundationAddress: *foundation,
		ChainStore:        idChainStore.ChainStore,
		AssetId:           genesisBlock.Transactions[0].Hash(),
		PowLimit:          params.ChainParam.PowLimit,
		MaxOrphanBlocks:   params.ChainParam.MaxOrphanBlocks,
		MinMemoryNodes:    params.ChainParam.MinMemoryNodes,
	}

	mempoolCfg := mempool.Config{
		FoundationAddress: *foundation,
		AssetId:           genesisBlock.Transactions[0].Hash(),
		ExchangeRage:      params.ExchangeRate,
		ChainStore:        idChainStore.ChainStore,
	}

	txFeeHelper := mempool.NewFeeHelper(&mempoolCfg)
	mempoolCfg.FeeHelper = txFeeHelper
	chainCfg.GetTxFee = txFeeHelper.GetTxFee

	eladlog.Info("2. SPV module init")
	spvService, err := spv.NewService(spvslog)
	if err != nil {
		eladlog.Fatalf("SPV module initialize failed, %s", err)
		os.Exit(1)
	}
	spvService.Start()
	mempoolCfg.SpvService = spvService

	txValidator := mp.NewValidator(&mempoolCfg)
	mempoolCfg.Validator = txValidator
	chainCfg.CheckTxSanity = txValidator.CheckTransactionSanity
	chainCfg.CheckTxContext = txValidator.CheckTransactionContext

	chain, err := blockchain.New(&chainCfg)
	if err != nil {
		eladlog.Fatalf("BlockChain initialize failed, %s", err)
		os.Exit(1)
	}

	txPool := mempool.New(&mempoolCfg)

	eladlog.Info("3. Start the P2P networks")
	server, err := newServer(chain, txPool)
	if err != nil {
		eladlog.Fatalf("initialize P2P networks failed, %s", err)
		os.Exit(1)
	}
	server.Start()

	eladlog.Info("4. --Initialize pow service")
	powParam := params.PowConfiguration
	powCfg := pow.Config{
		Foundation:                *foundation,
		MinerAddr:                 powParam.PayToAddr,
		MinerInfo:                 powParam.MinerInfo,
		LimitBits:                 params.ChainParam.PowLimitBits,
		MaxBlockSize:              params.MaxBlockSize,
		MaxTxPerBlock:             params.MaxTxInBlock,
		Server:                    server,
		Chain:                     chain,
		TxMemPool:                 txPool,
		TxFeeHelper:               txFeeHelper,
		CreateCoinBaseTx:          pow.CreateCoinBaseTx,
		GenerateBlock:             pow.GenerateBlock,
		GenerateBlockTransactions: pow.GenerateBlockTransactions,
	}

	powService := pow.NewService(&powCfg)
	if params.PowConfiguration.AutoMining {
		eladlog.Info("Start POW Services")
		go powService.Start()
	}

	eladlog.Info("5. --Start the RPC service")
	service := sv.NewHttpService(&servers.Config{
		Logger:                      elalog,
		Server:                      server,
		Chain:                       chain,
		TxMemPool:                   txPool,
		PowService:                  powService,
		GetBlockInfo:                servers.GetBlockInfo,
		GetTransactionInfo:          sv.GetTransactionInfo,
		GetTransactionInfoFromBytes: sv.GetTransactionInfoFromBytes,
		GetTransaction:              servers.GetTransaction,
		GetPayloadInfo:              sv.GetPayloadInfo,
		GetPayload:                  servers.GetPayload,
	}, idChainStore)

	startHttpJsonRpc(params.HttpJsonPort, service)
	startHttpRESTful(params.HttpRestPort, params.RestCertPath,
		params.RestKeyPath, service.HttpService)

	if params.HttpInfoStart {
		go httpnodeinfo.New(&httpnodeinfo.Config{
			NodePort:     params.NodePort,
			HttpJsonPort: params.HttpJsonPort,
			HttpRestPort: params.HttpRestPort,
			Chain:        chain,
			Server:       server,
		}).Start()
	}
	select {}
}

func startHttpJsonRpc(port uint16, service *sv.HttpServiceExtend) {
	s := httpjsonrpc.New(port)

	s.RegisterAction("setloglevel", service.SetLogLevel, "level")
	s.RegisterAction("getinfo", service.GetInfo)
	s.RegisterAction("getblock", service.GetBlockByHash, "hash", "verbosity")
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
	s.RegisterAction("getblockbyheight", service.GetBlockByHeight)
	s.RegisterAction("getdestroyedtransactions", service.GetDestroyedTransactionsByHeight)
	s.RegisterAction("getexistdeposittransactions", service.GetExistDepositTransactions)
	s.RegisterAction("help", service.AuxHelp)
	s.RegisterAction("submitsideauxblock", service.SubmitSideAuxBlock, "blockhash", "auxpow")
	s.RegisterAction("createauxblock", service.CreateAuxBlock, "paytoaddress")
	s.RegisterAction("togglemining", service.ToggleMining, "mining")
	s.RegisterAction("discretemining", service.DiscreteMining, "count")
	s.RegisterAction("getidentificationtxbyidandpath", service.GetIdentificationTxByIdAndPath, "id", "path")

	go func() {
		if err := s.Start(); err != nil {
			eladlog.Errorf("Start HttpJsonRpc service failed, %s", err.Error())
		}
	}()
}

func startHttpRESTful(port uint16, certFile, keyFile string, service *servers.HttpService) {
	s := httprestful.New(port, certFile, keyFile)

	const (
		ApiGetConnectionCount  = "/api/v1/node/connectioncount"
		ApiGetBlockTxsByHeight = "/api/v1/block/transactions/height"
		ApiGetBlockByHeight    = "/api/v1/block/details/height"
		ApiGetBlockByHash      = "/api/v1/block/details/hash"
		ApiGetBlockHeight      = "/api/v1/block/height"
		ApiGetBlockHash        = "/api/v1/block/hash"
		ApiGetTotalIssued      = "/api/v1/totalissued"
		ApiGetTransaction      = "/api/v1/transaction"
		ApiGetAsset            = "/api/v1/asset"
		ApiGetBalanceByAddr    = "/api/v1/asset/balances"
		ApiGetBalanceByAsset   = "/api/v1/asset/balance"
		ApiGetUTXOByAsset      = "/api/v1/asset/utxo"
		ApiGetUTXOByAddr       = "/api/v1/asset/utxos"
		ApiSendRawTransaction  = "/api/v1/transaction"
		ApiGetTransactionPool  = "/api/v1/transactionpool"
		ApiRestart             = "/api/v1/restart"
	)

	s.RegisterAction("GET", ApiGetConnectionCount, service.GetConnectionCount)
	s.RegisterAction("GET", ApiGetBlockTxsByHeight, service.GetTransactionsByHeight, "height")
	s.RegisterAction("GET", ApiGetBlockByHeight, service.GetBlockByHeight, "height")
	s.RegisterAction("GET", ApiGetBlockByHash, service.GetBlockByHash, "hash")
	s.RegisterAction("GET", ApiGetBlockHeight, service.GetBlockHeight)
	s.RegisterAction("GET", ApiGetBlockHash, service.GetBlockHash, "height")
	s.RegisterAction("GET", ApiGetTransactionPool, service.GetTransactionPool)
	s.RegisterAction("GET", ApiGetTransaction, service.GetTransactionByHash, "hash")
	s.RegisterAction("GET", ApiGetAsset, service.GetAssetByHash, "hash")
	s.RegisterAction("GET", ApiGetUTXOByAddr, service.GetUnspendsByAddr, "addr")
	s.RegisterAction("GET", ApiGetUTXOByAsset, service.GetUnspendsByAsset, "addr", "assetid")
	s.RegisterAction("GET", ApiGetBalanceByAddr, service.GetBalanceByAddr, "addr")
	s.RegisterAction("GET", ApiGetBalanceByAsset, service.GetBalanceByAsset, "addr", "assetid")
	s.RegisterAction("GET", ApiRestart, s.Restart)

	s.RegisterAction("POST", ApiSendRawTransaction, service.SendRawTransaction, "data")

	go func() {
		if err := s.Start(); err != nil {
			eladlog.Errorf("Start HttpRESTful service failed, %s", err.Error())
		}
	}()
}
