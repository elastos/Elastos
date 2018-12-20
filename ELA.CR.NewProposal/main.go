package main

import (
	"os"
	"runtime"

	"github.com/elastos/Elastos.ELA.Utility/signal"
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/blockchain/interfaces"
	"github.com/elastos/Elastos.ELA/cli/password"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/dpos"
	"github.com/elastos/Elastos.ELA/dpos/store"
	"github.com/elastos/Elastos.ELA/node"
	"github.com/elastos/Elastos.ELA/pow"
	"github.com/elastos/Elastos.ELA/protocol"
	"github.com/elastos/Elastos.ELA/servers"
	"github.com/elastos/Elastos.ELA/servers/httpjsonrpc"
	"github.com/elastos/Elastos.ELA/servers/httpnodeinfo"
	"github.com/elastos/Elastos.ELA/servers/httprestful"
	"github.com/elastos/Elastos.ELA/servers/httpwebsocket"
	"github.com/elastos/Elastos.ELA/version/verconfig"
)

const (
	DefaultMultiCoreNum = 4
)

func init() {
	log.Init(
		config.Parameters.PrintLevel,
		config.Parameters.MaxPerLogSize,
		config.Parameters.MaxLogsSize,
	)
	var coreNum int
	if config.Parameters.MultiCoreNum > DefaultMultiCoreNum {
		coreNum = int(config.Parameters.MultiCoreNum)
	} else {
		coreNum = DefaultMultiCoreNum
	}
	log.Debug("The Core number is ", coreNum)

	foundationAddress := config.Parameters.Configuration.FoundationAddress
	if foundationAddress == "" {
		foundationAddress = "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta"
	}

	address, err := common.Uint168FromAddress(foundationAddress)
	if err != nil {
		log.Error(err.Error())
		os.Exit(-1)
	}
	blockchain.FoundationAddress = *address

	runtime.GOMAXPROCS(coreNum)
}

func startConsensus() {
	servers.LocalPow = pow.NewPowService()
	if config.Parameters.PowConfiguration.AutoMining {
		log.Info("Start POW Services")
		go servers.LocalPow.Start()
	}
}

func main() {
	//var blockChain *ledger.Blockchain
	var err error
	var noder protocol.Noder
	var pwd []byte
	var arbitrator dpos.Arbitrator
	var interrupt = signal.NewInterrupt()

	log.Info("Node version: ", config.Version)
	log.Info("BlockChain init")
	versions := verconfig.InitVersions()
	var dposStore interfaces.IDposStore
	chainStore, err := blockchain.NewChainStore("Chain")
	if err != nil {
		goto ERROR
	}
	defer chainStore.Close()
	dposStore, err = store.NewDposStore("Dpos_Data")
	if err != nil {
		goto ERROR
	}
	defer dposStore.Disconnect()
	err = blockchain.Init(chainStore, versions)
	if err != nil {
		goto ERROR
	}
	store.InitArbitrators(store.ArbitratorsConfig{
		ArbitratorsCount: config.Parameters.ArbiterConfiguration.ArbitratorsCount,
		CandidatesCount:  config.Parameters.ArbiterConfiguration.CandidatesCount,
		MajorityCount:    config.Parameters.ArbiterConfiguration.MajorityCount,
		Store:            dposStore,
	})
	if err = blockchain.DefaultLedger.Arbitrators.StartUp(); err != nil {
		goto ERROR
	}

	log.Info("Start the P2P networks")
	noder = node.InitLocalNode()

	if config.Parameters.EnableArbiter {
		log.Info("Start the manager")
		pwd, err = password.GetPassword()
		if err != nil {
			goto ERROR
		}
		arbitrator, err = dpos.NewArbitrator(pwd,
			dpos.ArbitratorConfig{
				EnableEventLog:    true,
				EnableEventRecord: true,
				Store:             dposStore,
			})
		if err != nil {
			goto ERROR
		}
		defer arbitrator.Stop()
		arbitrator.Start()
		blockchain.DefaultLedger.Blockchain.NewBlocksListeners = append(blockchain.DefaultLedger.Blockchain.NewBlocksListeners, arbitrator)
		blockchain.DefaultLedger.Arbitrators.RegisterListener(arbitrator)
	}

	servers.ServerNode = noder
	servers.ServerNode.RegisterTxPoolListener(arbitrator)
	servers.ServerNode.RegisterTxPoolListener(chainStore)

	log.Info("Start services")
	go httpjsonrpc.StartRPCServer()
	go httprestful.StartServer()
	go httpwebsocket.StartServer()
	if config.Parameters.HttpInfoStart {
		go httpnodeinfo.StartServer()
	}

	noder.WaitForSyncFinish(interrupt.C)
	if interrupt.Interrupted() {
		return
	}
	log.Info("Start consensus")
	startConsensus()

	<-interrupt.C
ERROR:
	log.Error(err)
	os.Exit(-1)
}
