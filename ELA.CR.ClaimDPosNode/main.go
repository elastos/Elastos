// Copyright (c) 2017-2019 Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package main

import (
	"bytes"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"runtime/debug"
	"strconv"
	"time"

	"github.com/elastos/Elastos.ELA/blockchain"
	cmdcom "github.com/elastos/Elastos.ELA/cmd/common"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/types"
	crstate "github.com/elastos/Elastos.ELA/cr/state"
	"github.com/elastos/Elastos.ELA/dpos"
	"github.com/elastos/Elastos.ELA/dpos/account"
	dlog "github.com/elastos/Elastos.ELA/dpos/log"
	"github.com/elastos/Elastos.ELA/dpos/state"
	"github.com/elastos/Elastos.ELA/dpos/store"
	"github.com/elastos/Elastos.ELA/elanet"
	"github.com/elastos/Elastos.ELA/elanet/routes"
	"github.com/elastos/Elastos.ELA/mempool"
	"github.com/elastos/Elastos.ELA/p2p"
	"github.com/elastos/Elastos.ELA/p2p/msg"
	"github.com/elastos/Elastos.ELA/pow"
	"github.com/elastos/Elastos.ELA/servers"
	"github.com/elastos/Elastos.ELA/servers/httpjsonrpc"
	"github.com/elastos/Elastos.ELA/servers/httpnodeinfo"
	"github.com/elastos/Elastos.ELA/servers/httprestful"
	"github.com/elastos/Elastos.ELA/servers/httpwebsocket"
	"github.com/elastos/Elastos.ELA/utils"
	"github.com/elastos/Elastos.ELA/utils/elalog"
	"github.com/elastos/Elastos.ELA/utils/signal"
	"github.com/elastos/Elastos.ELA/wallet"

	"github.com/urfave/cli"
)

var (
	// Build version generated when build program.
	Version string

	// The go source code version at build.
	GoVersion string

	// printStateInterval is the interval to print out peer-to-peer network
	// state.
	printStateInterval = time.Minute
)

func main() {
	if err := setupNode().Run(os.Args); err != nil {
		cmdcom.PrintErrorMsg(err.Error())
		os.Exit(1)
	}
}

func setupNode() *cli.App {
	appSettings := newSettings()

	app := cli.NewApp()
	app.Name = "ela"
	app.Version = Version
	app.HelpName = "ela"
	app.Usage = "ela node for elastos blockchain"
	app.UsageText = "ela [options] [args]"
	app.Flags = []cli.Flag{
		cmdcom.ConfigFileFlag,
		cmdcom.DataDirFlag,
		cmdcom.AccountPasswordFlag,
	}
	app.Flags = append(app.Flags, appSettings.Flags()...)
	app.Action = func(c *cli.Context) {
		appSettings.SetContext(c)
		appSettings.SetupConfig()
		appSettings.InitParamsValue()
		setupLog(c, appSettings)
		startNode(c, appSettings)
	}
	app.Before = func(c *cli.Context) error {
		// Use all processor cores.
		runtime.GOMAXPROCS(runtime.NumCPU())

		// Block and transaction processing can cause bursty allocations.  This
		// limits the garbage collector from excessively overallocating during
		// bursts.  This value was arrived at with the help of profiling live
		// usage.
		debug.SetGCPercent(10)

		return nil
	}

	return app
}

func startNode(c *cli.Context, st *settings) {
	// Enable http profiling server if requested.
	if st.Config().ProfilePort != 0 {
		go utils.StartPProf(st.Config().ProfilePort)
	}

	flagDataDir := c.String("datadir")
	dataDir := filepath.Join(flagDataDir, dataPath)
	st.Params().CkpManager.SetDataPath(
		filepath.Join(dataDir, checkpointPath))

	var act account.Account
	if st.Config().DPoSConfiguration.EnableArbiter {
		password, err := cmdcom.GetFlagPassword(c)
		if err != nil {
			printErrorAndExit(err)
		}
		act, err = account.Open(password)
		if err != nil {
			printErrorAndExit(err)
		}
	}

	log.Infof("Node version: %s", Version)
	log.Info(GoVersion)

	var interrupt = signal.NewInterrupt()

	// fixme remove singleton Ledger
	ledger := blockchain.Ledger{}

	// Initializes the foundation address
	blockchain.FoundationAddress = st.Params().Foundation
	blockchain.EnableUtxoDB = st.Params().EnableUtxoDB
	chainStore, err := blockchain.NewChainStore(dataDir, st.Params().GenesisBlock)
	if err != nil {
		printErrorAndExit(err)
	}
	defer chainStore.Close()
	ledger.Store = chainStore // fixme

	var dposStore store.IDposStore
	dposStore, err = store.NewDposStore(dataDir, st.Params())
	if err != nil {
		printErrorAndExit(err)
	}
	defer dposStore.Close()

	txMemPool := mempool.NewTxPool(st.Params())
	blockMemPool := mempool.NewBlockPool(st.Params())
	blockMemPool.Store = chainStore

	blockchain.DefaultLedger = &ledger // fixme

	arbiters, err := state.NewArbitrators(st.Params(),
		chainStore.GetHeight, func(height uint32) (*types.Block, error) {
			hash, err := chainStore.GetBlockHash(height)
			if err != nil {
				return nil, err
			}
			block, err := chainStore.GetBlock(hash)
			if err != nil {
				return nil, err
			}
			blockchain.CalculateTxsFee(block)
			return block, nil
		}, func(programHash common.Uint168) (common.Fixed64,
			error) {
			amount := common.Fixed64(0)
			utxos, err := blockchain.DefaultLedger.Store.
				GetUnspentFromProgramHash(programHash, config.ELAAssetID)
			if err != nil {
				return amount, err
			}
			for _, utxo := range utxos {
				amount += utxo.Value
			}
			return amount, nil
		})
	if err != nil {
		printErrorAndExit(err)
	}
	ledger.Arbitrators = arbiters // fixme

	committee := crstate.NewCommittee(st.Params())
	ledger.Committee = committee

	chain, err := blockchain.New(chainStore, st.Params(), arbiters.State,
		committee)
	if err != nil {
		printErrorAndExit(err)
	}
	ledger.Blockchain = chain // fixme
	blockMemPool.Chain = chain

	routesCfg := &routes.Config{TimeSource: chain.TimeSource}
	if act != nil {
		routesCfg.PID = act.PublicKeyBytes()
		routesCfg.Addr = fmt.Sprintf("%s:%d",
			st.Config().DPoSConfiguration.IPAddress,
			st.Config().DPoSConfiguration.DPoSPort)
		routesCfg.Sign = act.Sign
	}

	route := routes.New(routesCfg)
	server, err := elanet.NewServer(dataDir, &elanet.Config{
		Chain:          chain,
		ChainParams:    st.Params(),
		PermanentPeers: st.Params().PermanentPeers,
		TxMemPool:      txMemPool,
		BlockMemPool:   blockMemPool,
		Routes:         route,
	})
	if err != nil {
		printErrorAndExit(err)
	}
	routesCfg.IsCurrent = server.IsCurrent
	routesCfg.RelayAddr = server.RelayInventory
	blockMemPool.IsCurrent = server.IsCurrent

	var arbitrator *dpos.Arbitrator
	if act != nil {
		dlog.Init(uint8(st.Config().PrintLevel), st.Config().MaxPerLogSize, st.Config().MaxLogsSize)
		arbitrator, err = dpos.NewArbitrator(act, dpos.Config{
			EnableEventLog:    true,
			EnableEventRecord: false,
			Localhost:         st.Config().DPoSConfiguration.IPAddress,
			ChainParams:       st.Params(),
			Arbitrators:       arbiters,
			Store:             dposStore,
			Server:            server,
			TxMemPool:         txMemPool,
			BlockMemPool:      blockMemPool,
			Broadcast: func(msg p2p.Message) {
				server.BroadcastMessage(msg)
			},
			AnnounceAddr: route.AnnounceAddr,
		})
		if err != nil {
			printErrorAndExit(err)
		}
		routesCfg.OnCipherAddr = arbitrator.OnCipherAddr
		servers.Arbiter = arbitrator
		arbitrator.Start()
		defer arbitrator.Stop()
	}

	wal := wallet.NewWallet()
	wallet.Store = chainStore
	wallet.ChainParam = st.Params()

	st.Params().CkpManager.Register(wal)

	servers.Compile = Version
	servers.Config = st.Config()
	servers.ChainParams = st.Params()
	servers.Chain = chain
	servers.Store = chainStore
	servers.TxMemPool = txMemPool
	servers.Server = server
	servers.Arbiters = arbiters
	servers.Wallet = wal
	servers.Pow = pow.NewService(&pow.Config{
		PayToAddr:   st.Config().PowConfiguration.PayToAddr,
		MinerInfo:   st.Config().PowConfiguration.MinerInfo,
		Chain:       chain,
		ChainParams: st.Params(),
		TxMemPool:   txMemPool,
		BlkMemPool:  blockMemPool,
		BroadcastBlock: func(block *types.Block) {
			hash := block.Hash()
			server.RelayInventory(msg.NewInvVect(msg.InvTypeBlock, &hash), block)
		},
		Arbitrators: arbiters,
	})

	// initialize producer state after arbiters has initialized.
	if err = chain.InitCheckpoint(interrupt.C, pgBar.Start,
		pgBar.Increase); err != nil {
		printErrorAndExit(err)
	}
	pgBar.Stop()

	log.Info("Start the P2P networks")
	server.Start()
	defer server.Stop()

	log.Info("Start services")
	if st.Config().EnableRPC {
		go httpjsonrpc.StartRPCServer()
	}
	if st.Config().HttpRestStart {
		go httprestful.StartServer()
	}
	if st.Config().HttpWsStart {
		go httpwebsocket.Start()
	}
	if st.Config().HttpInfoStart {
		go httpnodeinfo.StartServer()
	}

	go printSyncState(chainStore, server)

	waitForSyncFinish(server, interrupt.C)
	if interrupt.Interrupted() {
		return
	}
	log.Info("Start consensus")
	if st.Config().PowConfiguration.AutoMining {
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

func printSyncState(db blockchain.IChainStore, server elanet.Server) {
	statlog := elalog.NewBackend(logger.Writer()).Logger("STAT",
		elalog.LevelInfo)

	ticker := time.NewTicker(printStateInterval)
	defer ticker.Stop()

	for range ticker.C {
		var buf bytes.Buffer
		buf.WriteString("-> ")
		buf.WriteString(strconv.FormatUint(uint64(db.GetHeight()), 10))
		peers := server.ConnectedPeers()
		buf.WriteString(" [")
		for i, p := range peers {
			buf.WriteString(strconv.FormatUint(uint64(p.ToPeer().Height()), 10))
			buf.WriteString(" ")
			buf.WriteString(p.ToPeer().String())
			if i != len(peers)-1 {
				buf.WriteString(", ")
			}
		}
		buf.WriteString("]")
		statlog.Info(buf.String())
	}
}
