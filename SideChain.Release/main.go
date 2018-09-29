package main

import (
	"os"
	"runtime"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/core"
	"github.com/elastos/Elastos.ELA.SideChain/logger"
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

	log.Debug("The Core number is ", coreNum)
	runtime.GOMAXPROCS(coreNum)
}

func main() {
	logger := logger.NewLogger(
		"./logs/",
		config.Parameters.PrintLevel,
		config.Parameters.MaxPerLogSize,
		config.Parameters.MaxLogsSize,
	)
	UseLogger(logger)

	log.Info("Node version: ", config.Version)

	core.InitPayloadCreater()
	core.InitTransactionHelper()
	core.InitOutputHelper()

	params := config.Parameters

	foundation, err := common.Uint168FromAddress(config.Parameters.FoundationAddress)
	if err != nil {
		log.Info("Please set correct foundation address in config file")
		os.Exit(-1)
	}

	log.Info("1. BlockChain init")
	genesisBlock, err := blockchain.GenesisBlock()
	if err != nil {
		log.Fatalf("Get genesis block failed, error %s", err)
		os.Exit(1)
	}

	chainStore, err := blockchain.NewChainStore(genesisBlock)
	if err != nil {
		log.Fatalf("open chain store failed, %s", err)
		os.Exit(1)
	}
	defer chainStore.Close()

	chainCfg := blockchain.Config{
		FoundationAddress: *foundation,
		ChainStore:        chainStore,
		AssetId:           genesisBlock.Transactions[0].Hash(),
		PowLimit:          params.ChainParam.PowLimit,
		MaxOrphanBlocks:   params.ChainParam.MaxOrphanBlocks,
		MinMemoryNodes:    params.ChainParam.MinMemoryNodes,
	}

	mempoolCfg := mempool.Config{
		FoundationAddress: *foundation,
		AssetId:           genesisBlock.Transactions[0].Hash(),
		ExchangeRage:      params.ExchangeRate,
		ChainStore:        chainStore,
	}

	txFeeHelper := mempool.NewFeeHelper(&mempoolCfg)
	mempoolCfg.FeeHelper = txFeeHelper
	chainCfg.GetTxFee = txFeeHelper.GetTxFee

	txValidator := mempool.NewValidator(&mempoolCfg)
	mempoolCfg.Validator = txValidator
	chainCfg.CheckTxSanity = txValidator.CheckTransactionSanity
	chainCfg.CheckTxContext = txValidator.CheckTransactionContext

	chain, err := blockchain.New(&chainCfg)
	if err != nil {
		log.Fatalf("BlockChain initialize failed, %s", err)
		os.Exit(1)
	}

	log.Info("2. SPV module init")
	if err := spv.SpvInit(); err != nil {
		log.Fatalf("SPV module initialize failed, %s", err)
		os.Exit(1)
	}

	txPool := mempool.New(&mempoolCfg)

	log.Info("3. Start the P2P networks")
	server, err := newServer(chain, txPool)
	if err != nil {
		log.Fatalf("initialize P2P networks failed, %s", err)
		os.Exit(1)
	}
	server.Start()

	log.Info("4. --Initialize pow service")
	powParam := params.PowConfiguration
	powCfg := pow.Config{
		Foundation:    *foundation,
		MinerAddr:     powParam.PayToAddr,
		MinerInfo:     powParam.MinerInfo,
		LimitBits:     params.ChainParam.PowLimitBits,
		MaxBlockSize:  params.MaxBlockSize,
		MaxTxPerBlock: params.MaxTxInBlock,
		Server:        server,
		Chain:         chain,
		TxMemPool:     txPool,
		TxFeeHelper:   txFeeHelper,
	}

	powService := pow.NewService(&powCfg)
	if params.PowConfiguration.AutoMining {
		log.Info("Start POW Services")
		go powService.Start()
	}

	log.Info("5. --Start the RPC service")
	service := servers.NewHttpService(&servers.Config{
		Logger:     logger,
		Server:     server,
		Chain:      chain,
		TxMemPool:  txPool,
		PowService: powService,
	})

	go httpjsonrpc.New(params.HttpJsonPort, service).Start()
	go httprestful.New(params.HttpRestPort, service,
		params.RestCertPath, params.RestKeyPath).Start()
	if params.HttpInfoStart {
		go httpnodeinfo.New(&httpnodeinfo.Config{
			NodePort:     params.NodePort,
			HttpJsonPort: params.HttpJsonPort,
			HttpRestPort: params.HttpRestPort,
			DB:           chainStore,
			Server:       server,
		}).Start()
	}
	select {}
}
