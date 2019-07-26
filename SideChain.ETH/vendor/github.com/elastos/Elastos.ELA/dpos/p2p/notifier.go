package p2p

import (
	"bytes"
	"sort"
	"strconv"
	"strings"
	"time"

	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"
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
	flags    NotifyFlag
	notify   func(flag NotifyFlag)
	peers    chan []peer.PID
	newPeer  chan peer.PID
	donePeer chan peer.PID
}

func (n *Notifier) OnConnectPeers(peers []peer.PID) {
	n.peers <- peers
}

func (n *Notifier) OnNewPeer(pid peer.PID) {
	n.newPeer <- pid
}

func (n *Notifier) OnDonePeer(pid peer.PID) {
	n.donePeer <- pid
}

func (n *Notifier) notifyHandler() {
	// The current connect peer addresses.
	var peers []peer.PID

	// Connected peers whether inbound or outbound or both.
	var connected = make(map[peer.PID]int)

	// The timeout timer used to trigger peers connection timeout.
	var timer *time.Timer
	var timeout = make(chan struct{})

	// stable mark if server have connected to enough peers.
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
		case list := <-n.peers:
			// New ConnectPeers message received.
			if !equalList(peers, list) {
				peers = list
				startTimer()
			}

		case pid := <-n.newPeer:
			// There will be two peers(inbound and outbound) with the same pid
			// connected.
			connected[pid]++

			// Connected peers reach 2/3, mark server state as stable.
			if !stable && len(connected)/2 >= len(peers)/3 {
				timer.Stop()
				stable = true
				if n.flags&NFNetStabled == NFNetStabled {
					go n.notify(NFNetStabled)
				}
			}

		case pid := <-n.donePeer:
			// When both inbound and outbound with the same pid disconnected,
			// remove the pid key.
			connected[pid]--
			if connected[pid] == 0 {
				delete(connected, pid)
			}

			// Stabled server turn to unstable.
			if stable && len(connected)/2 < len(peers)/3 {
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
		flags:    flags,
		notify:   notify,
		peers:    make(chan []peer.PID),
		newPeer:  make(chan peer.PID),
		donePeer: make(chan peer.PID),
	}
	go n.notifyHandler()
	return &n
}

func equalList(list1, list2 []peer.PID) bool {
	if len(list1) != len(list2) || list1 == nil || list2 == nil {
		return false
	}

	list1, list2 = sortPeers(list1), sortPeers(list2)
	for i := range list1 {
		if list1[i].Equal(list2[i]) {
			continue
		}
		return false
	}
	return true
}

func sortPeers(peers []peer.PID) []peer.PID {
	sort.Slice(peers, func(i, j int) bool {
		return bytes.Compare(peers[i][:], peers[j][:]) < 0
	})
	return peers
}
