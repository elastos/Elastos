package p2p

import (
	"bytes"
	"errors"
	"fmt"
	"math/rand"
	"net"
	"runtime"
	"sort"
	"strconv"
	"sync"
	"sync/atomic"
	"time"

	"github.com/elastos/Elastos.ELA/dpos/p2p/addrmgr"
	"github.com/elastos/Elastos.ELA/dpos/p2p/connmgr"
	"github.com/elastos/Elastos.ELA/dpos/p2p/hub"
	"github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"

	"github.com/elastos/Elastos.ELA/p2p"
)

const (
	// Max number of inbound and outbound peers.
	maxPeers = 72 * 2

	// connectionRetryInterval is the base amount of time to wait in between
	// retries when connecting to persistent peers.  It is adjusted by the
	// number of retries such that there is a retry backoff.
	connectionRetryInterval = time.Second * 5
)

var (
	// ErrSendMessageFailed is the error to reply when send message to peer
	// failed.
	ErrSendMessageFailed = errors.New("send message failed")
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

// newPeerMsg represent a new connected peer.
type newPeerMsg struct {
	sp    *serverPeer
	reply chan struct{}
}

// donePeerMsg represent a disconnected peer.
type donePeerMsg struct {
	sp    *serverPeer
	reply chan struct{}
}

// broadcastMsg provides the ability to house a message to be broadcast
// to all connected peers except specified excluded peers.
type broadcastMsg struct {
	message      p2p.Message
	excludePeers []peer.PID
}

// peerState maintains state of inbound, persistent, outbound peers as well
// as banned peers and outbound groups.
type peerState struct {
	connectPeers  map[peer.PID]struct{}
	inboundPeers  map[uint64]*serverPeer
	outboundPeers map[uint64]*serverPeer
}

// havePeer returns how many connected peers matches to given PID, if no matches
// return 0
func (ps *peerState) havePeer(pid peer.PID) int {
	var matches int
	ps.forAllPeers(func(sp *serverPeer) {
		if sp.PID().Equal(pid) {
			matches++
		}
	})
	return matches
}

// Count returns the count of all known peers.
func (ps *peerState) Count() int {
	return len(ps.inboundPeers) + len(ps.outboundPeers)
}

// forAllOutboundPeers is a helper function that runs closure on all outbound
// peers known to peerState.
func (ps *peerState) forAllOutboundPeers(closure func(sp *serverPeer)) {
	for _, e := range ps.outboundPeers {
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
	hubService  *hub.Hub
	addrManager *addrmgr.AddrManager
	connManager *connmgr.ConnManager
	peerQueue   chan interface{}
	query       chan interface{}
	broadcast   chan broadcastMsg
	wg          sync.WaitGroup
	quit        chan struct{}
}

// IPeer extends the peer to maintain state shared by the server.
type serverPeer struct {
	*peer.Peer

	connReq        *connmgr.ConnReq
	server         *server
	sentAddrs      bool
	knownAddresses map[string]struct{}
	quit           chan struct{}
}

// newServerPeer returns a new IPeer instance. The peer needs to be set by
// the caller.
func newServerPeer(s *server) *serverPeer {
	return &serverPeer{
		server:         s,
		knownAddresses: make(map[string]struct{}),
		quit:           make(chan struct{}),
	}
}

// OnVersion is invoked when a peer receives a version message and is
// used to negotiate the protocol version details as well as kick start
// the communications.
func (sp *serverPeer) OnVersion(_ *peer.Peer, v *msg.Version) {
	// Check if received PID matches PeerAddr PID.
	if sp.connReq != nil && !bytes.Equal(v.PID[:], sp.connReq.PID[:]) {
		log.Infof("Disconnecting peer %v - version PID not match", sp)
		sp.Disconnect()
		return
	}

	// Advertise the local address when the server accepts outbound incoming
	// connections.
	if !sp.Inbound() {
		addr := msg.NewAddr(sp.server.cfg.Localhost, sp.server.cfg.DefaultPort)
		sp.QueueMessage(addr, nil)
	}

	// Add the remote peer time as a sample for creating an offset against
	// the local clock to keep the network time in sync.
	sp.server.cfg.TimeSource.AddTimeSample(sp.Addr(), v.Timestamp)

	// Add valid peer to the server.
	sp.server.AddPeer(sp)
}

// OnAddr is invoked when a peer receives an addr message and is used to notify
// the server about advertised address.
func (sp *serverPeer) OnAddr(_ *peer.Peer, msg *msg.Addr) {
	addr := normalizeAddress(msg.Host, fmt.Sprint(msg.Port))
	sp.server.addrManager.AddAddress(sp.PID(), simpleAddr{net: "tcp",
		addr: addr})
}

// PID returns the peer's public key id.
func (sp *serverPeer) PID() peer.PID {
	return sp.Peer.PID()
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

	// Ignore peers not exist in connect list.
	if _, ok := state.connectPeers[sp.PID()]; !ok {
		log.Infof("New peer %s ignored - not in connect list", sp)
		sp.Disconnect()
		return false
	}

	// Add the new peer and start it.
	log.Debugf("New peer %s", sp)
	if sp.Inbound() {
		state.inboundPeers[sp.ID()] = sp
	} else {
		state.outboundPeers[sp.ID()] = sp
	}

	// Add message handler for the new peer.
	if sp.server.cfg.HandleMessage != nil {
		sp.Peer.AddMessageFunc(func(peer *peer.Peer, msg p2p.Message) {
			sp.server.cfg.HandleMessage(peer.PID(), msg)
		})
	}

	// Notify peer state change for new peer added.
	if s.cfg.StateNotifier != nil {
		s.cfg.StateNotifier.OnNewPeer(sp.PID())
	}

	return true
}

// handleDonePeerMsg deals with peers that have signalled they are done.  It is
// invoked from the peerHandler goroutine.
func (s *server) handleDonePeerMsg(state *peerState, sp *serverPeer) {
	var list map[uint64]*serverPeer
	if sp.Inbound() {
		list = state.inboundPeers
	} else {
		list = state.outboundPeers
	}
	if _, ok := list[sp.ID()]; ok {
		delete(list, sp.ID())
		log.Debugf("Removed peer %s", sp)
	}

	// Remove connection request if peer not in connect list.
	if sp.connReq != nil {
		s.connManager.Disconnect(sp.connReq.PID)
	}

	// Notify peer state change for done peer removed.
	if s.cfg.StateNotifier != nil {
		s.cfg.StateNotifier.OnDonePeer(sp.PID())
	}
}

// handleBroadcastMsg deals with broadcasting messages to peers.  It is invoked
// from the peerHandler goroutine.
func (s *server) handleBroadcastMsg(state *peerState, bmsg *broadcastMsg) {
	// Group peers by PID, so each peer only get one broadcast message.
	groups := make(map[peer.PID]*serverPeer)
	state.forAllPeers(func(sp *serverPeer) {
		if !sp.Connected() {
			return
		}

		for _, ep := range bmsg.excludePeers {
			if sp.PID().Equal(ep) {
				return
			}
		}

		groups[sp.PID()] = sp
	})

	for _, sp := range groups {
		sp.QueueMessage(bmsg.message, nil)
	}
}

type connectPeersMsg struct {
	peers []peer.PID
	reply chan struct{}
}

type sendToPeerMsg struct {
	pid   peer.PID
	msg   p2p.Message
	reply chan error
}

type getConnCountMsg struct {
	reply chan int32
}

type getPeersMsg struct {
	reply chan []*serverPeer
}

type dumpPeersInfoMsg struct {
	reply chan []*PeerInfo
}

// handleQuery is the central handler for all queries and commands from other
// goroutines related to peer state.
func (s *server) handleQuery(state *peerState, querymsg interface{}) {
	switch msg := querymsg.(type) {
	case connectPeersMsg:
		// A connectPeers message will make server connect to the peers
		// in connect list, and disconnect peers not in connect list.

		// connectPeers saves the new received connect peer addresses.
		connectPeers := make(map[peer.PID]struct{})

		// Loop through the new received connect peer addresses.
		for _, pid := range msg.peers {
			// Do not create connection to self.
			if pid.Equal(s.cfg.PID) {
				continue
			}

			// Add address to connectPeers
			connectPeers[pid] = struct{}{}

			// Peers in previous connect list already.
			if _, ok := state.connectPeers[pid]; ok {
				continue
			}

			// Connect the peer.
			go s.connManager.Connect(pid)
		}

		// disconnectPeers saves the peers need to be disconnected.
		disconnectPeers := make(map[peer.PID]struct{})

		// Peers not in new received connect list need to be disconnected.
		for pid := range state.connectPeers {
			if _, ok := connectPeers[pid]; !ok {
				disconnectPeers[pid] = struct{}{}
			}
		}

		// Disconnect peers in disconnect list.
		for pid := range disconnectPeers {
			s.connManager.Remove(pid)
		}
		state.forAllPeers(func(sp *serverPeer) {
			if _, ok := disconnectPeers[sp.PID()]; ok {
				sp.Disconnect()
			}
		})

		// Set new connect list into state.
		state.connectPeers = connectPeers

		// Notify peer state change for connect list changed.
		if s.cfg.StateNotifier != nil {
			s.cfg.StateNotifier.OnConnectPeers(msg.peers)
		}

		msg.reply <- struct{}{}

	case sendToPeerMsg:
		// SendToPeer message send a message to the connected peer by PID.
		// There may be multiple connected peers matches the given PID,
		// we just pick the first matched peer to send the message, if
		// something goes wrong, we try next. If all attempts are failed
		// return an send message failed error.
		sent := false
		done := make(chan error, 1)
		state.forAllPeers(func(sp *serverPeer) {
			if !sent && sp.PID().Equal(msg.pid) {
				sp.QueueMessage(msg.msg, done)
				if err := <-done; err == nil {
					sent = true
				}
			}
		})

		if !sent {
			msg.reply <- ErrSendMessageFailed
		} else {
			msg.reply <- nil
		}

	case getConnCountMsg:
		connected := int32(0)
		state.forAllPeers(func(sp *serverPeer) {
			if sp.Connected() {
				connected++
			}
		})
		msg.reply <- connected

	case getPeersMsg:
		peers := make([]*serverPeer, 0, state.Count())
		state.forAllPeers(func(sp *serverPeer) {
			if !sp.Connected() {
				return
			}
			peers = append(peers, sp)
		})
		msg.reply <- peers

	case dumpPeersInfoMsg:
		// DumpPeesInfo returns the peers info in connect peers list.  The peers
		// in connect list can be in 4 states.
		// 1. NoneConnection, no outbound or inbound connection.
		// 2. 2WayConnection, have both outbound and inbound connections.
		// 3. OutboundOnly, has one outbound connection.
		// 4. InboundOnly, has one inbound connection.

		// To get the actual state of a peer in connect list, we need to loop
		// through outbound peers list, inbound peers list and connect peers
		// list, these are high cost operations.  So this method should not be
		// called frequently.
		peers := make(map[peer.PID]*PeerInfo)
		for _, sp := range state.outboundPeers {
			peers[sp.PID()] = &PeerInfo{
				PID:   sp.PID(),
				Addr:  sp.Addr(),
				State: CSOutboundOnly,
			}
		}
		for _, sp := range state.inboundPeers {
			if pi, ok := peers[sp.PID()]; ok {
				pi.State = CS2WayConnection
				continue
			}
			peers[sp.PID()] = &PeerInfo{
				PID:   sp.PID(),
				Addr:  sp.Addr(),
				State: CSInboundOnly,
			}
		}
		for pid := range state.connectPeers {
			if _, ok := peers[pid]; ok {
				continue
			}
			addr := "unknown"
			na := s.addrManager.GetAddress(pid)
			if na != nil {
				addr = na.String()
			}
			peers[pid] = &PeerInfo{
				PID:   pid,
				Addr:  addr,
				State: CSNoneConnection,
			}
		}
		msg.reply <- sortPeersInfo(peers)
	}
}

// sortPeersInfo returns an ordered PeerInfo slice by peer's PID in asc.
func sortPeersInfo(peers map[peer.PID]*PeerInfo) []*PeerInfo {
	list := make([]*PeerInfo, 0, len(peers))
	for _, pi := range peers {
		list = append(list, pi)
	}
	sort.Slice(list, func(i, j int) bool {
		return bytes.Compare(list[i].PID[:], list[j].PID[:]) < 0
	})
	return list
}

func (s *server) pingNonce(pid peer.PID) uint64 {
	if s.cfg.PingNonce == nil {
		return 0
	}
	return s.cfg.PingNonce(pid)
}

func (s *server) pongNonce(pid peer.PID) uint64 {
	if s.cfg.PongNonce == nil {
		return 0
	}
	return s.cfg.PongNonce(pid)
}

// newPeerConfig returns the configuration for the given serverPeer.
func newPeerConfig(sp *serverPeer) *peer.Config {
	return &peer.Config{
		PID:              sp.server.cfg.PID,
		Magic:            sp.server.cfg.MagicNumber,
		Port:             sp.server.cfg.DefaultPort,
		PingInterval:     sp.server.cfg.PingInterval,
		Sign:             sp.server.cfg.Sign,
		PingNonce:        sp.server.pingNonce,
		PongNonce:        sp.server.pongNonce,
		MakeEmptyMessage: sp.server.cfg.MakeEmptyMessage,
		MessageFunc: func(peer *peer.Peer, m p2p.Message) {
			switch m := m.(type) {
			case *msg.Version:
				sp.OnVersion(peer, m)

			case *msg.Addr:
				sp.OnAddr(peer, m)

			}
		},
	}
}

// inboundPeerConnected is invoked by the connection manager when a new inbound
// connection is established.  It initializes a new inbound server peer
// instance, associates it with the connection, and starts a goroutine to wait
// for disconnection.
func (s *server) inboundPeerConnected(conn net.Conn) {
	// If hub service is enabled, intercept the connection.
	if s.hubService != nil {
		conn = s.hubService.Intercept(conn)
	}

	// If the connection has been intercepted, do nothing.
	if conn == nil {
		return
	}

	sp := newServerPeer(s)
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
	sp := newServerPeer(s)
	cfg := newPeerConfig(sp)
	cfg.Target = hub.PIDTo16(c.PID)
	p, err := peer.NewOutboundPeer(cfg, c.Addr.String())
	if err != nil {
		log.Debugf("Cannot create outbound peer %s: %v", c.Addr, err)
		s.connManager.Disconnect(c.PID)
	}
	sp.Peer = p
	sp.connReq = c
	sp.AssociateConnection(conn)
	go s.peerDoneHandler(sp)
}

// peerDoneHandler handles peer disconnects by notifiying the server that it's
// done along with other performing other desirable cleanup.
func (s *server) peerDoneHandler(sp *serverPeer) {
	sp.WaitForDisconnect()
	reply := make(chan struct{})
	s.peerQueue <- donePeerMsg{sp: sp, reply: reply}
	<-reply
	close(sp.quit)
}

// peerHandler is used to handle peer operations such as adding and removing
// peers to and from the server, banning peers, and broadcasting messages to
// peers.  It must be run in a goroutine.
func (s *server) peerHandler() {
	s.addrManager.Start()

	state := &peerState{
		inboundPeers:  make(map[uint64]*serverPeer),
		outboundPeers: make(map[uint64]*serverPeer),
	}

	go s.connManager.Start()

out:
	for {
		select {
		// Deal with peer messages.
		case pmsg := <-s.peerQueue:
			s.handlePeerMsg(state, pmsg)

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
		case <-s.peerQueue:
		case <-s.broadcast:
		case <-s.query:
		default:
			break cleanup
		}
	}
	s.wg.Done()
}

// handlePeerMsg deals with adding/removing and ban peer message.
func (s *server) handlePeerMsg(state *peerState, msg interface{}) {
	switch msg := msg.(type) {
	case newPeerMsg:
		s.handleAddPeerMsg(state, msg.sp)
		msg.reply <- struct{}{}

	case donePeerMsg:
		s.handleDonePeerMsg(state, msg.sp)
		msg.reply <- struct{}{}

	}
}

// AddAddr adds an arbiter address into AddrManager.
func (s *server) AddAddr(pid peer.PID, addr string) {
	addr = normalizeAddress(addr, fmt.Sprint(s.cfg.DefaultPort))
	s.addrManager.AddAddress(pid, &simpleAddr{net: "tcp", addr: addr})
}

// AddPeer adds a new peer that has already been connected to the server.
func (s *server) AddPeer(sp *serverPeer) {
	reply := make(chan struct{})
	s.peerQueue <- newPeerMsg{sp: sp, reply: reply}
	<-reply
}

// BroadcastMessage sends msg to all peers currently connected to the server
// except those in the passed peers to exclude.
func (s *server) BroadcastMessage(msg p2p.Message, exclPeers ...peer.PID) {
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

// ConnectPeers let server connect the peers in the given peers, and
// disconnect peers that not in the peers.
func (s *server) ConnectPeers(peers []peer.PID) {
	reply := make(chan struct{})
	s.query <- connectPeersMsg{peers: peers, reply: reply}
	<-reply
}

// SendMessageToPeer send a message to the peer with the given id, error
// will be returned if there is no matches, or fail to send the message.
func (s *server) SendMessageToPeer(id peer.PID, msg p2p.Message) error {
	reply := make(chan error)
	s.query <- sendToPeerMsg{pid: id, msg: msg, reply: reply}
	return <-reply
}

// ConnectedPeers returns an array consisting of all connected peers.
//
// This function is safe for concurrent access and is part of the
// IServer interface implementation.
func (s *server) ConnectedPeers() []Peer {
	replyChan := make(chan []*serverPeer)
	s.query <- getPeersMsg{reply: replyChan}
	serverPeers := <-replyChan

	peers := make([]Peer, 0, len(serverPeers))
	for _, sp := range serverPeers {
		peers = append(peers, (Peer)(sp))
	}
	return peers
}

// DumpPeersInfo returns an array consisting of all peers state in connect list.
//
// This function is safe for concurrent access and is part of the
// IServer interface implementation.
func (s *server) DumpPeersInfo() []*PeerInfo {
	replyChan := make(chan []*PeerInfo)
	s.query <- dumpPeersInfoMsg{reply: replyChan}
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

func (s *server) dialTimeout(addr net.Addr) (net.Conn, error) {
	log.Debugf("Server dial addr %s", addr)
	addr, err := addrStringToNetAddr(addr.String())
	if err != nil {
		return nil, err
	}
	return net.DialTimeout(addr.Network(), addr.String(), s.cfg.ConnectTimeout)
}

// parseListeners determines whether each listen address is IPv4 and IPv6 and
// returns a slice of appropriate net.Addrs to listen on with TCP. It also
// properly detects addresses which apply to "all interfaces" and adds the
// address as both IPv4 and IPv6.
func parseListeners(addr string) ([]net.Addr, error) {
	netAddrs := make([]net.Addr, 0, 2)
	host, _, err := net.SplitHostPort(addr)
	if err != nil {
		// Shouldn't happen due to already being normalized.
		return nil, err
	}

	// Empty host or host of * on plan9 is both IPv4 and IPv6.
	if host == "" || (host == "*" && runtime.GOOS == "plan9") {
		netAddrs = append(netAddrs, simpleAddr{net: "tcp4", addr: addr})
		netAddrs = append(netAddrs, simpleAddr{net: "tcp6", addr: addr})
	}

	return netAddrs, nil
}

// NewServer returns a new server instance by the given config.
// Use start to begin accepting connections from peers.
func NewServer(origCfg *Config) (*server, error) {
	cfg := *origCfg // Copy to avoid mutating caller.
	if cfg.ConnectTimeout <= 0 {
		cfg.ConnectTimeout = defaultConnectTimeout
	}
	if cfg.PingInterval <= 0 {
		cfg.PingInterval = defaultPingInterval
	}

	listeners, err := initListeners(cfg)
	if err != nil {
		return nil, err
	}

	admgr := addrmgr.New(cfg.DataDir)
	var hubService *hub.Hub
	if cfg.EnableHub {
		hubService = hub.New(cfg.MagicNumber, cfg.PID, admgr)
	}

	s := server{
		cfg:         cfg,
		hubService:  hubService,
		addrManager: admgr,
		peerQueue:   make(chan interface{}, maxPeers),
		query:       make(chan interface{}, maxPeers),
		broadcast:   make(chan broadcastMsg, maxPeers),
		quit:        make(chan struct{}),
	}

	cmgr, err := connmgr.New(&connmgr.Config{
		Listeners:     listeners,
		OnAccept:      s.inboundPeerConnected,
		RetryDuration: connectionRetryInterval,
		Dial:          s.dialTimeout,
		OnConnection:  s.outboundPeerConnected,
		GetAddr: func(pid [33]byte) (net.Addr, error) {
			na := s.addrManager.GetAddress(pid)
			if na == nil {
				return nil, fmt.Errorf("can not find network"+
					" address for %s", peer.PID(pid))
			}
			return na, nil
		},
	})
	if err != nil {
		return nil, err
	}
	s.connManager = cmgr

	return &s, nil
}

// initListeners initializes the configured net listeners and adds any bound
// addresses to the address manager. Returns the listeners and a NAT interface,
// which is non-nil if UPnP is in use.
func initListeners(cfg Config) ([]net.Listener, error) {
	// Listen for TCP connections at the configured addresses
	netAddrs, err := parseListeners(fmt.Sprintf(":%d", cfg.DefaultPort))
	if err != nil {
		return nil, err
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

	return listeners, nil
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
