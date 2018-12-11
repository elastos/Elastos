package server

import (
	"errors"
	"fmt"
	"math/rand"
	"net"
	"os"
	"runtime"
	"strconv"
	"strings"
	"sync"
	"sync/atomic"
	"time"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/addrmgr"
	"github.com/elastos/Elastos.ELA.Utility/p2p/connmgr"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA.Utility/p2p/peer"
)

const (
	// defaultTargetOutbound is the default number of outbound peers to target.
	defaultTargetOutbound = 8

	// connectionRetryInterval is the base amount of time to wait in between
	// retries when connecting to persistent peers.  It is adjusted by the
	// number of retries such that there is a retry backoff.
	connectionRetryInterval = time.Second * 5
)

// simpleAddr implements the net.Addr interface with two struct fields
type simpleAddr struct {
	net, addr string
}

// String returns the address.
//
// This is part of the net.Addr interface.
func (a simpleAddr) String() string {
	return a.addr
}

// Network returns the network.
//
// This is part of the net.Addr interface.
func (a simpleAddr) Network() string {
	return a.net
}

// Ensure simpleAddr implements the net.Addr interface.
var _ net.Addr = simpleAddr{}

// broadcastMsg provides the ability to house a message to be broadcast
// to all connected peers except specified excluded peers.
type broadcastMsg struct {
	message      p2p.Message
	excludePeers []*serverPeer
}

// peerState maintains state of inbound, persistent, outbound peers as well
// as banned peers and outbound groups.
type peerState struct {
	inboundPeers    map[uint64]*serverPeer
	outboundPeers   map[uint64]*serverPeer
	persistentPeers map[uint64]*serverPeer
	banned          map[string]time.Time
	outboundGroups  map[string]int
}

// Count returns the count of all known peers.
func (ps *peerState) Count() int {
	return len(ps.inboundPeers) + len(ps.outboundPeers) +
		len(ps.persistentPeers)
}

// forAllOutboundPeers is a helper function that runs closure on all outbound
// peers known to peerState.
func (ps *peerState) forAllOutboundPeers(closure func(sp *serverPeer)) {
	for _, e := range ps.outboundPeers {
		closure(e)
	}
	for _, e := range ps.persistentPeers {
		closure(e)
	}
}

// forAllPeers is a helper function that runs closure on all peers known to
// peerState.
func (ps *peerState) forAllPeers(closure func(sp *serverPeer)) {
	for _, e := range ps.inboundPeers {
		closure(e)
	}
	ps.forAllOutboundPeers(closure)
}

// server provides a server for handling communications to and from peers.
type server struct {
	// The following variables must only be used atomically.
	// Putting the uint64s first makes them 64-bit aligned for 32-bit systems.
	started       int32
	shutdown      int32
	shutdownSched int32
	startupTime   int64

	cfg         Config
	sentNonces  *mruNonceMap
	addrManager *addrmgr.AddrManager
	connManager *connmgr.ConnManager
	newPeers    chan *serverPeer
	donePeers   chan *serverPeer
	banPeers    chan *serverPeer
	query       chan interface{}
	broadcast   chan broadcastMsg
	wg          sync.WaitGroup
	quit        chan struct{}
	nat         NAT
}

// IPeer extends the peer to maintain state shared by the server.
type serverPeer struct {
	*peer.Peer

	connReq        *connmgr.ConnReq
	server         *server
	persistent     bool
	sentAddrs      bool
	isWhitelisted  bool
	knownAddresses map[string]struct{}
	banScore       connmgr.DynamicBanScore
	quit           chan struct{}
}

// newServerPeer returns a new IPeer instance. The peer needs to be set by
// the caller.
func newServerPeer(s *server, isPersistent bool) *serverPeer {
	return &serverPeer{
		server:         s,
		persistent:     isPersistent,
		knownAddresses: make(map[string]struct{}),
		quit:           make(chan struct{}),
	}
}

// addKnownAddresses adds the given addresses to the set of known addresses to
// the peer to prevent sending duplicate addresses.
func (sp *serverPeer) addKnownAddresses(addresses []*p2p.NetAddress) {
	for _, na := range addresses {
		sp.knownAddresses[addrmgr.NetAddressKey(na)] = struct{}{}
	}
}

// addressKnown true if the given address is already known to the peer.
func (sp *serverPeer) addressKnown(na *p2p.NetAddress) bool {
	_, exists := sp.knownAddresses[addrmgr.NetAddressKey(na)]
	return exists
}

// pushAddrMsg sends an addr message to the connected peer using the provided
// addresses.
func (sp *serverPeer) pushAddrMsg(addresses []*p2p.NetAddress) {
	// Filter addresses already known to the peer.
	addrs := make([]*p2p.NetAddress, 0, len(addresses))
	for _, addr := range addresses {
		if !sp.addressKnown(addr) {
			addrs = append(addrs, addr)
		}
	}
	known, err := sp.PushAddrMsg(addrs)
	if err != nil {
		log.Errorf("Can't push address message to %s: %v", sp.Peer, err)
		sp.Disconnect()
		return
	}
	sp.addKnownAddresses(known)
}

// OnVersion is invoked when a peer receives a version message and is
// used to negotiate the protocol version details as well as kick start
// the communications.
func (sp *serverPeer) OnVersion(_ *peer.Peer, v *msg.Version) {
	// Signal the new peer.
	if sp.server.cfg.OnNewPeer != nil {
		sp.server.cfg.OnNewPeer(sp)
	}

	// Update the address manager and request known addresses from the
	// remote peer for outbound connections.  This is skipped when running
	// on the simulation test network since it is only intended to connect
	// to specified peers and actively avoids advertising and connecting to
	// discovered peers.
	addrManager := sp.server.addrManager

	// Outbound connections.
	if !sp.Inbound() {
		// Get address that best matches.
		lna := addrManager.GetBestLocalAddress(sp.NA())
		if addrmgr.IsRoutable(lna) {
			// Filter addresses the peer already knows about.
			addresses := []*p2p.NetAddress{lna}
			sp.pushAddrMsg(addresses)
		}

		// Request known addresses if the server address manager needs more.
		if addrManager.NeedMoreAddresses() {
			sp.SendMessage(new(msg.GetAddr), nil)
		}

		// Mark the address as a known good address.
		addrManager.Good(sp.NA())
	}

	// Add valid peer to the server.
	sp.server.AddPeer(sp)
}

// OnGetAddr is invoked when a peer receives a getaddr message and is used
// to provide the peer with known addresses from the address manager.
func (sp *serverPeer) OnGetAddr(_ *peer.Peer, msg *msg.GetAddr) {
	// Do not accept getaddr requests from outbound peers.  This reduces
	// fingerprinting attacks.
	if !sp.Inbound() {
		log.Debugf("Ignoring getaddr request from outbound peer ", "%v", sp)
		return
	}

	// Only allow one getaddr request per connection to discourage
	// address stamping of inv announcements.
	if sp.sentAddrs {
		log.Debugf("Ignoring repeated getaddr request from peer ", "%v", sp)
		return
	}
	sp.sentAddrs = true

	// Get the current known addresses from the address manager.
	addrCache := sp.server.addrManager.AddressCache()

	// Push the addresses.
	sp.pushAddrMsg(addrCache)
}

// OnAddr is invoked when a peer receives an addr message and is
// used to notify the server about advertised addresses.
func (sp *serverPeer) OnAddr(_ *peer.Peer, msg *msg.Addr) {
	// A message that has no addresses is invalid.
	if len(msg.AddrList) == 0 {
		log.Errorf("Command [%s] from %s does not contain any addresses", msg.CMD(), sp.Peer)
		sp.Disconnect()
		return
	}

	for _, na := range msg.AddrList {
		// Don't add more address if we're disconnecting.
		if !sp.Connected() {
			return
		}

		// Set the timestamp to 5 days ago if it's more than 24 hours
		// in the future so this address is one of the first to be
		// removed when space is needed.
		now := time.Now()
		if na.Timestamp.After(now.Add(time.Minute * 10)) {
			na.Timestamp = now.Add(-1 * time.Hour * 24 * 5)
		}

		// Add address to known addresses for this peer.
		sp.addKnownAddresses([]*p2p.NetAddress{na})
	}

	// Add addresses to server address manager.  The address manager handles
	// the details of things such as preventing duplicate addresses, max
	// addresses, and last seen updates.
	// XXX gives a 2 hour time penalty here, do we want to do the
	// same?
	sp.server.addrManager.AddAddresses(msg.AddrList, sp.NA())
}

// ToPeer returns the underlying peer instance.
//
// This function is safe for concurrent access and is part of the IPeer
// interface implementation.
func (sp *serverPeer) ToPeer() *peer.Peer {
	if sp == nil {
		return nil
	}
	return sp.Peer
}

// addBanScore increases the persistent and decaying ban score fields by the
// values passed as parameters. If the resulting score exceeds half of the ban
// threshold, a warning is logged including the reason provided. Further, if
// the score is above the ban threshold, the peer will be banned and
// disconnected.
func (sp *serverPeer) AddBanScore(persistent, transient uint32, reason string) {
	cfg := sp.server.cfg
	// No warning is logged and no score is calculated if banning is disabled.
	if cfg.DisableBanning {
		return
	}
	if sp.isWhitelisted {
		log.Debugf("Misbehaving whitelisted peer %s: %s", sp, reason)
		return
	}

	warnThreshold := cfg.BanThreshold >> 1
	if transient == 0 && persistent == 0 {
		// The score is not being increased, but a warning message is still
		// logged if the score is above the warn threshold.
		score := sp.banScore.Int()
		if score > warnThreshold {
			log.Warnf("Misbehaving peer %s: %s -- ban score is %d, "+
				"it was not increased this time", sp, reason, score)
		}
		return
	}
	score := sp.banScore.Increase(persistent, transient)
	if score > warnThreshold {
		log.Warnf("Misbehaving peer %s: %s -- ban score increased to %d", sp, reason, score)
		if score > cfg.BanThreshold {
			log.Warnf("Misbehaving peer %s -- banning and disconnecting", sp)
			sp.server.BanPeer(sp)
			sp.Disconnect()
		}
	}
}

// BanScore returns the current integer value that represents how close the peer
// is to being banned.
//
// This function is safe for concurrent access and is part of the IPeer
// interface implementation.
func (sp *serverPeer) BanScore() uint32 {
	return sp.banScore.Int()
}

// handleAddPeerMsg deals with adding new peers.  It is invoked from the
// peerHandler goroutine.
func (s *server) handleAddPeerMsg(state *peerState, sp *serverPeer) bool {
	if sp == nil {
		return false
	}

	// Ignore new peers if we're shutting down.
	if atomic.LoadInt32(&s.shutdown) != 0 {
		log.Infof("New peer %s ignored - server is shutting down", sp)
		sp.Disconnect()
		return false
	}

	// Disconnect banned peers.
	host, _, err := net.SplitHostPort(sp.Addr())
	if err != nil {
		log.Debugf("can't split hostport %v", err)
		sp.Disconnect()
		return false
	}
	if banEnd, ok := state.banned[host]; ok {
		if time.Now().Before(banEnd) {
			log.Debugf("Peer %s is banned for another %v - disconnecting",
				host, time.Until(banEnd))
			sp.Disconnect()
			return false
		}

		log.Infof("Peer %s is no longer banned", host)
		delete(state.banned, host)
	}

	// TODO: Check for max peers from a single IP.

	// Limit max number of total peers.
	if state.Count() >= s.cfg.MaxPeers {
		log.Infof("Max peers reached [%d] - disconnecting peer %s", s.cfg.MaxPeers, sp)
		sp.Disconnect()
		// TODO: how to handle permanent peers here?
		// they should be rescheduled.
		return false
	}

	// Add the new peer and start it.
	log.Debugf("New peer %s", sp)
	if sp.Inbound() {
		state.inboundPeers[sp.ID()] = sp
	} else {
		state.outboundGroups[addrmgr.GroupKey(sp.NA())]++
		if sp.persistent {
			state.persistentPeers[sp.ID()] = sp
		} else {
			state.outboundPeers[sp.ID()] = sp
		}
	}

	return true
}

// handleDonePeerMsg deals with peers that have signalled they are done.  It is
// invoked from the peerHandler goroutine.
func (s *server) handleDonePeerMsg(state *peerState, sp *serverPeer) {
	var list map[uint64]*serverPeer
	if sp.persistent {
		list = state.persistentPeers
	} else if sp.Inbound() {
		list = state.inboundPeers
	} else {
		list = state.outboundPeers
	}
	if _, ok := list[sp.ID()]; ok {
		if !sp.Inbound() && sp.VersionKnown() {
			state.outboundGroups[addrmgr.GroupKey(sp.NA())]--
		}
		if !sp.Inbound() && sp.connReq != nil {
			s.connManager.Disconnect(sp.connReq.ID())
		}
		delete(list, sp.ID())
		log.Debugf("Removed peer %s", sp)
		return
	}

	if sp.connReq != nil {
		s.connManager.Disconnect(sp.connReq.ID())
	}

	// Update the address' last seen time if the peer has acknowledged
	// our version and has sent us its version as well.
	if sp.VerAckReceived() && sp.VersionKnown() && sp.NA() != nil {
		s.addrManager.Connected(sp.NA())
	}

	// If we get here it means that either we didn't know about the peer
	// or we purposefully deleted it.
}

// handleBanPeerMsg deals with banning peers.  It is invoked from the
// peerHandler goroutine.
func (s *server) handleBanPeerMsg(state *peerState, sp *serverPeer) {
	host, _, err := net.SplitHostPort(sp.Addr())
	if err != nil {
		log.Debugf("can't split ban peer %s %v", sp.Addr(), err)
		return
	}
	direction := directionString(sp.Inbound())
	log.Infof("Banned peer %s (%s) for %v", host, direction, s.cfg.BanDuration)
	state.banned[host] = time.Now().Add(s.cfg.BanDuration)
}

// handleBroadcastMsg deals with broadcasting messages to peers.  It is invoked
// from the peerHandler goroutine.
func (s *server) handleBroadcastMsg(state *peerState, bmsg *broadcastMsg) {
	state.forAllPeers(func(sp *serverPeer) {
		if !sp.Connected() {
			return
		}

		for _, ep := range bmsg.excludePeers {
			if sp == ep {
				return
			}
		}

		sp.SendMessage(bmsg.message, nil)
	})
}

type getConnCountMsg struct {
	reply chan int32
}

type getPeersMsg struct {
	reply chan []*serverPeer
}

type getOutboundGroup struct {
	key   string
	reply chan int
}

type getAddedNodesMsg struct {
	reply chan []*serverPeer
}

type disconnectNodeMsg struct {
	cmp   func(*serverPeer) bool
	reply chan error
}

type connectNodeMsg struct {
	addr      string
	permanent bool
	reply     chan error
}

type removeNodeMsg struct {
	cmp   func(*serverPeer) bool
	reply chan error
}

// handleQuery is the central handler for all queries and commands from other
// goroutines related to peer state.
func (s *server) handleQuery(state *peerState, querymsg interface{}) {
	switch msg := querymsg.(type) {
	case getConnCountMsg:
		nconnected := int32(0)
		state.forAllPeers(func(sp *serverPeer) {
			if sp.Connected() {
				nconnected++
			}
		})
		msg.reply <- nconnected

	case getPeersMsg:
		peers := make([]*serverPeer, 0, state.Count())
		state.forAllPeers(func(sp *serverPeer) {
			if !sp.Connected() {
				return
			}
			peers = append(peers, sp)
		})
		msg.reply <- peers

	case connectNodeMsg:
		// TODO: duplicate oneshots?
		// Limit max number of total peers.
		if state.Count() >= s.cfg.MaxPeers {
			msg.reply <- errors.New("max peers reached")
			return
		}
		for _, peer := range state.persistentPeers {
			if peer.Addr() == msg.addr {
				if msg.permanent {
					msg.reply <- errors.New("peer already connected")
				} else {
					msg.reply <- errors.New("peer exists as a permanent peer")
				}
				return
			}
		}

		netAddr, err := addrStringToNetAddr(msg.addr)
		if err != nil {
			msg.reply <- err
			return
		}

		// TODO: if too many, nuke a non-perm peer.
		go s.connManager.Connect(&connmgr.ConnReq{
			Addr:      netAddr,
			Permanent: msg.permanent,
		})
		msg.reply <- nil
	case removeNodeMsg:
		found := disconnectPeer(state.persistentPeers, msg.cmp, func(sp *serverPeer) {
			// Keep group counts ok since we remove from
			// the list now.
			state.outboundGroups[addrmgr.GroupKey(sp.NA())]--
		})

		if found {
			msg.reply <- nil
		} else {
			msg.reply <- errors.New("peer not found")
		}
	case getOutboundGroup:
		count, ok := state.outboundGroups[msg.key]
		if ok {
			msg.reply <- count
		} else {
			msg.reply <- 0
		}
		// Request a list of the persistent (added) peers.
	case getAddedNodesMsg:
		// Respond with a slice of the relevant peers.
		peers := make([]*serverPeer, 0, len(state.persistentPeers))
		for _, sp := range state.persistentPeers {
			peers = append(peers, sp)
		}
		msg.reply <- peers
	case disconnectNodeMsg:
		// Check inbound peers. We pass a nil callback since we don't
		// require any additional actions on disconnect for inbound peers.
		found := disconnectPeer(state.inboundPeers, msg.cmp, nil)
		if found {
			msg.reply <- nil
			return
		}

		// Check outbound peers.
		found = disconnectPeer(state.outboundPeers, msg.cmp, func(sp *serverPeer) {
			// Keep group counts ok since we remove from
			// the list now.
			state.outboundGroups[addrmgr.GroupKey(sp.NA())]--
		})
		if found {
			// If there are multiple outbound connections to the same
			// ip:port, continue disconnecting them all until no such
			// peers are found.
			for found {
				found = disconnectPeer(state.outboundPeers, msg.cmp, func(sp *serverPeer) {
					state.outboundGroups[addrmgr.GroupKey(sp.NA())]--
				})
			}
			msg.reply <- nil
			return
		}

		msg.reply <- errors.New("peer not found")
	}
}

// disconnectPeer attempts to drop the connection of a targeted peer in the
// passed peer list. Targets are identified via usage of the passed
// `compareFunc`, which should return `true` if the passed peer is the target
// peer. This function returns true on success and false if the peer is unable
// to be located. If the peer is found, and the passed callback: `whenFound'
// isn't nil, we call it with the peer as the argument before it is removed
// from the peerList, and is disconnected from the server.
func disconnectPeer(peerList map[uint64]*serverPeer, compareFunc func(*serverPeer) bool, whenFound func(*serverPeer)) bool {
	for addr, peer := range peerList {
		if compareFunc(peer) {
			if whenFound != nil {
				whenFound(peer)
			}

			// This is ok because we are not continuing
			// to iterate so won't corrupt the loop.
			delete(peerList, addr)
			peer.Disconnect()
			return true
		}
	}
	return false
}

// newPeerConfig returns the configuration for the given serverPeer.
func newPeerConfig(sp *serverPeer) *peer.Config {
	cfg := &peer.Config{
		Magic:            sp.server.cfg.MagicNumber,
		ProtocolVersion:  sp.server.cfg.ProtocolVersion,
		Services:         sp.server.cfg.Services,
		DisableRelayTx:   sp.server.cfg.DisableRelayTx,
		HostToNetAddress: sp.server.addrManager.HostToNetAddress,
		MakeEmptyMessage: sp.server.cfg.MakeEmptyMessage,
		BestHeight:       sp.server.cfg.BestHeight,
		IsSelfConnection: func(nonce uint64) bool {
			return sp.server.sentNonces.Exists(nonce)
		},
		GetVersionNonce: func() uint64 {
			nonce := uint64(rand.Int63())
			sp.server.sentNonces.Add(nonce)
			return nonce
		},
	}

	// Add default message function for peer configuration.
	cfg.AddMessageFunc(func(peer *peer.Peer, m p2p.Message) {
		switch m := m.(type) {
		case *msg.Version:
			sp.OnVersion(peer, m)

		case *msg.GetAddr:
			sp.OnGetAddr(peer, m)

		case *msg.Addr:
			sp.OnAddr(peer, m)

		}
	})
	return cfg
}

// inboundPeerConnected is invoked by the connection manager when a new inbound
// connection is established.  It initializes a new inbound server peer
// instance, associates it with the connection, and starts a goroutine to wait
// for disconnection.
func (s *server) inboundPeerConnected(conn net.Conn) {
	sp := newServerPeer(s, false)
	sp.isWhitelisted = s.cfg.inWhitelist(conn.RemoteAddr())
	sp.Peer = peer.NewInboundPeer(newPeerConfig(sp))
	sp.AssociateConnection(conn)
	go s.peerDoneHandler(sp)
}

// outboundPeerConnected is invoked by the connection manager when a new
// outbound connection is established.  It initializes a new outbound server
// peer instance, associates it with the relevant state such as the connection
// request instance and the connection itself, and finally notifies the address
// manager of the attempt.
func (s *server) outboundPeerConnected(c *connmgr.ConnReq, conn net.Conn) {
	sp := newServerPeer(s, c.Permanent)
	p, err := peer.NewOutboundPeer(newPeerConfig(sp), c.Addr.String())
	if err != nil {
		log.Debugf("Cannot create outbound peer %s: %v", c.Addr, err)
		s.connManager.Disconnect(c.ID())
	}
	sp.Peer = p
	sp.connReq = c
	sp.isWhitelisted = s.cfg.inWhitelist(conn.RemoteAddr())
	sp.AssociateConnection(conn)
	go s.peerDoneHandler(sp)
	s.addrManager.Attempt(sp.NA())
}

// peerDoneHandler handles peer disconnects by notifiying the server that it's
// done along with other performing other desirable cleanup.
func (s *server) peerDoneHandler(sp *serverPeer) {
	sp.WaitForDisconnect()
	s.donePeers <- sp

	// Only tell sync manager we are gone if we ever told it we existed.
	if sp.VersionKnown() && s.cfg.OnDonePeer != nil {
		s.cfg.OnDonePeer(sp)
	}
	close(sp.quit)
}

// peerHandler is used to handle peer operations such as adding and removing
// peers to and from the server, banning peers, and broadcasting messages to
// peers.  It must be run in a goroutine.
func (s *server) peerHandler() {
	// Start the address manager, which are needed by peers.  This is done
	// here since their lifecycle is closely tied  to this handler and rather
	// than adding more channels, it's easier and slightly faster to simply
	// start and stop them in this handler.
	s.addrManager.Start()

	state := &peerState{
		inboundPeers:    make(map[uint64]*serverPeer),
		persistentPeers: make(map[uint64]*serverPeer),
		outboundPeers:   make(map[uint64]*serverPeer),
		banned:          make(map[string]time.Time),
		outboundGroups:  make(map[string]int),
	}

	go s.connManager.Start()

out:
	for {
		select {
		// New peers connected to the server.
		case p := <-s.newPeers:
			s.handleAddPeerMsg(state, p)

			// Disconnected peers.
		case p := <-s.donePeers:
			s.handleDonePeerMsg(state, p)

			// Peer to ban.
		case p := <-s.banPeers:
			s.handleBanPeerMsg(state, p)

			// Message to broadcast to all connected peers except those
			// which are excluded by the message.
		case bmsg := <-s.broadcast:
			s.handleBroadcastMsg(state, &bmsg)

		case qmsg := <-s.query:
			s.handleQuery(state, qmsg)

		case <-s.quit:
			// Disconnect all peers on server shutdown.
			state.forAllPeers(func(sp *serverPeer) {
				sp.Disconnect()
			})
			break out
		}
	}

	s.connManager.Stop()
	s.addrManager.Stop()

	// Drain channels before exiting so nothing is left waiting around
	// to send.
cleanup:
	for {
		select {
		case <-s.newPeers:
		case <-s.donePeers:
		case <-s.broadcast:
		case <-s.query:
		default:
			break cleanup
		}
	}
	s.wg.Done()
}

// AddPeer adds a new peer that has already been connected to the server.
func (s *server) AddPeer(sp *serverPeer) {
	s.newPeers <- sp
}

// BanPeer bans a peer that has already been connected to the server by ip.
func (s *server) BanPeer(sp *serverPeer) {
	s.banPeers <- sp
}

// BroadcastMessage sends msg to all peers currently connected to the server
// except those in the passed peers to exclude.
func (s *server) BroadcastMessage(msg p2p.Message, exclPeers ...*serverPeer) {
	// XXX: Need to determine if this is an alert that has already been
	// broadcast and refrain from broadcasting again.
	bmsg := broadcastMsg{message: msg, excludePeers: exclPeers}
	s.broadcast <- bmsg
}

// ConnectedCount returns the number of currently connected peers.
func (s *server) ConnectedCount() int32 {
	replyChan := make(chan int32)

	s.query <- getConnCountMsg{reply: replyChan}

	return <-replyChan
}

// OutboundGroupCount returns the number of peers connected to the given
// outbound group key.
func (s *server) OutboundGroupCount(key string) int {
	replyChan := make(chan int)
	s.query <- getOutboundGroup{key: key, reply: replyChan}
	return <-replyChan
}

// Start begins accepting connections from peers.
func (s *server) Start() {
	// Already started?
	if atomic.AddInt32(&s.started, 1) != 1 {
		return
	}

	// server startup time. Used for the uptime command for uptime calculation.
	s.startupTime = time.Now().Unix()

	// Start the peer handler which in turn starts the address and block
	// managers.
	s.wg.Add(1)
	go s.peerHandler()

	if s.nat != nil {
		s.wg.Add(1)
		go s.upnpUpdateThread()
	}
}

// Stop gracefully shuts down the server by stopping and disconnecting all
// peers and the main listener.
func (s *server) Stop() error {
	// Make sure this only happens once.
	if atomic.AddInt32(&s.shutdown, 1) != 1 {
		log.Infof("server is already in the process of shutting down")
		return nil
	}

	log.Warnf("server shutting down")

	// Signal the remaining goroutines to quit.
	close(s.quit)
	return nil
}

// WaitForShutdown blocks until the main listener and peer handlers are stopped.
func (s *server) WaitForShutdown() {
	s.wg.Wait()
}

// ScheduleShutdown schedules a server shutdown after the specified duration.
// It also dynamically adjusts how often to warn the server is going down based
// on remaining duration.
func (s *server) ScheduleShutdown(duration time.Duration) {
	// Don't schedule shutdown more than once.
	if atomic.AddInt32(&s.shutdownSched, 1) != 1 {
		return
	}
	log.Warnf("Server shutdown in %v", duration)
	go func() {
		remaining := duration
		tickDuration := dynamicTickDuration(remaining)
		done := time.After(remaining)
		ticker := time.NewTicker(tickDuration)
	out:
		for {
			select {
			case <-done:
				ticker.Stop()
				s.Stop()
				break out
			case <-ticker.C:
				remaining = remaining - tickDuration
				if remaining < time.Second {
					continue
				}

				// Change tick duration dynamically based on remaining time.
				newDuration := dynamicTickDuration(remaining)
				if tickDuration != newDuration {
					tickDuration = newDuration
					ticker.Stop()
					ticker = time.NewTicker(tickDuration)
				}
				log.Warnf("Server shutdown in %v", remaining)
			}
		}
	}()
}

// parseListeners determines whether each listen address is IPv4 and IPv6 and
// returns a slice of appropriate net.Addrs to listen on with TCP. It also
// properly detects addresses which apply to "all interfaces" and adds the
// address as both IPv4 and IPv6.
func parseListeners(addrs []string) ([]net.Addr, error) {
	netAddrs := make([]net.Addr, 0, len(addrs)*2)
	for _, addr := range addrs {
		host, _, err := net.SplitHostPort(addr)
		if err != nil {
			// Shouldn't happen due to already being normalized.
			return nil, err
		}

		// Empty host or host of * on plan9 is both IPv4 and IPv6.
		if host == "" || (host == "*" && runtime.GOOS == "plan9") {
			netAddrs = append(netAddrs, simpleAddr{net: "tcp4", addr: addr})
			netAddrs = append(netAddrs, simpleAddr{net: "tcp6", addr: addr})
			continue
		}

		// Strip IPv6 zone id if present since net.ParseIP does not
		// handle it.
		zoneIndex := strings.LastIndex(host, "%")
		if zoneIndex > 0 {
			host = host[:zoneIndex]
		}

		// Parse the IP.
		ip := net.ParseIP(host)
		if ip == nil {
			return nil, fmt.Errorf("'%s' is not a valid IP address", host)
		}

		// To4 returns nil when the IP is not an IPv4 address, so use
		// this determine the address type.
		if ip.To4() == nil {
			netAddrs = append(netAddrs, simpleAddr{net: "tcp6", addr: addr})
		} else {
			netAddrs = append(netAddrs, simpleAddr{net: "tcp4", addr: addr})
		}
	}
	return netAddrs, nil
}

func (s *server) upnpUpdateThread() {
	// Go off immediately to prevent code duplication, thereafter we renew
	// lease every 15 minutes.
	timer := time.NewTimer(0 * time.Second)
	lport := s.cfg.DefaultPort
	first := true
out:
	for {
		select {
		case <-timer.C:
			// XXX this assumes timeout is in seconds.
			listenPort, err := s.nat.AddPortMapping("tcp", int(lport), int(lport),
				"listen port", 20*60)
			if err != nil {
				log.Warnf("can't add UPnP port mapping: %v", err)
			}
			if first && err == nil {
				externalip, err := s.nat.GetExternalAddress()
				if err != nil {
					log.Warnf("UPnP can't get external address: %v", err)
					continue out
				}
				na := p2p.NewNetAddressIPPort(externalip, uint16(listenPort),
					s.cfg.Services)
				err = s.addrManager.AddLocalAddress(na, addrmgr.UpnpPrio)
				if err != nil {
					// XXX DeletePortMapping?
				}
				log.Warnf("Successfully bound via UPnP to %s", addrmgr.NetAddressKey(na))
				first = false
			}
			timer.Reset(time.Minute * 15)
		case <-s.quit:
			break out
		}
	}

	timer.Stop()

	if err := s.nat.DeletePortMapping("tcp", int(lport), int(lport)); err != nil {
		log.Warnf("unable to remove UPnP port mapping: %v", err)
	} else {
		log.Debugf("successfully disestablished UPnP port mapping")
	}

	s.wg.Done()
}

// Connect adds the provided address as a new outbound peer.  The permanent flag
// indicates whether or not to make the peer persistent and reconnect if the
// connection is lost.  Attempting to connect to an already existing peer will
// return an error.
//
// This function is safe for concurrent access and is part of the
// IServer interface implementation.
func (s *server) Connect(addr string, permanent bool) error {
	replyChan := make(chan error)
	s.query <- connectNodeMsg{
		addr:      addr,
		permanent: permanent,
		reply:     replyChan,
	}
	return <-replyChan
}

// RemoveByID removes the peer associated with the provided id from the list of
// persistent peers.  Attempting to remove an id that does not exist will return
// an error.
//
// This function is safe for concurrent access and is part of the
// IServer interface implementation.
func (s *server) RemoveByID(id uint64) error {
	replyChan := make(chan error)
	s.query <- removeNodeMsg{
		cmp:   func(sp *serverPeer) bool { return sp.ID() == id },
		reply: replyChan,
	}
	return <-replyChan
}

// RemoveByAddr removes the peer associated with the provided address from the
// list of persistent peers.  Attempting to remove an address that does not
// exist will return an error.
//
// This function is safe for concurrent access and is part of the
// IServer interface implementation.
func (s *server) RemoveByAddr(addr string) error {
	replyChan := make(chan error)
	s.query <- removeNodeMsg{
		cmp:   func(sp *serverPeer) bool { return sp.Addr() == addr },
		reply: replyChan,
	}
	return <-replyChan
}

// DisconnectByID disconnects the peer associated with the provided id.  This
// applies to both inbound and outbound peers.  Attempting to remove an id that
// does not exist will return an error.
//
// This function is safe for concurrent access and is part of the
// IServer interface implementation.
func (s *server) DisconnectByID(id uint64) error {
	replyChan := make(chan error)
	s.query <- disconnectNodeMsg{
		cmp:   func(sp *serverPeer) bool { return sp.ID() == id },
		reply: replyChan,
	}
	return <-replyChan
}

// DisconnectByAddr disconnects the peer associated with the provided address.
// This applies to both inbound and outbound peers.  Attempting to remove an
// address that does not exist will return an error.
//
// This function is safe for concurrent access and is part of the
// IServer interface implementation.
func (s *server) DisconnectByAddr(addr string) error {
	replyChan := make(chan error)
	s.query <- disconnectNodeMsg{
		cmp:   func(sp *serverPeer) bool { return sp.Addr() == addr },
		reply: replyChan,
	}
	return <-replyChan
}

// ConnectedPeers returns an array consisting of all connected peers.
//
// This function is safe for concurrent access and is part of the
// IServer interface implementation.
func (s *server) ConnectedPeers() []IPeer {
	replyChan := make(chan []*serverPeer)
	s.query <- getPeersMsg{reply: replyChan}
	serverPeers := <-replyChan

	// Convert to RPC server peers.
	peers := make([]IPeer, 0, len(serverPeers))
	for _, sp := range serverPeers {
		peers = append(peers, (IPeer)(sp))
	}
	return peers
}

// PersistentPeers returns an array consisting of all the added persistent
// peers.
//
// This function is safe for concurrent access and is part of the
// IServer interface implementation.
func (s *server) PersistentPeers() []IPeer {
	replyChan := make(chan []*serverPeer)
	s.query <- getAddedNodesMsg{reply: replyChan}
	serverPeers := <-replyChan

	// Convert to generic peers.
	peers := make([]IPeer, 0, len(serverPeers))
	for _, sp := range serverPeers {
		peers = append(peers, (IPeer)(sp))
	}
	return peers
}

// NewServer returns a new server instance by the given config.
// Use start to begin accepting connections from peers.
func newServer(origCfg *Config) (*server, error) {
	cfg := *origCfg // Copy to avoid mutating caller.
	cfg.normalize()

	// Create data dir for addrManager to store peer addresses.
	dataDir := defaultDataDir
	if len(cfg.DataDir) > 0 {
		dataDir = cfg.DataDir
	}
	_, err := os.Stat(dataDir)
	if os.IsNotExist(err) {
		os.MkdirAll(dataDir, os.ModePerm)
	}
	amgr := addrmgr.New(dataDir)

	var listeners []net.Listener
	var nat NAT
	if !cfg.DisableListen {
		var err error
		listeners, nat, err = initListeners(amgr, cfg)
		if err != nil {
			return nil, err
		}
		if len(listeners) == 0 {
			return nil, errors.New("no valid listen address")
		}
	}

	s := server{
		cfg:         cfg,
		sentNonces:  newMruNonceMap(50),
		addrManager: amgr,
		newPeers:    make(chan *serverPeer, cfg.MaxPeers),
		donePeers:   make(chan *serverPeer, cfg.MaxPeers),
		banPeers:    make(chan *serverPeer, cfg.MaxPeers),
		query:       make(chan interface{}),
		broadcast:   make(chan broadcastMsg, cfg.MaxPeers),
		quit:        make(chan struct{}),
		nat:         nat,
	}

	// Setup a function to return new addresses to connect to.
	var newAddressFunc = func() (net.Addr, error) {
		for tries := 0; tries < 100; tries++ {
			addr := s.addrManager.GetAddress()
			if addr == nil {
				break
			}

			log.Debugf("netAddressFunc pick addr %v", addr.NetAddress())
			// Address will not be invalid, local or unroutable
			// because addrmanager rejects those on addition.
			// Just check that we don't already have an address
			// in the same group so that we are not connecting
			// to the same network segment at the expense of
			// others.
			key := addrmgr.GroupKey(addr.NetAddress())
			if s.OutboundGroupCount(key) != 0 {
				continue
			}

			// only allow recent nodes (10mins) after we failed 30 times
			if tries < 30 && time.Since(addr.LastAttempt()) < 10*time.Minute {
				continue
			}

			// allow nondefault ports after 50 failed tries.
			if tries < 50 && addr.NetAddress().Port != cfg.DefaultPort {
				continue
			}

			addrString := addrmgr.NetAddressKey(addr.NetAddress())
			return addrStringToNetAddr(addrString)
		}

		return nil, errors.New("no valid connect address")
	}

	// Create a connection manager.
	targetOutbound := cfg.TargetOutbound
	if targetOutbound == 0 {
		targetOutbound = defaultTargetOutbound
	}
	if cfg.MaxPeers < targetOutbound {
		targetOutbound = cfg.MaxPeers
	}
	cmgr, err := connmgr.New(&connmgr.Config{
		Listeners:      listeners,
		OnAccept:       s.inboundPeerConnected,
		RetryDuration:  connectionRetryInterval,
		TargetOutbound: uint32(targetOutbound),
		Dial:           dialTimeout,
		OnConnection:   s.outboundPeerConnected,
		GetNewAddress:  newAddressFunc,
	})
	if err != nil {
		return nil, err
	}
	s.connManager = cmgr

	// Startup persistent peers.
	for _, addr := range cfg.SeedPeers {
		netAddr, err := addrStringToNetAddr(addr)
		if err != nil {
			return nil, err
		}

		go s.connManager.Connect(&connmgr.ConnReq{
			Addr:      netAddr,
			Permanent: true,
		})
	}

	return &s, nil
}

// initListeners initializes the configured net listeners and adds any bound
// addresses to the address manager. Returns the listeners and a NAT interface,
// which is non-nil if UPnP is in use.
func initListeners(amgr *addrmgr.AddrManager, cfg Config) ([]net.Listener, NAT, error) {
	// Listen for TCP connections at the configured addresses
	netAddrs, err := parseListeners(cfg.ListenAddrs)
	if err != nil {
		return nil, nil, err
	}

	listeners := make([]net.Listener, 0, len(netAddrs))
	for _, addr := range netAddrs {
		listener, err := net.Listen(addr.Network(), addr.String())
		if err != nil {
			log.Warnf("Can't listen on %s: %v", addr, err)
			continue
		}
		listeners = append(listeners, listener)
	}

	var nat NAT
	if len(cfg.ExternalIPs) != 0 {
		for _, sip := range cfg.ExternalIPs {
			eport := cfg.DefaultPort
			host, portstr, err := net.SplitHostPort(sip)
			if err != nil {
				// no port, use default.
				host = sip
			} else {
				port, err := strconv.ParseUint(portstr, 10, 16)
				if err != nil {
					log.Warnf("Can not parse port from %s for externalip: %v", sip, err)
					continue
				}
				eport = uint16(port)
			}
			na, err := amgr.HostToNetAddress(host, eport, cfg.Services)
			if err != nil {
				log.Warnf("Not adding %s as externalip: %v", sip, err)
				continue
			}

			err = amgr.AddLocalAddress(na, addrmgr.ManualPrio)
			if err != nil {
				log.Warnf("Skipping specified external IP: %v", err)
			}
		}
	} else {
		if cfg.Upnp {
			var err error
			nat, err = Discover()
			if err != nil {
				log.Warnf("Can't discover upnp: %v", err)
			}
			// nil nat here is fine, just means no upnp on network.
		}

		// Add bound addresses to address manager to be advertised to peers.
		for _, listener := range listeners {
			addr := listener.Addr().String()
			err := addLocalAddress(amgr, addr, cfg.Services)
			if err != nil {
				log.Warnf("Skipping bound address %s: %v", addr, err)
			}
		}
	}

	return listeners, nat, nil
}

// addrStringToNetAddr takes an address in the form of 'host:port' and returns
// a net.Addr which maps to the original address with any host names resolved
// to IP addresses.  It also handles tor addresses properly by returning a
// net.Addr that encapsulates the address.
func addrStringToNetAddr(addr string) (net.Addr, error) {
	host, strPort, err := net.SplitHostPort(addr)
	if err != nil {
		return nil, err
	}

	port, err := strconv.Atoi(strPort)
	if err != nil {
		return nil, err
	}

	// Skip if host is already an IP address.
	if ip := net.ParseIP(host); ip != nil {
		return &net.TCPAddr{
			IP:   ip,
			Port: port,
		}, nil
	}

	// Attempt to look up an IP address associated with the parsed host.
	ips, err := net.LookupIP(host)
	if err != nil {
		return nil, err
	}
	if len(ips) == 0 {
		return nil, fmt.Errorf("no addresses found for %s", host)
	}

	return &net.TCPAddr{
		IP:   ips[0],
		Port: port,
	}, nil
}

// addLocalAddress adds an address that this node is listening on to the
// address manager so that it may be relayed to peers.
func addLocalAddress(addrMgr *addrmgr.AddrManager, addr string, services uint64) error {
	host, portStr, err := net.SplitHostPort(addr)
	if err != nil {
		return err
	}
	port, err := strconv.ParseUint(portStr, 10, 16)
	if err != nil {
		return err
	}

	if ip := net.ParseIP(host); ip != nil && ip.IsUnspecified() {
		// If bound to unspecified address, advertise all local interfaces
		addrs, err := net.InterfaceAddrs()
		if err != nil {
			return err
		}

		for _, addr := range addrs {
			ifaceIP, _, err := net.ParseCIDR(addr.String())
			if err != nil {
				continue
			}

			// If bound to 0.0.0.0, do not add IPv6 interfaces and if bound to
			// ::, do not add IPv4 interfaces.
			if (ip.To4() == nil) != (ifaceIP.To4() == nil) {
				continue
			}

			netAddr := p2p.NewNetAddressIPPort(ifaceIP, uint16(port), services)
			addrMgr.AddLocalAddress(netAddr, addrmgr.BoundPrio)
		}
	} else {
		netAddr, err := addrMgr.HostToNetAddress(host, uint16(port), services)
		if err != nil {
			return err
		}

		addrMgr.AddLocalAddress(netAddr, addrmgr.BoundPrio)
	}

	return nil
}

// dynamicTickDuration is a convenience function used to dynamically choose a
// tick duration based on remaining time.  It is primarily used during
// server shutdown to make shutdown warnings more frequent as the shutdown time
// approaches.
func dynamicTickDuration(remaining time.Duration) time.Duration {
	switch {
	case remaining <= time.Second*5:
		return time.Second
	case remaining <= time.Second*15:
		return time.Second * 5
	case remaining <= time.Minute:
		return time.Second * 15
	case remaining <= time.Minute*5:
		return time.Minute
	case remaining <= time.Minute*15:
		return time.Minute * 5
	case remaining <= time.Hour:
		return time.Minute * 15
	}
	return time.Hour
}

func init() {
	rand.Seed(time.Now().UnixNano())
}
