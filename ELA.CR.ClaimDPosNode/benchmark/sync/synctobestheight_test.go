// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package sync

import (
	"bytes"
	"flag"
	"fmt"
	"os"
	"os/exec"
	"path"
	"path/filepath"
	"runtime"
	"strconv"
	"testing"
	"time"

	"github.com/elastos/Elastos.ELA/blockchain"
	cmdcom "github.com/elastos/Elastos.ELA/cmd/common"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config/settings"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/types"
	crstate "github.com/elastos/Elastos.ELA/cr/state"
	"github.com/elastos/Elastos.ELA/dpos/state"
	"github.com/elastos/Elastos.ELA/elanet"
	"github.com/elastos/Elastos.ELA/elanet/netsync"
	"github.com/elastos/Elastos.ELA/elanet/peer"
	"github.com/elastos/Elastos.ELA/elanet/routes"
	"github.com/elastos/Elastos.ELA/mempool"
	"github.com/elastos/Elastos.ELA/p2p"
	"github.com/elastos/Elastos.ELA/p2p/addrmgr"
	"github.com/elastos/Elastos.ELA/p2p/connmgr"
	"github.com/elastos/Elastos.ELA/p2p/msg"
	"github.com/elastos/Elastos.ELA/pow"
	"github.com/elastos/Elastos.ELA/servers"
	"github.com/elastos/Elastos.ELA/utils"
	"github.com/elastos/Elastos.ELA/utils/elalog"
	"github.com/elastos/Elastos.ELA/utils/signal"
	"github.com/elastos/Elastos.ELA/utils/test"
	"github.com/elastos/Elastos.ELA/wallet"

	"github.com/urfave/cli"
)

const (
	configPath     = "config.json"
	magic          = "201912"
	dataPath       = "data"
	checkpointPath = "checkpoints"

	srcAppName  = "ela"
	srcNodePort = "20086"

	dstNodePort    = "20087"
	dstDataDir     = "temp"
	dstNodeLogPath = "logs/node"
)

var (
	srcApp = startSrcNode()
	srcDir string

	dstSettings = initDstSettings()
	dstContext  *cli.Context
	logger      *log.Logger
)

func Benchmark_Sync_ToBestHeight(b *testing.B) {
	startDstNode()
	endNodes()
}

func startDstNode() {
	// Enable http profiling server if requested.
	if dstSettings.Config().ProfilePort != 0 {
		go utils.StartPProf(dstSettings.Config().ProfilePort)
	}

	flagDataDir := dstContext.String("datadir")
	dataDir := filepath.Join(flagDataDir, dataPath)
	dstSettings.Params().CkpManager.SetDataPath(
		filepath.Join(dataDir, checkpointPath))
	dstSettings.Params().CkpManager.SetNeedSave(true)

	var interrupt = signal.NewInterrupt()

	ledger := blockchain.Ledger{}

	// Initializes the foundation address
	blockchain.FoundationAddress = dstSettings.Params().Foundation

	chainStore, err := blockchain.NewChainStore(dataDir, dstSettings.Params())
	if err != nil {
		logger.Error(err)
		return
	}
	defer chainStore.Close()
	ledger.Store = chainStore

	txMemPool := mempool.NewTxPool(dstSettings.Params())
	blockMemPool := mempool.NewBlockPool(dstSettings.Params())
	blockMemPool.Store = chainStore

	blockchain.DefaultLedger = &ledger

	committee := crstate.NewCommittee(dstSettings.Params())
	ledger.Committee = committee

	arbiters, err := state.NewArbitrators(dstSettings.Params(), committee,
		func(programHash common.Uint168) (common.Fixed64,
			error) {
			amount := common.Fixed64(0)
			utxos, err := blockchain.DefaultLedger.Store.
				GetFFLDB().GetUTXO(&programHash)
			if err != nil {
				return amount, err
			}
			for _, utxo := range utxos {
				amount += utxo.Value
			}
			return amount, nil
		})
	if err != nil {
		logger.Error(err)
		return
	}
	ledger.Arbitrators = arbiters

	chain, err := blockchain.New(chainStore, dstSettings.Params(), arbiters.State, committee)
	if err != nil {
		logger.Error(err)
		return
	}
	if err := chain.Init(interrupt.C); err != nil {
		logger.Error(err)
		return
	}
	if err := chain.MigrateOldDB(interrupt.C, func(uint32) {},
		func() {}, dataDir, dstSettings.Params()); err != nil {
		logger.Error(err)
		return
	}
	ledger.Blockchain = chain
	blockMemPool.Chain = chain
	arbiters.RegisterFunction(chain.GetHeight,
		func(height uint32) (*types.Block, error) {
			hash, err := chain.GetBlockHash(height)
			if err != nil {
				return nil, err
			}
			block, err := chainStore.GetFFLDB().GetBlock(hash)
			if err != nil {
				return nil, err
			}
			blockchain.CalculateTxsFee(block.Block)
			return block.Block, nil
		}, chain.UTXOCache.GetTxReference)

	routesCfg := &routes.Config{TimeSource: chain.TimeSource}

	route := routes.New(routesCfg)
	server, err := elanet.NewServer(dataDir, &elanet.Config{
		Chain:          chain,
		ChainParams:    dstSettings.Params(),
		PermanentPeers: dstSettings.Params().PermanentPeers,
		TxMemPool:      txMemPool,
		BlockMemPool:   blockMemPool,
		Routes:         route,
	}, "")
	if err != nil {
		logger.Error(err)
		return
	}
	routesCfg.IsCurrent = server.IsCurrent
	routesCfg.RelayAddr = server.RelayInventory
	blockMemPool.IsCurrent = server.IsCurrent

	committee.RegisterFuncitons(&crstate.CommitteeFuncsConfig{
		GetTxReference:                   chain.UTXOCache.GetTxReference,
		GetUTXO:                          chainStore.GetFFLDB().GetUTXO,
		GetHeight:                        chainStore.GetHeight,
		CreateCRAppropriationTransaction: chain.CreateCRCAppropriationTransaction,
		IsCurrent:                        server.IsCurrent,
		Broadcast: func(msg p2p.Message) {
			server.BroadcastMessage(msg)
		},
		AppendToTxpool: txMemPool.AppendToTxPool,
	})

	wal := wallet.NewWallet()
	wallet.Store = chainStore
	wallet.ChainParam = dstSettings.Params()
	wallet.Chain = chain

	dstSettings.Params().CkpManager.Register(wal)

	servers.Compile = "benchmark"
	servers.Config = dstSettings.Config()
	servers.ChainParams = dstSettings.Params()
	servers.Chain = chain
	servers.Store = chainStore
	servers.TxMemPool = txMemPool
	servers.Server = server
	servers.Arbiters = arbiters
	servers.Wallet = wal
	servers.Pow = pow.NewService(&pow.Config{
		PayToAddr:   dstSettings.Config().PowConfiguration.PayToAddr,
		MinerInfo:   dstSettings.Config().PowConfiguration.MinerInfo,
		Chain:       chain,
		ChainParams: dstSettings.Params(),
		TxMemPool:   txMemPool,
		BlkMemPool:  blockMemPool,
		BroadcastBlock: func(block *types.Block) {
			hash := block.Hash()
			server.RelayInventory(msg.NewInvVect(msg.InvTypeBlock, &hash), block)
		},
		Arbitrators: arbiters,
	})

	// initialize producer state after arbiters has initialized.
	if err = chain.InitCheckpoint(interrupt.C, func(uint32) {},
		func() {}); err != nil {
		logger.Error(err)
		return
	}

	log.Info("Start the P2P networks")
	server.Start()
	defer server.Stop()

	go printSyncState(chain, server)
	waitForSyncFinish(server, interrupt.C)
}

func startSrcNode() *exec.Cmd {
	_, filename, _, _ := runtime.Caller(0)
	srcDir = path.Dir(filename)

	app := exec.Command(path.Join(srcDir, srcAppName), getSrcRunArgs()...)
	if err := app.Start(); err != nil {
		fmt.Println(err)
		return nil
	}

	return app
}

func endNodes() {
	os.RemoveAll(dstDataDir)
	if srcApp != nil && srcApp.Process != nil {
		srcApp.Process.Kill()
	}
}

func getPeerAddr(port string) string {
	return "127.0.0.1" + ":" + port
}

func getSrcRunArgs() []string {
	return []string{
		"--magic", magic,
		"--conf", path.Join(srcDir, configPath),
		"--datadir", path.Join(srcDir, test.DataDir),
		"--peers", getPeerAddr(dstNodePort),
		"--port", srcNodePort,
		"--arbiter", "false",
		"--server", "false",
		"--automining", "false",
	}
}

func newCliContext(st *settings.Settings) *cli.Context {
	ctx := cli.NewContext(nil, newFlagSet(st), nil)
	ctx.Set("magic", magic)
	ctx.Set("conf", configPath)
	ctx.Set("datadir", dstDataDir)
	ctx.Set("peers", getPeerAddr(srcNodePort))
	ctx.Set("port", dstNodePort)
	ctx.Set("arbiter", "false")
	ctx.Set("server", "false")
	ctx.Set("automining", "false")
	return ctx
}

func initDstSettings() *settings.Settings {
	st := settings.NewSettings()
	dstContext = newCliContext(st)
	st.SetContext(dstContext)
	st.SetupConfig()
	st.InitParamsValue()
	setupLog(dstContext, st)
	return st
}

func newFlagSet(st *settings.Settings) *flag.FlagSet {
	originFlags := []cli.StringFlag{
		cmdcom.ConfigFileFlag,
		cmdcom.DataDirFlag,
		cmdcom.AccountPasswordFlag,
		cmdcom.TestNetFlag,
		cmdcom.RegTestFlag,
		cmdcom.InfoPortFlag,
		cmdcom.RestPortFlag,
		cmdcom.WsPortFlag,
		cmdcom.InstantBlockFlag,
		cmdcom.RPCPortFlag,
	}
	result := &flag.FlagSet{}

	for _, v := range originFlags {
		result.String(v.GetName(), v.GetValue(), v.GetUsage())
	}

	for _, v := range st.Flags() {
		if strFlag, ok := v.(cli.StringFlag); ok {
			result.String(strFlag.GetName(), strFlag.GetValue(),
				strFlag.GetUsage())
		}
	}
	return result
}

func waitForSyncFinish(server elanet.Server, interrupt <-chan struct{}) {
	ticker := time.NewTicker(time.Second * 5)
	defer ticker.Stop()

out:
	for {
		select {
		case <-ticker.C:
			if len(server.ConnectedPeers()) > 0 && server.IsCurrent() {
				break out
			}

		case <-interrupt:
			break out
		}
	}
}

func setupLog(c *cli.Context, s *settings.Settings) {
	flagDataDir := c.String("datadir")
	path := filepath.Join(flagDataDir, dstNodeLogPath)

	logger = log.NewDefault(path, uint8(s.Config().PrintLevel),
		s.Config().MaxPerLogSize, s.Config().MaxLogsSize)

	addrmgr.UseLogger(logger)
	connmgr.UseLogger(logger)
	netsync.UseLogger(logger)
	peer.UseLogger(logger)
	routes.UseLogger(logger)
	elanet.UseLogger(logger)
	state.UseLogger(logger)
	crstate.UseLogger(logger)
}

func printSyncState(bc *blockchain.BlockChain, server elanet.Server) {
	statlog := elalog.NewBackend(logger.Writer()).Logger("STAT",
		elalog.LevelInfo)

	ticker := time.NewTicker(time.Second * 20)
	defer ticker.Stop()

	for range ticker.C {
		var buf bytes.Buffer
		buf.WriteString("-> ")
		buf.WriteString(strconv.FormatUint(uint64(bc.GetHeight()), 10))
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
