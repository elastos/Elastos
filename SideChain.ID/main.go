package main

import (
	"crypto/tls"
	"encoding/json"
	"errors"
	"fmt"
	"net"
	"os"
	"runtime"
	"time"

	bc "github.com/elastos/Elastos.ELA.SideChain.ID/blockchain"
	mp "github.com/elastos/Elastos.ELA.SideChain.ID/mempool"
	sv "github.com/elastos/Elastos.ELA.SideChain.ID/service"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/mempool"
	"github.com/elastos/Elastos.ELA.SideChain/pow"
	"github.com/elastos/Elastos.ELA.SideChain/server"
	"github.com/elastos/Elastos.ELA.SideChain/service"
	"github.com/elastos/Elastos.ELA.SideChain/service/httpnodeinfo"
	"github.com/elastos/Elastos.ELA.SideChain/spv"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/http/jsonrpc"
	"github.com/elastos/Elastos.ELA.Utility/http/restful"
	"github.com/elastos/Elastos.ELA.Utility/http/util"
)

const (
	defaultMultiCoreNum = 4

	restfulTlsPort = 443
)

func init() {
	var coreNum int
	if config.Parameters.MultiCoreNum > defaultMultiCoreNum {
		coreNum = int(config.Parameters.MultiCoreNum)
	} else {
		coreNum = defaultMultiCoreNum
	}

	eladlog.Debug("The Core number is ", coreNum)
	runtime.GOMAXPROCS(coreNum)
}

func main() {
	eladlog.Info("Node version: ", config.Version)
	params := config.Parameters
	foundation, err := common.Uint168FromAddress(config.Parameters.FoundationAddress)
	if err != nil {
		eladlog.Info("Please set correct foundation address in config file")
		os.Exit(-1)
	}

	eladlog.Info("1. BlockChain init")
	genesisBlock, err := bc.GenesisBlock()
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
	address, err := mempool.GetGenesisAddress(genesisBlock.Hash())
	if err != nil {
		eladlog.Fatalf("Genesis block hash to address failed, %s", err)
		os.Exit(1)
	}
	serviceCfg := spv.Config{
		Logger:        spvslog,
		ListenAddress: address,
		ChainStore:    chainCfg.ChainStore,
	}
	spvService, err := spv.NewService(&serviceCfg)
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
	server, err := server.New(chain, txPool)
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
	service := sv.NewHttpService(&service.Config{
		Server:                      server,
		Chain:                       chain,
		TxMemPool:                   txPool,
		PowService:                  powService,
		GetBlockInfo:                service.GetBlockInfo,
		GetTransactionInfo:          sv.GetTransactionInfo,
		GetTransactionInfoFromBytes: sv.GetTransactionInfoFromBytes,
		GetTransaction:              service.GetTransaction,
		GetPayloadInfo:              sv.GetPayloadInfo,
		GetPayload:                  service.GetPayload,
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
	s := jsonrpc.NewServer(&jsonrpc.Config{ServePort: port})

	s.RegisterAction("setloglevel", service.SetLogLevel, "level")
	s.RegisterAction("getinfo", service.GetInfo)
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
	s.RegisterAction("getblockbyheight", service.GetBlockByHeight)
	s.RegisterAction("getdestroyedtransactions", service.GetDestroyedTransactionsByHeight)
	s.RegisterAction("getexistdeposittransactions", service.GetExistDepositTransactions)
	s.RegisterAction("gettransactioninfo", service.GetTransactionInfoByHash)
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

func startHttpRESTful(port uint16, certFile, keyFile string, service *service.HttpService) {
	var (
		s = restful.NewServer(&restful.Config{
			ServePort: port,
			NetListen: func(port uint16) (net.Listener, error) {
				var err error
				var listener net.Listener

				if port%1000 == restfulTlsPort {
					// load cert
					cert, err := tls.LoadX509KeyPair(certFile, keyFile)
					if err != nil {
						restlog.Error("load keys fail", err)
						return nil, err
					}

					tlsConfig := &tls.Config{
						Certificates: []tls.Certificate{cert},
					}

					restlog.Infof("TLS listen port is %d", port)
					listener, err = tls.Listen("tcp", fmt.Sprint(":", port), tlsConfig)
					if err != nil {
						restlog.Error(err)
						return nil, err
					}

				} else {
					listener, err = net.Listen("tcp", fmt.Sprint(":", port))

				}

				return listener, err
			},
		})

		restartServer = func(params util.Params) (interface{}, error) {
			if err := s.Stop(); err != nil {
				str := fmt.Sprintf("Stop HttpRESTful server failed, %s", err.Error())
				restlog.Error(str)
				return nil, errors.New(str)
			}

			done := make(chan error)
			go func() {
				done <- s.Start()
			}()

			select {
			case err := <-done:
				return nil, fmt.Errorf("Start HttpRESTful server failed, %s", err.Error())
			case <-time.After(time.Millisecond * 100):
			}
			return nil, nil
		}

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
	s.RegisterGetAction(ApiRestart, restartServer)

	s.RegisterPostAction(ApiSendRawTransaction, sendRawTransaction)

	go func() {
		if err := s.Start(); err != nil {
			restlog.Errorf("Start HttpRESTful server failed, %s", err.Error())
		}
	}()
}
