// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package dpos

import (
	"bytes"
	"time"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/dpos/account"
	"github.com/elastos/Elastos.ELA/dpos/dtime"
	"github.com/elastos/Elastos.ELA/dpos/log"
	"github.com/elastos/Elastos.ELA/dpos/manager"
	dp2p "github.com/elastos/Elastos.ELA/dpos/p2p"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"
	"github.com/elastos/Elastos.ELA/dpos/state"
	"github.com/elastos/Elastos.ELA/elanet"
	"github.com/elastos/Elastos.ELA/events"
	"github.com/elastos/Elastos.ELA/mempool"
	"github.com/elastos/Elastos.ELA/p2p"
)

type Config struct {
	EnableEventLog bool
	Arbitrators    state.Arbitrators
	Server         elanet.Server
	TxMemPool      *mempool.TxPool
	BlockMemPool   *mempool.BlockPool
	ChainParams    *config.Params
	Broadcast      func(msg p2p.Message)
	AnnounceAddr   func()
}

type Arbitrator struct {
	cfg            Config
	account        account.Account
	enableViewLoop bool
	network        *network
	dposManager    *manager.DPOSManager
}

func (a *Arbitrator) Start() {
	a.network.Start()

	go a.changeViewLoop()
	go a.recover()
}

func (a *Arbitrator) recover() {
	for {
		if a.cfg.Server.IsCurrent() && a.dposManager.GetArbitrators().
			HasArbitersMinorityCount(len(a.network.GetActivePeers())) {
			a.network.recoverChan <- true
			return
		}
		time.Sleep(time.Second)
	}
}

func (a *Arbitrator) Stop() error {
	a.enableViewLoop = false

	if err := a.network.Stop(); err != nil {
		return err
	}

	return nil
}

func (a *Arbitrator) GetCurrentCRCs() []*state.ArbiterInfo {
	return a.dposManager.GetArbitrators().GetCRCArbiters()
}

func (a *Arbitrator) GetNextCRCs() [][]byte {
	return a.dposManager.GetArbitrators().GetNextCRCArbiters()
}

func (a *Arbitrator) GetArbiterPeersInfo() []*dp2p.PeerInfo {
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
		inactiveEliminateCount := a.cfg.Arbitrators.GetArbitersCount() / 3
		for i := 0; i < len(candidates) && i < inactiveEliminateCount; i++ {
			if bytes.Equal(candidates[i], a.dposManager.GetPublicKey()) {
				isEmergencyCandidate = true
			}
		}

		if isEmergencyCandidate {
			if err := a.cfg.Arbitrators.ProcessSpecialTxPayload(p,
				blockchain.DefaultLedger.Blockchain.GetHeight()); err != nil {
				log.Error("[OnInactiveArbitratorsTxReceived] force change "+
					"arbitrators error: ", err)
				return
			}
			go a.recover()
		}
	} else {
		a.network.PostInactiveArbitersTask(p)
	}
}

func (a *Arbitrator) OnSidechainIllegalEvidenceReceived(
	data *payload.SidechainIllegalData) {
	log.Info("[OnSidechainIllegalEvidenceReceived] listener received" +
		" sidechain illegal evidence")
	a.network.PostSidechainIllegalDataTask(data)
}

func (a *Arbitrator) OnBlockReceived(b *types.Block, confirmed bool) {
	if !a.cfg.Server.IsCurrent() {
		return
	}
	log.Info("[OnBlockReceived] listener received block")
	a.network.PostBlockReceivedTask(b, confirmed)
}

func (a *Arbitrator) OnConfirmReceived(p *mempool.ConfirmInfo) {
	if !a.cfg.Server.IsCurrent() {
		return
	}
	log.Info("[OnConfirmReceived] listener received confirm")
	a.network.PostConfirmReceivedTask(p)
}

func (a *Arbitrator) OnPeersChanged(peers []peer.PID) {
	a.network.UpdatePeers(peers)
}

func (a *Arbitrator) changeViewLoop() {
	for a.enableViewLoop {
		a.network.PostChangeViewTask()

		time.Sleep(1 * time.Second)
	}
}

// OnCipherAddr will be invoked when an address cipher received.
func (a *Arbitrator) OnCipherAddr(pid peer.PID, cipher []byte) {
	addr, err := a.account.DecryptAddr(cipher)
	if err != nil {
		log.Errorf("decrypt address cipher error %s", err)
		return
	}
	a.network.p2pServer.AddAddr(pid, addr)
}

func NewArbitrator(account account.Account, cfg Config) (*Arbitrator, error) {
	medianTime := dtime.NewMedianTime()
	dposManager := manager.NewManager(manager.DPOSManagerConfig{
		PublicKey:   account.PublicKeyBytes(),
		Arbitrators: cfg.Arbitrators,
		ChainParams: cfg.ChainParams,
		TimeSource:  medianTime,
		Server:      cfg.Server,
	})

	network, err := NewDposNetwork(NetworkConfig{
		ChainParams: cfg.ChainParams,
		Account:     account,
		MedianTime:  medianTime,
		Listener:    dposManager,
	})
	if err != nil {
		log.Error("Init p2p network error")
		return nil, err
	}

	eventMonitor := log.NewEventMonitor()

	if cfg.EnableEventLog {
		eventLogs := &log.EventLogs{}
		eventMonitor.RegisterListener(eventLogs)
	}

	dposHandlerSwitch := manager.NewHandler(manager.DPOSHandlerConfig{
		Network:     network,
		Manager:     dposManager,
		Monitor:     eventMonitor,
		Arbitrators: cfg.Arbitrators,
		TimeSource:  medianTime,
	})

	consensus := manager.NewConsensus(dposManager, cfg.ChainParams.ToleranceDuration, dposHandlerSwitch)
	proposalDispatcher, illegalMonitor := manager.NewDispatcherAndIllegalMonitor(
		manager.ProposalDispatcherConfig{
			EventMonitor: eventMonitor,
			Consensus:    consensus,
			Network:      network,
			Manager:      dposManager,
			Account:      account,
			ChainParams:  cfg.ChainParams,
			TimeSource:   medianTime,
			EventAnalyzerConfig: manager.EventAnalyzerConfig{
				Arbitrators: cfg.Arbitrators,
			},
		})
	dposHandlerSwitch.Initialize(proposalDispatcher, consensus)

	dposManager.Initialize(dposHandlerSwitch, proposalDispatcher, consensus,
		network, illegalMonitor, cfg.BlockMemPool, cfg.TxMemPool, cfg.Broadcast)
	network.Initialize(manager.DPOSNetworkConfig{
		ProposalDispatcher: proposalDispatcher,
		PublicKey:          account.PublicKeyBytes(),
		AnnounceAddr:       cfg.AnnounceAddr,
	})

	a := Arbitrator{
		cfg:            cfg,
		account:        account,
		enableViewLoop: true,
		dposManager:    dposManager,
		network:        network,
	}

	events.Subscribe(func(e *events.Event) {
		switch e.Type {
		case events.ETNewBlockReceived:
			block := e.Data.(*types.DposBlock)
			go a.OnBlockReceived(block.Block, block.HaveConfirm)

		case events.ETConfirmAccepted:
			go a.OnConfirmReceived(e.Data.(*mempool.ConfirmInfo))

		case events.ETDirectPeersChanged:
			a.OnPeersChanged(e.Data.([]peer.PID))

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
