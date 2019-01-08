package main

import (
	"os"
	"runtime"
	"runtime/debug"
	"time"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/blockchain/interfaces"
	"github.com/elastos/Elastos.ELA/cli/password"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/dpos"
	"github.com/elastos/Elastos.ELA/dpos/store"
	"github.com/elastos/Elastos.ELA/elanet"
	"github.com/elastos/Elastos.ELA/mempool"
	"github.com/elastos/Elastos.ELA/p2p"
	"github.com/elastos/Elastos.ELA/p2p/msg"
	"github.com/elastos/Elastos.ELA/pow"
	"github.com/elastos/Elastos.ELA/servers"
	"github.com/elastos/Elastos.ELA/servers/httpjsonrpc"
	"github.com/elastos/Elastos.ELA/servers/httpnodeinfo"
	"github.com/elastos/Elastos.ELA/servers/httprestful"
	"github.com/elastos/Elastos.ELA/servers/httpwebsocket"
	"github.com/elastos/Elastos.ELA/version"
	"github.com/elastos/Elastos.ELA/version/verconf"

	"github.com/elastos/Elastos.ELA.Utility/signal"
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

	log.Infof("Node version: %s", Version)
	log.Info(GoVersion)

	var interrupt = signal.NewInterrupt()

	// fixme remove singleton Ledger
	ledger := blockchain.Ledger{}

	var dposStore interfaces.IDposStore
	chainStore, err := blockchain.NewChainStore(dataDir,
		activeNetParams.GenesisBlock)
	if err != nil {
		printErrorAndExit(err)
	}
	defer chainStore.Close()
	ledger.Store = chainStore // fixme

	dposStore, err = store.NewDposStore(dataDir)
	if err != nil {
		printErrorAndExit(err)
	}
	defer dposStore.Close()

	txMemPool := mempool.NewTxPool()
	blockMemPool := mempool.NewBlockPool()
	verconf := verconf.Config{
		ChainStore:   chainStore,
		ChainParams:  activeNetParams,
		TxMemPool:    txMemPool,
		BlockMemPool: blockMemPool,
	}
	versions := version.NewVersions(&verconf)
	ledger.HeightVersions = versions // fixme
	chain, err := blockchain.New(chainStore, activeNetParams, versions)
	if err != nil {
		printErrorAndExit(err)
	}
	verconf.Chain = chain
	ledger.Blockchain = chain          // fixme
	blockchain.DefaultLedger = &ledger // fixme

	arbiters, err := store.NewArbitrators(&store.ArbitratorsConfig{
		ArbitratorsCount: config.Parameters.ArbiterConfiguration.NormalArbitratorsCount + uint32(len(activeNetParams.CRCArbiters)),
		CandidatesCount:  config.Parameters.ArbiterConfiguration.CandidatesCount,
		MajorityCount:    config.Parameters.ArbiterConfiguration.MajorityCount,
		CRCArbitrators:   activeNetParams.CRCArbiters,
		Versions:         versions,
		Store:            dposStore,
		ChainStore:       chainStore,
	})
	if err = arbiters.Start(); err != nil {
		printErrorAndExit(err)
	}
	verconf.Arbitrators = arbiters
	ledger.Arbitrators = arbiters // fixme

	log.Info("Start the P2P networks")
	server, err := elanet.NewServer(dataDir, &elanet.Config{
		Chain:        chain,
		ChainParams:  activeNetParams,
		Versions:     versions,
		TxMemPool:    txMemPool,
		BlockMemPool: blockMemPool,
	})
	if err != nil {
		printErrorAndExit(err)
	}
	server.Start()
	defer server.Stop()
	verconf.Server = server

	if config.Parameters.EnableArbiter {
		log.Info("Start the manager")
		pwd, err := password.GetPassword()
		if err != nil {
			printErrorAndExit(err)
		}
		arbitrator, err := dpos.NewArbitrator(pwd, dpos.ArbitratorConfig{
			EnableEventLog:    true,
			EnableEventRecord: true,
			Params:            cfg.ArbiterConfiguration,
			Arbitrators:       arbiters,
			Store:             dposStore,
			TxMemPool:         txMemPool,
			BlockMemPool:      blockMemPool,
			Broadcast: func(msg p2p.Message) {
				server.BroadcastMessage(msg)
			},
		})
		if err != nil {
			printErrorAndExit(err)
		}
		defer arbitrator.Stop()
		arbitrator.Start()
	}

	servers.Chain = chain
	servers.Store = chainStore
	servers.TxMemPool = txMemPool
	servers.Server = server
	servers.Versions = versions
	servers.Arbiters = arbiters
	servers.Pow = pow.NewService(&pow.Config{
		PayToAddr:   cfg.PowConfiguration.PayToAddr,
		MinerInfo:   cfg.PowConfiguration.MinerInfo,
		Chain:       chain,
		ChainParams: activeNetParams,
		TxMemPool:   txMemPool,
		Versions:    versions,
		BroadcastBlock: func(block *types.Block) {
			hash := block.Hash()
			server.RelayInventory(msg.NewInvVect(msg.InvTypeBlock, &hash), block)
		},
	})

	log.Info("Start services")
	go httpjsonrpc.StartRPCServer()
	go httprestful.StartServer()
	go httpwebsocket.StartServer()
	if config.Parameters.HttpInfoStart {
		go httpnodeinfo.StartServer()
	}

	waitForSyncFinish(server, interrupt.C)
	if interrupt.Interrupted() {
		return
	}
	log.Info("Start consensus")
	if config.Parameters.PowConfiguration.AutoMining {
		log.Info("Start POW Services")
		go servers.Pow.Start()
	}

	<-interrupt.C
}

func printErrorAndExit(err error) {
	log.Error(err)
	os.Exit(-1)
}

func waitForSyncFinish(server elanet.Server, interrupt <-chan struct{}) {
	ticker := time.NewTicker(time.Second * 5)
	defer ticker.Stop()

out:
	for {
		select {
		case <-ticker.C:
			if server.IsCurrent() {
				break out
			}

		case <-interrupt:
			break out
		}
	}
}
