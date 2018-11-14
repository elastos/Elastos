package p2p

import (
	"strconv"
	"strings"
	"time"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

// NotifyFlag identifies notifies should be callback.
type NotifyFlag int64

const (
	// NFNetStabled is a flag to indicate network stabled.
	NFNetStabled NotifyFlag = 1 << iota

	// NFBadNetwork is a flag to indicate network unstable.
	NFBadNetwork
)

var nfStrings = map[NotifyFlag]string{
	NFBadNetwork: "NFBadNetwork",
}

var orderedNfStrings = []NotifyFlag{
	NFBadNetwork,
}

// String returns the NotifyFlag in human-readable form.
func (f NotifyFlag) String() string {
	// No flags are set.
	if f == 0 {
		return "0x0"
	}

	// Add individual bit flags.
	s := ""
	for _, flag := range orderedNfStrings {
		if f&flag == flag {
			s += nfStrings[flag] + "|"
			f -= flag
		}
	}

	// Add any remaining flags which aren't accounted for as hex.
	s = strings.TrimRight(s, "|")
	if f != 0 {
		s += "|0x" + strconv.FormatUint(uint64(f), 16)
	}
	s = strings.TrimLeft(s, "|")
	return s
}

// Ensure Notifier implement StateNotifier interface.
var _ StateNotifier = (*Notifier)(nil)

const (
	// ConnectionTimeout is the duration we timeout peer connections.
	ConnectionTimeout = time.Second * 30
)

type Notifier struct {
	flags     NotifyFlag
	notify    func(flag NotifyFlag)
	stateChan chan *peerState
}

func (n *Notifier) OnStateChange(state *peerState) {
	n.stateChan <- state
}

func (n *Notifier) notifyHandler() {
	var connectPeers map[common.Uint256]PeerAddr
	var timeout = make(chan struct{})
	var timer *time.Timer
	var stable bool

	startTimer := func() {
		stable = false

		if timer == nil {
			timer = time.NewTimer(ConnectionTimeout)
		} else {
			timer.Reset(ConnectionTimeout)
		}

		go func() {
			select {
			case <-timer.C:
				timeout <- struct{}{}
			}
		}()
	}

	for {
		select {
		case ps := <-n.stateChan:
			// ConnectList changed.
			if connectPeers == nil || !equalList(connectPeers, ps.connectPeers) {
				connectPeers = ps.connectPeers
				startTimer()
				continue
			}

			// Connected peers reach 2/3, mark server state as stable.
			if !stable && len(ps.outboundPeers)/2 >= len(connectPeers)/3 {
				timer.Stop()
				stable = true
				if n.flags&NFNetStabled == NFNetStabled {
					go n.notify(NFNetStabled)
				}
			}

			// Stabled server turn to unstable.
			if stable && len(ps.outboundPeers)/2 < len(connectPeers)/3 {
				startTimer()
				if n.flags&NFBadNetwork == NFBadNetwork {
					go n.notify(NFBadNetwork)
				}
			}

		case <-timeout:

			startTimer()
			if n.flags&NFBadNetwork == NFBadNetwork {
				go n.notify(NFBadNetwork)
			}

		}
	}
}

func NewNotifier(flags NotifyFlag, notify func(flag NotifyFlag)) *Notifier {
	n := Notifier{
		flags:     flags,
		notify:    notify,
		stateChan: make(chan *peerState),
	}
	go n.notifyHandler()
	return &n
}

func equalList(m1 map[common.Uint256]PeerAddr, m2 map[common.Uint256]PeerAddr) bool {
	if len(m1) != len(m2) {
		return false
	}
	for k, v := range m1 {
		addr, ok := m2[k]
		if !ok || addr.Addr != v.Addr {
			return false
		}
	}
	return true
}
