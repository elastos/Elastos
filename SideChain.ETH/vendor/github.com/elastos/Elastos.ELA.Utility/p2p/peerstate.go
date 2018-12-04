package p2p

import "sync"

// Peer states
const (
	INIT = iota
	HAND
	HANDSHAKE
	HANDSHAKED
	ESTABLISH
	INACTIVITY
)

type PeerState struct {
	sync.Mutex
	state uint
}

func (p *PeerState) SetState(state uint) {
	p.Lock()
	defer p.Unlock()

	p.state = state
}

func (p *PeerState) State() uint {
	p.Lock()
	defer p.Unlock()

	return p.state
}

func (p *PeerState) String() string {
	switch p.state {
	case INIT:
		return "INIT"
	case HAND:
		return "HAND"
	case HANDSHAKE:
		return "HANDSHAKE"
	case HANDSHAKED:
		return "HANDSHAKED"
	case ESTABLISH:
		return "ESTABLISH"
	case INACTIVITY:
		return "INACTIVITY"
	}
	return "Unknown peer state"
}
