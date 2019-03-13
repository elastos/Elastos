package dpos

import (
	"bytes"
	"time"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/dpos/account"
	"github.com/elastos/Elastos.ELA/dpos/log"
	"github.com/elastos/Elastos.ELA/dpos/manager"
	dposp2p "github.com/elastos/Elastos.ELA/dpos/p2p"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"
	"github.com/elastos/Elastos.ELA/dpos/state"
	"github.com/elastos/Elastos.ELA/dpos/store"
	"github.com/elastos/Elastos.ELA/events"
	"github.com/elastos/Elastos.ELA/mempool"
	"github.com/elastos/Elastos.ELA/p2p"
)

type Config struct {
	EnableEventLog    bool
	EnableEventRecord bool
	Params            config.ArbiterConfiguration
	Arbitrators       state.Arbitrators
	Store             store.IDposStore
	TxMemPool         *mempool.TxPool
	BlockMemPool      *mempool.BlockPool
	ChainParams       *config.Params
	Broadcast         func(msg p2p.Message)
}

type Arbitrator struct {
	cfg            Config
	enableViewLoop bool
	network        *network
	dposManager    *manager.DPOSManager
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

func (a *Arbitrator) GetDPOSPeersInfo() []*dposp2p.PeerInfo {
	return a.network.p2pServer.DumpPeersInfo()
}

func (a *Arbitrator) OnIllegalBlockTxReceived(p *payload.DPOSIllegalBlocks) {
	log.Info("[OnIllegalBlockTxReceived] listener received illegal block tx")
	if p.CoinType != payload.ELACoin {
		a.network.PostIllegalBlocksTask(p)
	}
}

func (a *Arbitrator) OnInactiveArbitratorsTxReceived(
	p *payload.InactiveArbitrators) {
	log.Info("[OnInactiveArbitratorsTxReceived] listener received " +
		"inactive arbitrators tx")

	if !a.cfg.Arbitrators.IsArbitrator(a.dposManager.GetPublicKey()) {
		isEmergencyCandidate := false

		candidates := a.cfg.Arbitrators.GetCandidates()
		for i := 0; i < len(candidates) && i < int(a.cfg.Params.
			InactiveEliminateCount); i++ {
			if bytes.Equal(candidates[i], a.dposManager.GetPublicKey()) {
				isEmergencyCandidate = true
			}
		}

		if isEmergencyCandidate {
			if err := a.cfg.Arbitrators.ProcessSpecialTxPayload(p,
				blockchain.DefaultLedger.Blockchain.GetHeight()); err != nil {
				log.Error("[OnInactiveArbitratorsTxReceived] force change "+
					"arbitrators error: ", err)
			}
		}
	}
}

func (a *Arbitrator) OnSidechainIllegalEvidenceReceived(
	data *payload.SidechainIllegalData) {
	log.Info("[OnSidechainIllegalEvidenceReceived] listener received" +
		" sidechain illegal evidence")
	a.network.PostSidechainIllegalDataTask(data)
}

func (a *Arbitrator) OnBlockReceived(b *types.Block, confirmed bool) {
	log.Info("[OnBlockReceived] listener received block")
	a.network.PostBlockReceivedTask(b, confirmed)
}

func (a *Arbitrator) OnConfirmReceived(p *payload.Confirm) {
	log.Info("[OnConfirmReceived] listener received confirm")
	a.network.PostConfirmReceivedTask(p)
}

func (a *Arbitrator) OnNewElection(arbiters map[string]struct{}) {
	if err := a.network.UpdatePeers(arbiters); err != nil {
		log.Warn("[OnNewElection] update peers error: ", err)
	}
}

func (a *Arbitrator) changeViewLoop() {
	for a.enableViewLoop {
		a.network.PostChangeViewTask()

		time.Sleep(1 * time.Second)
	}
}

func NewArbitrator(password []byte, cfg Config) (*Arbitrator, error) {
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
	dposManager := manager.NewManager(manager.DPOSManagerConfig{
		PublicKey:   pubKey,
		Arbitrators: cfg.Arbitrators,
		ChainParams: cfg.ChainParams,
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

	eventMonitor := log.NewEventMonitor()

	if cfg.EnableEventLog {
		eventLogs := &log.EventLogs{}
		eventMonitor.RegisterListener(eventLogs)
	}

	if cfg.EnableEventRecord {
		eventRecorder := &store.EventRecord{}
		eventRecorder.Initialize(cfg.Store)
		eventMonitor.RegisterListener(eventRecorder)
	}

	dposHandlerSwitch := manager.NewHandler(manager.DPOSHandlerConfig{
		Network:     network,
		Manager:     dposManager,
		Monitor:     eventMonitor,
		Arbitrators: cfg.Arbitrators,
	})

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
				InactiveEliminateCount: cfg.ChainParams.InactiveEliminateCount,
				Store:                  cfg.Store,
				Arbitrators:            cfg.Arbitrators,
			},
		})
	dposHandlerSwitch.Initialize(proposalDispatcher, consensus)

	dposManager.Initialize(dposHandlerSwitch, proposalDispatcher, consensus,
		network, illegalMonitor, cfg.BlockMemPool, cfg.TxMemPool, cfg.Broadcast)
	network.Initialize(manager.DPOSNetworkConfig{
		ProposalDispatcher: proposalDispatcher,
		Store:              cfg.Store,
		PublicKey:          pubKey,
	})

	cfg.Store.StartEventRecord()
	cfg.Store.StartArbitratorsRecord()

	a := Arbitrator{
		enableViewLoop: true,
		dposManager:    dposManager,
		network:        network,
		cfg:            cfg,
	}

	events.Subscribe(func(e *events.Event) {
		switch e.Type {
		case events.ETNewBlockReceived:
			block := e.Data.(*types.DposBlock)
			a.OnBlockReceived(block.Block, block.HaveConfirm)

		case events.ETConfirmAccepted:
			a.OnConfirmReceived(e.Data.(*payload.Confirm))

		case events.ETNewArbiterElection:
			a.OnNewElection(e.Data.(map[string]struct{}))

		case events.ETTransactionAccepted:
			tx := e.Data.(*types.Transaction)
			if tx.IsIllegalBlockTx() {
				a.OnIllegalBlockTxReceived(tx.Payload.(*payload.DPOSIllegalBlocks))
			} else if tx.IsInactiveArbitrators() {
				a.OnInactiveArbitratorsTxReceived(tx.Payload.(*payload.InactiveArbitrators))
			}
		}
	})

	return &a, nil
}
