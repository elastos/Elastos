package main

import (
	"os"
	"runtime"
	"time"

	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/consensus/pow"
	"github.com/elastos/Elastos.ELA/log"
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/store/chainstore"
	"github.com/elastos/Elastos.ELA/net/node"
	"github.com/elastos/Elastos.ELA/net/protocol"
	"github.com/elastos/Elastos.ELA/net/servers"
	"github.com/elastos/Elastos.ELA/net/servers/httpjsonrpc"
	"github.com/elastos/Elastos.ELA/net/servers/httpnodeinfo"
	"github.com/elastos/Elastos.ELA/net/servers/httprestful"
	"github.com/elastos/Elastos.ELA/net/servers/httpwebsocket"

	"github.com/elastos/Elastos.ELA.Utility/core/transaction"
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
	blockchain.DefaultLedger = new(blockchain.Ledger)
	blockchain.DefaultLedger.Store, err = ChainStore.NewLedgerStore()
	if err != nil {
		log.Fatal("open LedgerStore err:", err)
		os.Exit(1)
	}
	defer blockchain.DefaultLedger.Store.Close()

	blockchain.DefaultLedger.Store.InitLedgerStore(blockchain.DefaultLedger)
	transaction.TxStore = blockchain.DefaultLedger.Store
	_, err = blockchain.NewBlockchainWithGenesisBlock()
	if err != nil {
		log.Fatal(err, "BlockChain generate failed")
		goto ERROR
	}

	log.Info("2. Start the P2P networks")
	noder = node.InitNode()
	noder.WaitForSyncFinish()

	servers.NodeForServers = noder
	startConsensus(noder)

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
	os.Exit(1)
}
