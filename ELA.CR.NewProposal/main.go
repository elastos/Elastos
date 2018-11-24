package main

import (
	"os"
	"runtime"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/dpos"
	"github.com/elastos/Elastos.ELA/log"
	"github.com/elastos/Elastos.ELA/node"
	"github.com/elastos/Elastos.ELA/pow"
	"github.com/elastos/Elastos.ELA/protocol"
	"github.com/elastos/Elastos.ELA/servers"
	"github.com/elastos/Elastos.ELA/servers/httpjsonrpc"
	"github.com/elastos/Elastos.ELA/servers/httpnodeinfo"
	"github.com/elastos/Elastos.ELA/servers/httprestful"
	"github.com/elastos/Elastos.ELA/servers/httpwebsocket"
	. "github.com/elastos/Elastos.ELA/version"
	"github.com/elastos/Elastos.ELA/version/blockhistory"
	"github.com/elastos/Elastos.ELA/version/txhistory"

	"github.com/elastos/Elastos.ELA.Utility/common"
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

func initVersions() {
	txV0 := &txhistory.TxVersionV0{}
	txV1 := &txhistory.TxVersionV1{}
	txVCurrent := &TxVersionMain{}

	blockV0 := &blockhistory.BlockVersionV0{}
	blockVCurrent := &BlockVersionMain{}

	blockchain.DefaultLedger.HeightVersions = NewHeightVersions(
		map[uint32]VersionInfo{
			0: {
				map[byte]TxVersion{txV0.GetVersion(): txV0},
				map[uint32]BlockVersion{blockV0.GetVersion(): blockV0},
			},
			88812: {
				map[byte]TxVersion{txV1.GetVersion(): txV1},
				map[uint32]BlockVersion{blockV0.GetVersion(): blockV0},
			},
			108812: {
				map[byte]TxVersion{txV1.GetVersion(): txV1, txVCurrent.GetVersion(): txVCurrent},
				map[uint32]BlockVersion{blockVCurrent.GetVersion(): blockVCurrent},
			}, //fixme height edit  later
		},
	)
}

func main() {
	//var blockChain *ledger.Blockchain
	var err error
	var noder protocol.Noder
	log.Info("Node version: ", config.Version)
	log.Info("1. BlockChain init")
	chainStore, err := blockchain.NewChainStore()
	if err != nil {
		goto ERROR
	}
	defer chainStore.Close()

	err = blockchain.Init(chainStore)
	initVersions()
	if err != nil {
		goto ERROR
	}

	log.Info("2. Start the P2P networks")
	noder = node.InitLocalNode()

	servers.ServerNode = noder

	log.Info("3. Start the RPC service")
	go httpjsonrpc.StartRPCServer()

	noder.WaitForSyncFinish()
	go httprestful.StartServer()
	go httpwebsocket.StartServer()
	if config.Parameters.HttpInfoStart {
		go httpnodeinfo.StartServer()
	}

	log.Info("4. Start consensus")
	startConsensus()

	if config.Parameters.EnableArbiter {
		log.Info("5. Start the manager")
		arbitrator := dpos.NewArbitrator()
		arbitrator.Start()
		blockchain.DefaultLedger.Blockchain.NewBlocksListeners = append(blockchain.DefaultLedger.Blockchain.NewBlocksListeners, arbitrator)
		blockchain.DefaultLedger.Arbitrators.RegisterListener(arbitrator)
	}

	select {}
ERROR:
	log.Error(err)
	os.Exit(-1)
}
