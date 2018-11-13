package dpos

import (
	"os"
	"time"

	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/dpos/arbitration/cs"
	"github.com/elastos/Elastos.ELA/dpos/dpos/arbitrator"
	"github.com/elastos/Elastos.ELA/dpos/dpos/cache"
	"github.com/elastos/Elastos.ELA/dpos/dpos/consensus"
	"github.com/elastos/Elastos.ELA/dpos/dpos/handler"
	"github.com/elastos/Elastos.ELA/dpos/dpos/manager"
	"github.com/elastos/Elastos.ELA/dpos/dpos/proposal"
	"github.com/elastos/Elastos.ELA/dpos/log"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

func init() {
	log.Init(
		config.Parameters.ArbiterConfiguration.PrintLevel,
		config.Parameters.ArbiterConfiguration.MaxPerLogSize,
		config.Parameters.ArbiterConfiguration.MaxLogsSize,
	)
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
	consensusManager.Initialize(time.Duration(config.Parameters.ArbiterConfiguration.SignTolerance)*time.Second, proposalManager, dposHandlerSwitch)
	dposHandlerSwitch.Initialize(proposalManager, consensusManager, dposManager)

	arbitrator.ArbitratorSingleton = &arbitrator.Arbitrator{
		Name:     config.Parameters.ArbiterConfiguration.Name,
		IsOnDuty: false,
		BlockCache: cache.ConsensusBlockCache{
			ConsensusBlocks: make(map[common.Uint256]*core.Block, 0),
		},
		DposManager: dposManager,
	}
	cs.P2PClientSingleton.PeerHandler = peerConnectionPool

	arbitrator.ArbitratorSingleton.DposManager.Recover()

	go dposHandlerSwitch.ChangeViewLoop()
	go consensusManager.StartHeartHeat()

	log.Info("3. Start loop to process message queue")
	cs.P2PClientSingleton.Work()
}
