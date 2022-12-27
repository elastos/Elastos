package server

import (
	"errors"
	"fmt"
	"math/rand"
	"net"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA/p2p"
	"github.com/elastos/Elastos.ELA/p2p/addrmgr"
	"github.com/elastos/Elastos.ELA/p2p/msg"
	"github.com/elastos/Elastos.ELA/p2p/peer"
)

const (
	// maxRetryCount defines the maximum retry time to a DNS host.
	maxRetryCount = 50

	// retryDuration defines the duration to retry seeding a DNS host.
	retryDuration = 10 * time.Second

	// retryDuration defines the maximum duration to retry seeding a DNS host.
	maxRetryDuration = 5 * time.Minute

	// seedingTimeout indicates the duration to timeout a seeding request.
	seedingTimeout = 10 * time.Second
)

// seed defines a seed provider to fetch addresses from DNS.
type seed struct {
	cfg                *Config
	addrManager        *addrmgr.AddrManager
	outboundGroupCount func(key string) int
	seeding            sync.Map
}

// GetAddress returns a address to connect to.
func (s *seed) GetAddress() (net.Addr, error) {
	for tries := 0; tries < 100; tries++ {
		addr := s.addrManager.GetAddress()
		if addr == nil {
			break
		}

		log.Debugf("seeds pick addr %v", addr.NetAddress())
		// Address will not be invalid, local or unroutable
		// because addrmanager rejects those on addition.
		// Just check that we don't already have an address
		// in the same group so that we are not connecting
		// to the same network segment at the expense of
		// others.
		key := addrmgr.GroupKey(addr.NetAddress())
		if s.outboundGroupCount(key) != 0 {
			continue
		}

		// only allow recent nodes (10mins) after we failed 30 times
		if tries < 30 && time.Since(addr.LastAttempt()) < 10*time.Minute {
			continue
		}

		// allow non-default ports after 50 failed tries.
		if tries < 50 && addr.NetAddress().Port != s.cfg.DefaultPort {
			continue
		}

		addrString := addrmgr.NetAddressKey(addr.NetAddress())
		return addrStringToNetAddr(addrString)
	}

	// Trigger DNS seeding if their are no valid address.
	s.fromDNS()

	return nil, errors.New("no valid connect address")
}

// newConfig create a configuration for the peer.
func (s *seed) newConfig(addrChan chan []*p2p.NetAddress) *peer.Config {
	return &peer.Config{
		Magic:           s.cfg.MagicNumber,
		ProtocolVersion: s.cfg.ProtocolVersion,
		DefaultPort:     s.cfg.DefaultPort,
		Services:        s.cfg.Services,
		DisableRelayTx:  true,
		MakeEmptyMessage: func(cmd string) (p2p.Message, error) {
			return nil, fmt.Errorf("unhandled message %s from DNS", cmd)
		},
		BestHeight:       func() uint64 { return 0 },
		IsSelfConnection: func(net.IP, int, uint64) bool { return false },
		GetVersionNonce:  func() uint64 { return uint64(rand.Int63()) },
		MessageFunc: func(peer *peer.Peer, m p2p.Message) {
			switch m := m.(type) {
			case *msg.Version:
				peer.QueueMessage(msg.NewGetAddr(), nil)

			case *msg.Addr:
				addrChan <- m.AddrList

			}
		},
	}
}

// discover discovers seed addresses from the host.
func (s *seed) discover(host string, addrChan chan []*p2p.NetAddress,
	retryCount int, retryTime time.Duration) {
	// Quit retry if max retry count arrived.
	if retryCount >= maxRetryCount {
		addrChan <- nil
		return
	}

	// Connect to the DNS host.
	conn, err := net.Dial("tcp", host)
	if err != nil {
		log.Debugf("Can not connect to host %s, %s", host, err)
		return
	}

	addrs := make(chan []*p2p.NetAddress)
	p, err := peer.NewOutboundPeer(s.newConfig(addrs), host)
	if err != nil {
		log.Debugf("Cannot create outbound peer %s: %v", host, err)
		conn.Close()
		return
	}
	p.AssociateConnection(conn)

	select {
	case seeds := <-addrs:
		// Notify the received addresses.
		addrChan <- seeds

	case <-time.After(seedingTimeout):
		// Limit retry duration.
		if retryTime > maxRetryDuration {
			retryTime = maxRetryDuration
		}
		log.Debugf("Retry seeding %s in %v, times [%d]", host,
			retryTime, retryCount)
		time.AfterFunc(retryTime, func() {
			s.discover(host, addrChan, retryCount+1,
				time.Duration(retryCount+1)*retryDuration)
		})
	}

	// Disconnect the peer.
	p.Disconnect()
}

// FromDNS uses DNS seeding to populate the address manager with peers.
func (s *seed) fromDNS() {
	for _, host := range s.cfg.DNSSeeds {
		// Do not seeding a DNS if a previous request is not finished.
		if _, ok := s.seeding.LoadOrStore(host, host); ok {
			continue
		}

		go func(host string) {
			// Get seeds from the host.
			addrChan := make(chan []*p2p.NetAddress, 1)
			s.discover(host, addrChan, 0, 0)
			addrs := <-addrChan
			log.Debugf("Get %d seeds from %s\n%v", len(addrs), host,
				addrs)

			// Remove host from seeding list.
			s.seeding.Delete(host)

			// No addresses received.
			if len(addrs) == 0 {
				return
			}

			// Populate the address manager with received addresses.
			s.addrManager.AddAddresses(addrs, addrs[0])

		}(host)
	}
}

// newSeed creates and return a seed instance.
func newSeed(cfg *Config, addrmgr *addrmgr.AddrManager,
	outboundGroupCount func(key string) int) *seed {
	return &seed{
		cfg:                cfg,
		addrManager:        addrmgr,
		outboundGroupCount: outboundGroupCount,
	}
}
