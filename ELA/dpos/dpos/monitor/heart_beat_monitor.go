package monitor

import (
	"time"

	"github.com/elastos/Elastos.ELA.Utility/p2p/peer"
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/config"
	. "github.com/elastos/Elastos.ELA/dpos/dpos/arbitrator"
	"github.com/elastos/Elastos.ELA/dpos/log"
)

type HeartBeatListener interface {
	OnSendPing(peer *peer.Peer)
	OnAbnormalStateDetected()
}

type HeartBeatMonitor struct {
	listener HeartBeatListener

	beatDurations  map[string]time.Duration
	lastActiveTime map[string]time.Time
}

func (m *HeartBeatMonitor) Initialize(listener HeartBeatListener) {
	m.beatDurations = make(map[string]time.Duration)
	m.listener = listener

	m.ChangeArbitrators(config.Parameters.Arbiters)
}

func (m *HeartBeatMonitor) ChangeArbitrators(arbitrators []string) {
	m.beatDurations = make(map[string]time.Duration)
	m.lastActiveTime = make(map[string]time.Time)
	for _, v := range arbitrators {
		if v != config.Parameters.ArbiterConfiguration.Name {
			m.beatDurations[v] = 0
			m.lastActiveTime[v] = time.Now()
		}
	}
}

func (m *HeartBeatMonitor) Reset(arbitrator string) {
	if _, ok := m.beatDurations[arbitrator]; ok {
		m.beatDurations[arbitrator] = 0
	}
}

func (m *HeartBeatMonitor) SendPing(arbitrator string) {
	if peer, ok := GetPeerByName(arbitrator); ok {
		m.listener.OnSendPing(peer)
	} else {
		log.Warn("Send ping error because can not find peer, arbitrator public key :", arbitrator)
	}
}

func (m *HeartBeatMonitor) ResetActive(arbitrator string, t time.Time) {
	if _, ok := m.lastActiveTime[arbitrator]; ok {
		m.lastActiveTime[arbitrator] = t
	}
}

func (m *HeartBeatMonitor) checkAbnormalState() {
	inactiveCount := uint32(0)
	now := time.Now()
	for _, v := range m.lastActiveTime {
		if now.After(v.Add(3 * time.Second)) { //todo config inactive time
			inactiveCount++
		}
	}

	if blockchain.HasArbitersMajorityCount(inactiveCount) { //todo config inactive count
		m.listener.OnAbnormalStateDetected()
		for k := range m.lastActiveTime {
			m.lastActiveTime[k] = now
		}
	}
}

func (m *HeartBeatMonitor) HeartBeatTick() {
	startTime := time.Now()

	for {
		now := time.Now()
		duration := now.Sub(startTime)
		startTime = now

		ArbitratorSingleton.DposManager.Lock()
		for k, v := range m.beatDurations {
			//todo add config for heart beat interval
			if v > 500*time.Millisecond {
				m.Reset(k)
				m.SendPing(k)
			} else {
				m.beatDurations[k] += duration
			}
		}
		m.checkAbnormalState()
		ArbitratorSingleton.DposManager.Unlock()

		//todo add config for tick interval
		time.Sleep(10 * time.Millisecond)
	}
}
