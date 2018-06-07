package main

import (
	"os"
	"runtime"
	"time"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/log"
	"github.com/elastos/Elastos.ELA/node"
	"github.com/elastos/Elastos.ELA/pow"
	"github.com/elastos/Elastos.ELA/protocol"
	"github.com/elastos/Elastos.ELA/servers"
	"github.com/elastos/Elastos.ELA/servers/httpjsonrpc"
	"github.com/elastos/Elastos.ELA/servers/httpnodeinfo"
	"github.com/elastos/Elastos.ELA/servers/httprestful"
	"github.com/elastos/Elastos.ELA/servers/httpwebsocket"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

const (
	DefaultMultiCoreNum = 4
)

func init() {
	log.Init(log.Path, log.Stdout)
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

func handleLogFile() {
	go func() {
		for {
			time.Sleep(6 * time.Second)
			log.Trace("BlockHeight = ", blockchain.DefaultLedger.Blockchain.BlockHeight)
			blockchain.DefaultLedger.Blockchain.DumpState()
			bc := blockchain.DefaultLedger.Blockchain
			log.Info("[", len(bc.Index), len(bc.BlockCache), len(bc.Orphans), "]")
			//blockchain.DefaultLedger.Blockchain.DumpState()
			isNeedNewFile := log.CheckIfNeedNewFile()
			if isNeedNewFile {
				log.ClosePrintLog()
				log.Init(log.Path, os.Stdout)
			}
		} //for end
	}()

}

func startConsensus() {
	servers.LocalPow = pow.NewPowService("logPow")
	if config.Parameters.PowConfiguration.AutoMining {
		log.Info("Start POW Services")
		go servers.LocalPow.Start()
	}
}

func main() {
	//var blockChain *ledger.Blockchain
	var err error
	var noder protocol.Noder
	log.Trace("Node version: ", config.Version)
	log.Info("1. BlockChain init")
	chainStore, err := blockchain.NewChainStore()
	if err != nil {
		goto ERROR
	}
	defer chainStore.Close()

	err = blockchain.Init(chainStore)
	if err != nil {
		goto ERROR
	}

	log.Info("2. Start the P2P networks")
	noder = node.InitLocalNode()
	noder.WaitForSyncFinish()

	servers.NodeForServers = noder
	startConsensus()

	handleLogFile()

	log.Info("3. --Start the RPC service")
	go httpjsonrpc.StartRPCServer()
	go httprestful.StartServer()
	go httpwebsocket.StartServer()
	if config.Parameters.HttpInfoStart {
		go httpnodeinfo.StartServer()
	}
	select {}
ERROR:
	log.Error(err)
	os.Exit(-1)
}
