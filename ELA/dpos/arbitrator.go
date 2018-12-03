package dpos

import (
	"time"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/dpos/account"
	"github.com/elastos/Elastos.ELA/dpos/log"
	. "github.com/elastos/Elastos.ELA/dpos/manager"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"
	"github.com/elastos/Elastos.ELA/dpos/store"
	"github.com/elastos/Elastos.ELA/protocol"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type Arbitrator interface {
	blockchain.NewBlocksListener
	blockchain.ArbitratorsListener
	protocol.TxnPoolListener

	Start()
	Stop() error
}

type arbitrator struct {
	enableViewLoop bool
	network        *dposNetwork
	dposManager    DposManager
}

func (a *arbitrator) Start() {
	a.network.Start()
	go a.dposManager.Recover()

	go a.changeViewLoop()
}

func (a *arbitrator) Stop() error {
	a.enableViewLoop = false

	if err := a.network.Stop(); err != nil {
		return err
	}

	return nil
}

func (a *arbitrator) OnIllegalBlockTxnReceived(txn *types.Transaction) {
	log.Info("[OnIllegalBlockTxnReceived] listener received block")
	if txn.TxType == types.IllegalBlockEvidence {
		payload := txn.Payload.(*types.PayloadIllegalBlock)
		if payload.CoinType != types.ELACoin {
			a.network.PostIllegalBlocksTask(&payload.DposIllegalBlocks)
		}
	}
}

func (a *arbitrator) OnBlockReceived(b *types.Block, confirmed bool) {
	log.Info("[OnBlockReceived] listener received block")
	a.network.PostBlockReceivedTask(b, confirmed)
}

func (a *arbitrator) OnConfirmReceived(p *types.DPosProposalVoteSlot) {
	log.Info("[OnConfirmReceived] listener received confirm")
	a.network.PostConfirmReceivedTask(p)
}

func (a *arbitrator) OnNewElection(arbiters [][]byte) {
	a.network.UpdatePeers(arbiters)
}

func (a *arbitrator) changeViewLoop() {
	for a.enableViewLoop {
		a.network.PostChangeViewTask()

		time.Sleep(1 * time.Second)
	}
}

func NewArbitrator(password []byte) (Arbitrator, error) {
	dposAccount, err := account.NewDposAccount(password)
	if err != nil {
		log.Error("Init dpos account error")
		return nil, err
	}

	dposManager := NewManager(config.Parameters.ArbiterConfiguration.Name)
	pk := config.Parameters.GetArbiterID()
	var id peer.PID
	copy(id[:], pk)
	log.Info("ID:", common.BytesToHexString(pk))
	network, err := NewDposNetwork(id, dposManager, dposAccount)
	if err != nil {
		log.Error("Init p2p network error")
		return nil, err
	}

	eventMoniter := log.NewEventMoniter()
	eventRecorder := &store.EventRecord{}
	eventRecorder.Initialize()
	eventMoniter.RegisterListener(eventRecorder)

	dposHandlerSwitch := NewHandler(network, dposManager, eventMoniter)

	consensus := NewConsensus(dposManager, time.Duration(config.Parameters.ArbiterConfiguration.SignTolerance)*time.Second, dposHandlerSwitch)
	proposalDispatcher, illegalMonitor := NewDispatcherAndIllegalMonitor(consensus, eventMoniter, network, dposManager, dposAccount)
	dposHandlerSwitch.Initialize(proposalDispatcher, consensus)

	dposManager.Initialize(dposHandlerSwitch, proposalDispatcher, consensus, network, illegalMonitor)
	network.Initialize(proposalDispatcher)

	result := &arbitrator{
		enableViewLoop: true,
		dposManager:    dposManager,
		network:        network,
	}

	return result, nil
}

func init() {
	log.Init(
		config.Parameters.ArbiterConfiguration.PrintLevel,
		config.Parameters.ArbiterConfiguration.MaxPerLogSize,
		config.Parameters.ArbiterConfiguration.MaxLogsSize,
	)
}
