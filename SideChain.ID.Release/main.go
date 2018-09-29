package main

import (
	"os"
	"runtime"

	bc "github.com/elastos/Elastos.ELA.SideChain.ID/blockchain"
	ic "github.com/elastos/Elastos.ELA.SideChain.ID/core"
	sv "github.com/elastos/Elastos.ELA.SideChain.ID/servers"
	rpc "github.com/elastos/Elastos.ELA.SideChain.ID/servers/httpjsonrpc"
	mp "github.com/elastos/Elastos.ELA.SideChain.ID/mempool"

	"github.com/elastos/Elastos.ELA.SideChain/servers"
	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/pow"
	"github.com/elastos/Elastos.ELA.SideChain/servers/httpnodeinfo"
	"github.com/elastos/Elastos.ELA.SideChain/servers/httprestful"
	"github.com/elastos/Elastos.ELA.SideChain/spv"
	"github.com/elastos/Elastos.ELA.SideChain/core"
	"github.com/elastos/Elastos.ELA.SideChain/mempool"
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
	core.InitOutputHelper()
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

	chainStore, err := bc.NewChainStore(genesisBlock)
	if err != nil {
		eladlog.Fatalf("open chain store failed, %s", err)
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
		eladlog.Info("Start POW Services")
		go powService.Start()
	}

	eladlog.Info("5. --Start the RPC service")
	service := sv.NewHttpService(&servers.Config{
		Logger:     elalog,
		Server:     server,
		Chain:      chain,
		TxMemPool:  txPool,
		PowService: powService,
	}, chainStore)

	go rpc.New(params.HttpJsonPort, service).Start()
	go httprestful.New(params.HttpRestPort, service.HttpService,
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
