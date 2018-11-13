package dpos

import (
	"io"
	"os"
	"time"

	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/dpos/arbitration/cs"
	"github.com/elastos/Elastos.ELA/dpos/chain"
	"github.com/elastos/Elastos.ELA/dpos/config"
	"github.com/elastos/Elastos.ELA/dpos/dpos/arbitrator"
	"github.com/elastos/Elastos.ELA/dpos/dpos/cache"
	"github.com/elastos/Elastos.ELA/dpos/dpos/consensus"
	"github.com/elastos/Elastos.ELA/dpos/dpos/handler"
	"github.com/elastos/Elastos.ELA/dpos/dpos/manager"
	"github.com/elastos/Elastos.ELA/dpos/dpos/proposal"
	"github.com/elastos/Elastos.ELA/dpos/log"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/elalog"
	"github.com/elastos/Elastos.ELA.Utility/p2p/addrmgr"
	"github.com/elastos/Elastos.ELA.Utility/p2p/connmgr"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA.Utility/p2p/peer"
)

const (
	SPVLogOutputPath = "./SPVLogs/" // The spv log files output path

	defaultSpvMaxPerLogFileSize int64 = elalog.MBSize * 20
	defaultSpvMaxLogsFolderSize int64 = elalog.GBSize * 2
)

func init() {
	config.Init()
	log.Init(log.Path, log.Stdout)

	spvMaxPerLogFileSize := defaultSpvMaxPerLogFileSize
	spvMaxLogsFolderSize := defaultSpvMaxLogsFolderSize
	if config.Parameters.MaxPerLogSize > 0 {
		spvMaxPerLogFileSize = int64(config.Parameters.MaxPerLogSize) * elalog.MBSize
	}
	if config.Parameters.MaxLogSize > 0 {
		spvMaxLogsFolderSize = int64(config.Parameters.MaxLogSize) * elalog.MBSize
	}
	spvLogPath := SPVLogOutputPath
	if config.Parameters.SPVLogPath != "" {
		spvLogPath = config.Parameters.SPVLogPath
	}
	fileWriter := elalog.NewFileWriter(
		spvLogPath,
		spvMaxPerLogFileSize,
		spvMaxLogsFolderSize,
	)
	logWriter := io.MultiWriter(os.Stdout, fileWriter)
	level := elalog.Level(config.Parameters.SpvPrintLevel)
	backend := elalog.NewBackend(logWriter, elalog.Llongfile)

	addrlog := backend.Logger("ADDR", level)
	connlog := backend.Logger("CONN", level)
	peerlog := backend.Logger("PEER", level)

	addrmgr.UseLogger(addrlog)
	connmgr.UseLogger(connlog)
	peer.UseLogger(peerlog)
}

func initP2P() error {
	if err := cs.InitP2PClient(); err != nil {
		return err
	}

	cs.P2PClientSingleton.Start()
	return nil
}

func Start() {
	log.Info("1. Start arbitrator P2P networks.")
	if err := initP2P(); err != nil {
		log.Fatal(err)
		os.Exit(1)
	}

	log.Info("2. Start initialize character.")
	proposalManager := &proposal.ProposalDispatcher{}
	consensusManager := &consensus.Consensus{}
	dposHandlerSwitch := &handler.DposHandlerSwitch{}
	dposManager := &manager.DposManager{}
	dposManager.Initialize(dposHandlerSwitch)
	peerConnectionPool := &arbitrator.PeerConnectionPoolImpl{Listener: dposManager}
	consensusManager.Initialize(time.Duration(config.Parameters.SignTolerance)*time.Second, proposalManager, dposHandlerSwitch)
	dposHandlerSwitch.Initialize(proposalManager, consensusManager, dposManager)

	arbitrator.ArbitratorSingleton = &arbitrator.Arbitrator{
		Name:     config.Parameters.Name,
		IsOnDuty: false,
		Leger: chain.Ledger{
			BlockMap:             make(map[common.Uint256]*core.Block),
			BlockConfirmMap:      make(map[common.Uint256]*msg.DPosProposalVoteSlot),
			PendingBlockConfirms: make(map[common.Uint256]*msg.DPosProposalVoteSlot),
		},
		BlockCache: cache.ConsensusBlockCache{
			ConsensusBlocks: make(map[common.Uint256]*core.Block, 0),
		},
		DposManager: dposManager,
	}
	cs.P2PClientSingleton.PeerHandler = peerConnectionPool

	arbitrator.ArbitratorGroupSingleton.Initialize(arbitrator.ArbitratorSingleton)
	arbitrator.ArbitratorSingleton.DposManager.Recover()

	go dposHandlerSwitch.ChangeViewLoop()
	go consensusManager.StartHeartHeat()

	log.Info("3. Start loop to process message queue")
	cs.P2PClientSingleton.Work()
}
