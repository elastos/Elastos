package dpos

import (
	"time"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/core"
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

func Start() {
	log.Info("1. Start initialize character.")
	proposalManager := &proposal.ProposalDispatcher{}
	consensusManager := &consensus.Consensus{}
	dposHandlerSwitch := &handler.DposHandlerSwitch{}
	dposManager := &manager.DposManager{}

	var id [32]byte //fixme init id with current public key
	network, err := arbitrator.NewDposNetwork(id, dposManager)
	if err != nil {
		log.Error("Start p2p network error")
	}
	dposManager.Initialize(dposHandlerSwitch, proposalManager, network)
	consensusManager.Initialize(time.Duration(config.Parameters.ArbiterConfiguration.SignTolerance)*time.Second, dposHandlerSwitch)
	dposHandlerSwitch.Initialize(proposalManager, consensusManager, network, dposManager)

	arbitrator.ArbitratorSingleton = &arbitrator.Arbitrator{
		Name:     config.Parameters.ArbiterConfiguration.Name,
		IsOnDuty: false,
		BlockCache: cache.ConsensusBlockCache{
			ConsensusBlocks: make(map[common.Uint256]*core.Block, 0),
		},
		DposManager: dposManager,
	}

	arbitrator.ArbitratorSingleton.DposManager.Recover()

	blockchain.DefaultLedger.Blockchain.NewBlocksListener = arbitrator.ArbitratorSingleton

	go dposHandlerSwitch.ChangeViewLoop()

	log.Info("2. Start loop to process message queue")
	network.Start()
}
