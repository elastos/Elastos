package main

import (
	"os"
	"runtime"
	"time"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/log"
	"github.com/elastos/Elastos.ELA.SideChain/node"
	"github.com/elastos/Elastos.ELA.SideChain/protocol"
	"github.com/elastos/Elastos.ELA.SideChain/servers"
	"github.com/elastos/Elastos.ELA.SideChain/servers/httpjsonrpc"
	"github.com/elastos/Elastos.ELA.SideChain/servers/httpnodeinfo"
	"github.com/elastos/Elastos.ELA.SideChain/servers/httprestful"
	"github.com/elastos/Elastos.ELA.SideChain/servers/httpwebsocket"
	"github.com/elastos/Elastos.ELA.SideChain/spv"
	"github.com/elastos/Elastos.ELA.SideChain/pow"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

const (
	FoundationAddress = "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta"
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

	address, err := common.Uint168FromAddress(FoundationAddress)
	if err != nil {
		log.Error(err.Error())
		os.Exit(-1)
	}
	blockchain.FoundationAddress = *address

	log.Debug("The Core number is ", coreNum)
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

func startConsensus(noder protocol.Noder) {
	servers.Pow = pow.NewPowService("logPow", noder)
	if config.Parameters.PowConfiguration.AutoMining {
		log.Info("Start POW Services")
		go servers.Pow.Start()
	}
}

func main() {
	//var blockChain *ledger.Blockchain
	var err error
	var noder protocol.Noder
	log.Info("1. BlockChain init")
	chainStore, err := blockchain.NewChainStore()
	if err != nil {
		log.Fatal("open LedgerStore err:", err)
		goto ERROR
	}
	defer chainStore.Close()

	err = blockchain.Init(chainStore)
	if err != nil {
		log.Fatal(err, "BlockChain initialize failed")
		goto ERROR
	}

	log.Info("2. SPV module init")
	spv.SpvInit()

	log.Info("3. Start the P2P networks")
	noder = node.InitNode()
	noder.WaitForSyncFinish()

	servers.NodeForServers = noder
	startConsensus(noder)

	handleLogFile()

	log.Info("4. --Start the RPC service")
	go httpjsonrpc.StartRPCServer()
	go httprestful.StartServer()
	go httpwebsocket.StartServer()
	if config.Parameters.HttpInfoStart {
		go httpnodeinfo.StartServer()
	}
	select {}
ERROR:
	os.Exit(1)
}
