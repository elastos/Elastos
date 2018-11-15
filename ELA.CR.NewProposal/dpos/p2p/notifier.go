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
	flags    NotifyFlag
	notify   func(flag NotifyFlag)
	addrList chan map[common.Uint256]PeerAddr
	newPeer  chan common.Uint256
	donePeer chan common.Uint256
}

func (n *Notifier) OnConnectPeers(addrList map[common.Uint256]PeerAddr) {
	n.addrList <- addrList
}

func (n *Notifier) OnNewPeer(pid common.Uint256) {
	n.newPeer <- pid
}

func (n *Notifier) OnDonePeer(pid common.Uint256) {
	n.donePeer <- pid
}

func (n *Notifier) notifyHandler() {
	// The current connect peer addresses.
	var addrList map[common.Uint256]PeerAddr

	// Connected peers whether inbound or outbound or both.
	var connected = make(map[common.Uint256]int)

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
		case list := <-n.addrList:
			// New ConnectPeers message received.
			if !equalList(addrList, list) {
				addrList = list
				startTimer()
			}

		case pid := <-n.newPeer:
			// There will be two peers(inbound and outbound) with the same pid
			// connected.
			connected[pid]++

			// Connected peers reach 2/3, mark server state as stable.
			if !stable && len(connected)/2 >= len(addrList)/3 {
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
			if stable && len(connected)/2 < len(addrList)/3 {
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
		addrList: make(chan map[common.Uint256]PeerAddr),
		newPeer:  make(chan common.Uint256),
		donePeer: make(chan common.Uint256),
	}
	go n.notifyHandler()
	return &n
}

func equalList(list1 map[common.Uint256]PeerAddr, list2 map[common.Uint256]PeerAddr) bool {
	if len(list1) != len(list2) || list1 == nil || list2 == nil {
		return false
	}
	for k, v := range list1 {
		addr, ok := list2[k]
		if !ok || addr.Addr != v.Addr {
			return false
		}
	}
	return true
}
