package dpos

import (
	"time"

	"github.com/elastos/Elastos.ELA/blockchain/interfaces"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/dpos/account"
	"github.com/elastos/Elastos.ELA/dpos/log"
	"github.com/elastos/Elastos.ELA/dpos/manager"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"
	"github.com/elastos/Elastos.ELA/dpos/store"
	"github.com/elastos/Elastos.ELA/events"
	"github.com/elastos/Elastos.ELA/mempool"
	"github.com/elastos/Elastos.ELA/p2p"
)

type ArbitratorConfig struct {
	EnableEventLog    bool
	EnableEventRecord bool
	Params            config.ArbiterConfiguration
	Arbitrators       interfaces.Arbitrators
	Store             interfaces.IDposStore
	TxMemPool         *mempool.TxPool
	BlockMemPool      *mempool.BlockPool
	ChainParams       *config.Params
	Broadcast         func(msg p2p.Message)
}

type Arbitrator struct {
	enableViewLoop bool
	network        *network
	dposManager    manager.DposManager
}

func (a *Arbitrator) Start() {
	a.network.Start()
	go a.dposManager.Recover()

	go a.changeViewLoop()
}

func (a *Arbitrator) Stop() error {
	a.enableViewLoop = false

	if err := a.network.Stop(); err != nil {
		return err
	}

	return nil
}

func (a *Arbitrator) OnIllegalBlockReceived(payload *types.PayloadIllegalBlock) {
	log.Info("[OnIllegalBlockReceived] listener received block")
	if payload.CoinType != types.ELACoin {
		a.network.PostIllegalBlocksTask(&payload.DposIllegalBlocks)
	}
}

func (a *Arbitrator) OnSidechainIllegalEvidenceReceived(data *types.SidechainIllegalData) {
	log.Info("[OnSidechainIllegalEvidenceReceived] listener received sidechain illegal evidence")
	a.network.PostSidechainIllegalDataTask(data)
}

func (a *Arbitrator) OnBlockReceived(b *types.Block, confirmed bool) {
	log.Info("[OnBlockReceived] listener received block")
	a.network.PostBlockReceivedTask(b, confirmed)
}

func (a *Arbitrator) OnConfirmReceived(p *types.DPosProposalVoteSlot) {
	log.Info("[OnConfirmReceived] listener received confirm")
	a.network.PostConfirmReceivedTask(p)
}

func (a *Arbitrator) OnNewElection(arbiters [][]byte) {
	a.network.UpdatePeers(arbiters)
}

func (a *Arbitrator) changeViewLoop() {
	for a.enableViewLoop {
		a.network.PostChangeViewTask()

		time.Sleep(1 * time.Second)
	}
}

func NewArbitrator(password []byte, cfg ArbitratorConfig) (*Arbitrator, error) {
	log.Init(cfg.Params.PrintLevel, cfg.Params.MaxPerLogSize,
		cfg.Params.MaxLogsSize)

	dposAccount, err := account.NewDposAccount(password)
	if err != nil {
		log.Error("init dpos account error")
		return nil, err
	}

	pubKey, err := common.HexStringToBytes(cfg.Params.PublicKey)
	if err != nil {
		log.Error("init dpos account error")
		return nil, err
	}
	dposManager := manager.NewManager(manager.DposManagerConfig{
		PublicKey:   pubKey,
		Arbitrators: cfg.Arbitrators,
	})
	pk := config.Parameters.GetArbiterID()
	var id peer.PID
	copy(id[:], pk)
	log.Info("ID:", common.BytesToHexString(pk))
	network, err := NewDposNetwork(id, dposManager, dposAccount)
	if err != nil {
		log.Error("Init p2p network error")
		return nil, err
	}

	eventMonitor := log.NewEventMoniter()

	if cfg.EnableEventLog {
		eventLogs := &log.EventLogs{}
		eventMonitor.RegisterListener(eventLogs)
	}

	if cfg.EnableEventRecord {
		eventRecorder := &store.EventRecord{}
		eventRecorder.Initialize(cfg.Store)
		eventMonitor.RegisterListener(eventRecorder)
	}

	dposHandlerSwitch := manager.NewHandler(network, dposManager, eventMonitor)

	consensus := manager.NewConsensus(dposManager, time.Duration(cfg.Params.SignTolerance)*time.Second, dposHandlerSwitch)
	proposalDispatcher, illegalMonitor := manager.NewDispatcherAndIllegalMonitor(
		manager.ProposalDispatcherConfig{
			EventMonitor: eventMonitor,
			Consensus:    consensus,
			Network:      network,
			Manager:      dposManager,
			Account:      dposAccount,
			ChainParams:  cfg.ChainParams,
			EventStoreAnalyzerConfig: store.EventStoreAnalyzerConfig{
				InactivePercentage: config.Parameters.ArbiterConfiguration.InactivePercentage,
				Store:              cfg.Store,
				Arbitrators:        cfg.Arbitrators,
			},
		})
	dposHandlerSwitch.Initialize(proposalDispatcher, consensus)

	dposManager.Initialize(dposHandlerSwitch, proposalDispatcher, consensus,
		network, illegalMonitor, cfg.BlockMemPool, cfg.TxMemPool, cfg.Broadcast)
	network.Initialize(manager.DposNetworkConfig{
		ProposalDispatcher: proposalDispatcher,
		Store:              cfg.Store,
	})

	cfg.Store.StartEventRecord()
	cfg.Store.StartArbitratorsRecord()

	a := Arbitrator{
		enableViewLoop: true,
		dposManager:    dposManager,
		network:        network,
	}

	events.Subscribe(func(e *events.Event) {
		switch e.Type {
		case events.ETNewBlockReceived:
			block := e.Data.(*types.DposBlock)
			a.OnBlockReceived(block.Block, block.ConfirmFlag)

		case events.ETConfirmReceived:
			a.OnConfirmReceived(e.Data.(*types.DPosProposalVoteSlot))

		case events.ETNewArbiterElection:
			a.OnNewElection(e.Data.([][]byte))

		case events.ETTransactionAccepted:
			tx := e.Data.(*types.Transaction)
			if tx.IsIllegalBlockTx() {
				a.OnIllegalBlockReceived(tx.Payload.(*types.PayloadIllegalBlock))
			}
		}
	})

	return &a, nil
}
