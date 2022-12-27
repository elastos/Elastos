// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package peer

import (
	"container/list"
	"errors"
	"fmt"
	"io"
	"math/rand"
	"net"
	"strconv"
	"sync"
	"sync/atomic"
	"time"

	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/elanet/pact"
	elaerr "github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/p2p"
	"github.com/elastos/Elastos.ELA/p2p/msg"
)

const (
	// outputBufferSize is the number of elements the output channels use.
	outputBufferSize = 50

	// idleTimeout is the duration of inactivity before we time out a peer.
	idleTimeout = 2 * time.Minute

	// pingInterval is the interval of time to wait in between sending ping
	// messages.
	pingInterval = 30 * time.Second

	// negotiateTimeout is the duration of inactivity before we timeout a
	// peer that hasn't completed the initial version negotiation.
	negotiateTimeout = 30 * time.Second
)

// outMsg is used to house a message to be sent along with a channel to signal
// when the message has been sent (or won't be sent due to things such as
// shutdown)
type outMsg struct {
	msg      p2p.Message
	doneChan chan<- struct{}
}

// StallControlCmd represents the command of a stall control message.
type StallControlCmd uint8

// Constants for the command of a stall control message.
const (
	// SCCSendMessage indicates a message is being sent to the remote peer.
	SCCSendMessage StallControlCmd = iota

	// SCCReceiveMessage indicates a message has been received from the
	// remote peer.
	SCCReceiveMessage

	// SCCHandlerStart indicates a callback handler is about to be invoked.
	SCCHandlerStart

	// SCCHandlerDone indicates a callback handler has completed.
	SCCHandlerDone
)

// StallControlMsg is used to signal the stall handler about specific events
// so it can properly detect and handle stalled remote peers.
type StallControlMsg struct {
	CMD StallControlCmd
	MSG p2p.Message
}

// StallHandler defines the interfaces to invoke stall control messages.
type StallHandler func(msg StallControlMsg)

// StatsSnap is a snapshot of peer stats at a point in time.
type StatsSnap struct {
	ID             uint64
	Addr           string
	Services       uint64
	RelayTx        uint8
	LastSend       time.Time
	LastRecv       time.Time
	ConnTime       time.Time
	TimeOffset     int64
	Version        uint32
	Inbound        bool
	StartingHeight uint32
	LastBlock      uint32
	LastPingTime   time.Time
	LastPingMicros int64
	NodeVersion    string
}

// MessageFunc is a message handler in peer's configuration
type MessageFunc func(peer *Peer, msg p2p.Message)

// Config is a descriptor which specifies the peer instance configuration.
type Config struct {
	Magic            uint32
	ProtocolVersion  uint32
	DefaultPort      uint16
	Services         uint64
	DisableRelayTx   bool
	HostToNetAddress HostToNetAddrFunc
	MakeEmptyMessage func(cmd string) (p2p.Message, error)
	BestHeight       func() uint64
	IsSelfConnection func(ip net.IP, port int, nonce uint64) bool
	GetVersionNonce  func() uint64
	MessageFunc      MessageFunc
	NewVersionHeight uint64
	NodeVersion      string
}

// minUint32 is a helper function to return the minimum of two uint32s.
// This avoids a math import and the need to cast to floats.
func minUint32(a, b uint32) uint32 {
	if a < b {
		return a
	}
	return b
}

// newNetAddress attempts to extract the IP address and port from the passed
// net.Addr interface and create a NetAddress structure using that information.
func newNetAddress(addr net.Addr, services uint64) (*p2p.NetAddress, error) {
	// addr will be a net.TCPAddr when not using a proxy.
	if tcpAddr, ok := addr.(*net.TCPAddr); ok {
		ip := tcpAddr.IP
		port := uint16(tcpAddr.Port)
		na := p2p.NewNetAddressIPPort(ip, port, services)
		return na, nil
	}

	// For the most part, addr should be one of the two above cases, but
	// to be safe, fall back to trying to parse the information from the
	// address string as a last resort.
	host, portStr, err := net.SplitHostPort(addr.String())
	if err != nil {
		return nil, err
	}
	ip := net.ParseIP(host)
	port, err := strconv.ParseUint(portStr, 10, 16)
	if err != nil {
		return nil, err
	}
	na := p2p.NewNetAddressIPPort(ip, uint16(port), services)
	return na, nil
}

// HostToNetAddrFunc is a func which takes a host, port, services and returns the netaddress.
type HostToNetAddrFunc func(host string, port uint16, services uint64) (*p2p.NetAddress, error)

type Peer struct {
	// The following variables must only be used atomically.
	lastRecv   int64
	lastSend   int64
	connected  int32
	disconnect int32

	conn net.Conn

	// These fields are set at creation time and never modified, so they are
	// safe to read from concurrently without a mutex.
	addr         string
	cfg          Config
	inbound      bool
	messageFuncs []MessageFunc
	stallHandle  StallHandler

	flagsMtx               sync.Mutex // protects the peer flags below
	na                     *p2p.NetAddress
	id                     uint64
	services               uint64
	versionKnown           bool
	advertisedProtoVer     uint32 // protocol version advertised by remote
	protocolVersion        uint32 // negotiated protocol version
	advertisedProtoNodeVer string // protocol node version advertised by remote
	verAckReceived         bool

	// These fields keep track of statistics for the peer and are protected
	// by the statsMtx mutex.
	statsMtx       sync.RWMutex
	timeOffset     int64
	timeConnected  time.Time
	startingHeight uint32
	height         uint32
	lastPingTime   time.Time // Time we sent last ping.
	lastPingMicros int64     // Time for last ping to return.

	stallControl  chan StallControlMsg
	outputQueue   chan outMsg
	sendQueue     chan outMsg
	sendDoneQueue chan struct{}
	inQuit        chan struct{}
	queueQuit     chan struct{}
	outQuit       chan struct{}
	quit          chan struct{}
}

// AddMessageFunc add a new message handler for the peer.
func (p *Peer) AddMessageFunc(messageFunc MessageFunc) {
	if messageFunc != nil {
		p.messageFuncs = append(p.messageFuncs, messageFunc)
	}
}

// handleMessage will be invoked when a message received.
func (p *Peer) handleMessage(peer *Peer, msg p2p.Message) {
	for _, messageFunc := range p.messageFuncs {
		messageFunc(peer, msg)
	}
}

// SetStallHandler set the stall handler for the peer.
func (p *Peer) SetStallHandler(handler StallHandler) {
	p.stallHandle = handler
}

// String returns the peer's address and directionality as a human-readable
// string.
//
// This function is safe for concurrent access.
func (p *Peer) String() string {
	return fmt.Sprintf("%s (%s) (%d %s)", p.addr, directionString(p.inbound), p.advertisedProtoVer, p.advertisedProtoNodeVer)
}

// UpdateHeight updates the last known block for the peer.
//
// This function is safe for concurrent access.
func (p *Peer) UpdateHeight(newHeight uint32) {
	p.statsMtx.Lock()
	p.height = newHeight
	p.statsMtx.Unlock()
}

// StatsSnapshot returns a snapshot of the current peer flags and statistics.
//
// This function is safe for concurrent access.
func (p *Peer) StatsSnapshot() *StatsSnap {
	p.statsMtx.RLock()

	p.flagsMtx.Lock()
	id := p.id
	addr := p.addr
	services := p.services
	protocolVersion := p.advertisedProtoVer
	nodeVersion := p.advertisedProtoNodeVer
	p.flagsMtx.Unlock()

	// Get a copy of all relevant flags and stats.
	statsSnap := &StatsSnap{
		ID:             id,
		Addr:           addr,
		Services:       services,
		LastSend:       p.LastSend(),
		LastRecv:       p.LastRecv(),
		ConnTime:       p.timeConnected,
		TimeOffset:     p.timeOffset,
		Version:        protocolVersion,
		Inbound:        p.inbound,
		StartingHeight: p.startingHeight,
		LastBlock:      p.height,
		LastPingMicros: p.lastPingMicros,
		LastPingTime:   p.lastPingTime,
		NodeVersion:    nodeVersion,
	}

	p.statsMtx.RUnlock()
	return statsSnap
}

// ID returns the peer id.
//
// This function is safe for concurrent access.
func (p *Peer) ID() uint64 {
	p.flagsMtx.Lock()
	id := p.id
	p.flagsMtx.Unlock()

	return id
}

// NA returns the peer network address.
//
// This function is safe for concurrent access.
func (p *Peer) NA() *p2p.NetAddress {
	p.flagsMtx.Lock()
	na := p.na
	p.flagsMtx.Unlock()

	return na
}

func (p *Peer) SetNA(addr net.Addr) bool {
	na, err := newNetAddress(addr, p.services)
	if err != nil {
		log.Errorf("SetNA Cannot create remote net address: %v", err)
		return false
	}
	p.flagsMtx.Lock()
	p.na = na
	p.flagsMtx.Unlock()

	return true
}

// Addr returns the peer address.
//
// This function is safe for concurrent access.
func (p *Peer) Addr() string {
	// The address doesn't change after initialization, therefore it is not
	// protected by a mutex.
	return p.addr
}

func (p *Peer) Host() (string, error) {
	host, _, err := net.SplitHostPort(p.addr)
	if err != nil {
		return "", err
	}
	return host, nil
}

func (p *Peer) SetAddr(addr string) {
	// The address doesn't change after initialization, therefore it is not
	// protected by a mutex.
	p.addr = addr
}

// Inbound returns whether the peer is inbound.
//
// This function is safe for concurrent access.
func (p *Peer) Inbound() bool {
	return p.inbound
}

// Services returns the services flag of the remote peer.
//
// This function is safe for concurrent access.
func (p *Peer) Services() uint64 {
	p.flagsMtx.Lock()
	services := p.services
	p.flagsMtx.Unlock()

	return services
}

// LastPingTime returns the last ping time of the remote peer.
//
// This function is safe for concurrent access.
func (p *Peer) LastPingTime() time.Time {
	p.statsMtx.RLock()
	lastPingTime := p.lastPingTime
	p.statsMtx.RUnlock()

	return lastPingTime
}

// LastPingMicros returns the last ping micros of the remote peer.
//
// This function is safe for concurrent access.
func (p *Peer) LastPingMicros() int64 {
	p.statsMtx.RLock()
	lastPingMicros := p.lastPingMicros
	p.statsMtx.RUnlock()

	return lastPingMicros
}

// VersionKnown returns the whether or not the version of a peer is known
// locally.
//
// This function is safe for concurrent access.
func (p *Peer) VersionKnown() bool {
	p.flagsMtx.Lock()
	versionKnown := p.versionKnown
	p.flagsMtx.Unlock()

	return versionKnown
}

// VerAckReceived returns whether or not a verack message was received by the
// peer.
//
// This function is safe for concurrent access.
func (p *Peer) VerAckReceived() bool {
	p.flagsMtx.Lock()
	verAckReceived := p.verAckReceived
	p.flagsMtx.Unlock()

	return verAckReceived
}

// ProtocolVersion returns the negotiated peer protocol version.
//
// This function is safe for concurrent access.
func (p *Peer) ProtocolVersion() uint32 {
	p.flagsMtx.Lock()
	protocolVersion := p.protocolVersion
	p.flagsMtx.Unlock()

	return protocolVersion
}

// Height returns the height of the peer.
//
// This function is safe for concurrent access.
func (p *Peer) Height() uint32 {
	p.statsMtx.RLock()
	lastBlock := p.height
	p.statsMtx.RUnlock()

	return lastBlock
}

// LastSend returns the last send time of the peer.
//
// This function is safe for concurrent access.
func (p *Peer) LastSend() time.Time {
	return time.Unix(atomic.LoadInt64(&p.lastSend), 0)
}

// LastRecv returns the last recv time of the peer.
//
// This function is safe for concurrent access.
func (p *Peer) LastRecv() time.Time {
	return time.Unix(atomic.LoadInt64(&p.lastRecv), 0)
}

// LocalAddr returns the local address of the connection.
//
// This function is safe fo concurrent access.
func (p *Peer) LocalAddr() net.Addr {
	var localAddr net.Addr
	if atomic.LoadInt32(&p.connected) != 0 {
		localAddr = p.conn.LocalAddr()
	}
	return localAddr
}

// TimeConnected returns the time at which the peer connected.
//
// This function is safe for concurrent access.
func (p *Peer) TimeConnected() time.Time {
	p.statsMtx.RLock()
	timeConnected := p.timeConnected
	p.statsMtx.RUnlock()

	return timeConnected
}

// TimeOffset returns the number of seconds the local time was offset from the
// time the peer reported during the initial negotiation phase.  Negative values
// indicate the remote peer's time is before the local time.
//
// This function is safe for concurrent access.
func (p *Peer) TimeOffset() int64 {
	p.statsMtx.RLock()
	timeOffset := p.timeOffset
	p.statsMtx.RUnlock()

	return timeOffset
}

// StartingHeight returns the last known height the peer reported during the
// initial negotiation phase.
//
// This function is safe for concurrent access.
func (p *Peer) StartingHeight() uint32 {
	p.statsMtx.RLock()
	startingHeight := p.startingHeight
	p.statsMtx.RUnlock()

	return startingHeight
}

// PushAddrMsg sends an addr message to the connected peer using the provided
// addresses.  This function is useful over manually sending the message via
// QueueMessage since it automatically limits the addresses to the maximum
// number allowed by the message and randomizes the chosen addresses when there
// are too many.  It returns the addresses that were actually sent and no
// message will be sent if there are no entries in the provided addresses slice.
//
// This function is safe for concurrent access.
func (p *Peer) PushAddrMsg(addresses []*p2p.NetAddress) []*p2p.NetAddress {
	addressCount := len(addresses)

	// Nothing to send.
	if addressCount == 0 {
		return nil
	}

	addr := msg.NewAddr(addresses)

	// Randomize the addresses sent if there are more than the maximum allowed.
	if addressCount > msg.MaxAddrPerMsg {
		// Shuffle the address list.
		for i := 0; i < msg.MaxAddrPerMsg; i++ {
			j := i + rand.Intn(addressCount-i)
			addr.AddrList[i], addr.AddrList[j] = addr.AddrList[j], addr.AddrList[i]
		}

		// Truncate it to the maximum size.
		addr.AddrList = addr.AddrList[:msg.MaxAddrPerMsg]
	}

	p.QueueMessage(addr, nil)
	return addr.AddrList
}

// handlePingMsg is invoked when a peer receives a ping message.
func (p *Peer) handlePingMsg(ping *msg.Ping) {
	// Update peer height when height has changed.
	p.statsMtx.Lock()
	newHeight := uint32(ping.Nonce)
	if p.height != newHeight {
		p.height = newHeight
	}
	p.statsMtx.Unlock()
	p.QueueMessage(msg.NewPong(p.cfg.BestHeight()), nil)
}

// handlePongMsg is invoked when a peer receives a pong message.
func (p *Peer) handlePongMsg(pong *msg.Pong) {
	p.statsMtx.Lock()
	p.height = uint32(pong.Nonce)
	p.lastPingMicros = time.Since(p.lastPingTime).Nanoseconds()
	p.lastPingMicros /= 1000 // convert to usec.
	p.statsMtx.Unlock()
}

func (p *Peer) makeEmptyMessage(cmd string) (p2p.Message, error) {
	var message p2p.Message
	switch cmd {
	case p2p.CmdVersion:
		message = &msg.Version{}

	case p2p.CmdVerAck:
		message = &msg.VerAck{}

	case p2p.CmdGetAddr:
		message = &msg.GetAddr{}

	case p2p.CmdAddr:
		message = &msg.Addr{}

	case p2p.CmdPing:
		message = &msg.Ping{}

	case p2p.CmdPong:
		message = &msg.Pong{}

	default:
		return p.cfg.MakeEmptyMessage(cmd)
	}
	return message, nil
}

func (p *Peer) readMessage() (p2p.Message, error) {
	msg, err := p2p.ReadMessage(
		p.conn, p.cfg.Magic, p2p.ReadMessageTimeOut, p.makeEmptyMessage)
	// Use closures to log expensive operations so they are only run when
	// the logging level requires it.
	log.Debugf("%v", newLogClosure(func() string {
		if err != nil {
			return fmt.Sprintf("Read message failed, %s", err)
		}

		// Debug summary of message.
		summary := messageSummary(msg)
		if len(summary) > 0 {
			summary = " (" + summary + ")"
		}
		return fmt.Sprintf("Received %v%s from %s", msg.CMD(), summary, p)
	}))

	return msg, err
}

func (p *Peer) writeMessage(m p2p.Message) error {
	// Don't do anything if we're disconnecting.
	if atomic.LoadInt32(&p.disconnect) != 0 {
		return nil
	}

	// Use closures to log expensive operations so they are only run when
	// the logging level requires it.
	log.Debugf("%v", newLogClosure(func() string {
		// Debug summary of message.
		summary := messageSummary(m)
		if len(summary) > 0 {
			summary = " (" + summary + ")"
		}
		return fmt.Sprintf("Sending %v%s to %s", m.CMD(), summary, p)
	}))

	// Write the message to the peer.
	return p2p.WriteMessage(p.conn, p.cfg.Magic, m, p2p.WriteMessageTimeOut,
		func(message p2p.Message) (*types.DposBlock, bool) {
			msgBlock, ok := message.(*msg.Block)
			if !ok {
				return nil, false
			}
			dposBlock, ok := msgBlock.Serializable.(*types.DposBlock)
			return dposBlock, ok
		})
}

// shouldHandleIOError returns whether or not the passed error, which is
// expected to have come from reading or writing should be handled.
func (p *Peer) shouldHandleIOError(err error) bool {
	// No logging when the peer is being forcibly disconnected.
	if atomic.LoadInt32(&p.disconnect) != 0 {
		return false
	}

	// No logging when the remote peer has been disconnected.
	if err == io.EOF {
		return false
	}
	if opErr, ok := err.(*net.OpError); ok && !opErr.Temporary() {
		return false
	}

	return true
}

// stallHandler handles stall detection for the peer.  This entails keeping
// track of expected responses and assigning them deadlines while accounting for
// the time spent in callbacks.  It must be run as a goroutine.
func (p *Peer) stallHandler() {
	// ioStopped is used to detect when both the input and output handler
	// goroutines are done.
	var ioStopped bool
out:
	for {
		select {
		case msg := <-p.stallControl:
			// Invoke stall control message to upper layer if configured.
			if p.stallHandle != nil {
				p.stallHandle(msg)
			}

		case <-p.inQuit:
			// The stall handler can exit once both the input and
			// output handler goroutines are done.
			if ioStopped {
				break out
			}
			ioStopped = true

		case <-p.outQuit:
			// The stall handler can exit once both the input and
			// output handler goroutines are done.
			if ioStopped {
				break out
			}
			ioStopped = true
		}
	}

	// Drain any wait channels before going away so there is nothing left
	// waiting on this goroutine.
cleanup:
	for {
		select {
		case <-p.stallControl:
		default:
			break cleanup
		}
	}
}

// inHandler handles all incoming messages for the peer.  It must be run as a
// goroutine.
func (p *Peer) inHandler() {
	// The timer is stopped when a new message is received and reset after it
	// is processed.
	idleTimer := time.AfterFunc(idleTimeout, func() {
		log.Warnf("Peer %s no answer for %s -- disconnecting", p, idleTimeout)
		p.Disconnect()
	})

out:
	for atomic.LoadInt32(&p.disconnect) == 0 {
		// Read a message and stop the idle timer as soon as the read
		// is done.  The timer is reset below for the next iteration if
		// needed.
		rmsg, err := p.readMessage()
		idleTimer.Stop()
		if err != nil {
			// Only log the error and send reject message if the
			// local peer is not forcibly disconnecting and the
			// remote peer has not disconnected.
			if p.shouldHandleIOError(err) {
				elaErr := elaerr.SimpleWithMessage(elaerr.ErrP2pRejectMalformed,
					nil, fmt.Sprintf(
						"Can't read message from %s: %v", p, err))
				if err != io.ErrUnexpectedEOF {
					log.Errorf(elaErr.Error())
				}

				// Push a reject message for the malformed message and wait for
				// the message to be sent before disconnecting.
				//
				// NOTE: Ideally this would include the command in the header if
				// at least that much of the message was valid, but that is not
				// currently exposed by wire, so just used malformed for the
				// command.
				rejectMsg := msg.NewReject("malformed", elaErr)
				// Send the message and block until it has been sent before returning.
				doneChan := make(chan struct{}, 1)
				p.QueueMessage(rejectMsg, doneChan)
				<-doneChan
			}
			break out
		}
		atomic.StoreInt64(&p.lastRecv, time.Now().Unix())
		p.stallControl <- StallControlMsg{SCCReceiveMessage, rmsg}

		// Handle each supported message type.
		p.stallControl <- StallControlMsg{SCCHandlerStart, rmsg}
		switch m := rmsg.(type) {
		case *msg.Version:
			elaErr := elaerr.SimpleWithMessage(elaerr.ErrP2pRejectDuplicate,
				nil, "duplicate version message")
			rejectMsg := msg.NewReject(m.CMD(), elaErr)
			// Send the message and block until it has been sent before returning.
			doneChan := make(chan struct{}, 1)
			p.QueueMessage(rejectMsg, doneChan)
			<-doneChan
			break out

		case *msg.VerAck:

			// No read lock is necessary because verAckReceived is not written
			// to in any other goroutine.
			if p.verAckReceived {
				log.Infof("Already received 'verack' from peer %v -- disconnecting", p)
				break out
			}
			p.flagsMtx.Lock()
			p.verAckReceived = true
			p.flagsMtx.Unlock()

		case *msg.Ping:
			p.handlePingMsg(m)

		case *msg.Pong:
			p.handlePongMsg(m)
		}

		// Call handle message which is configured on peer creation.
		p.handleMessage(p, rmsg)
		p.stallControl <- StallControlMsg{SCCHandlerDone, rmsg}

		// A message was received so reset the idle timer.
		idleTimer.Reset(idleTimeout)
	}

	// Ensure the idle timer is stopped to avoid leaking the resource.
	idleTimer.Stop()

	// Ensure connection is closed.
	p.Disconnect()

	close(p.inQuit)
}

// queueHandler handles the queuing of outgoing data for the peer. This runs as
// a muxer for various sources of input so we can ensure that server and peer
// handlers will not block on us sending a message.  That data is then passed on
// to outHandler to be actually written.
func (p *Peer) queueHandler() {
	pendingMsgs := list.New()

	// We keep the waiting flag so that we know if we have a message queued
	// to the outHandler or not.  We could use the presence of a head of
	// the list for this but then we have rather racy concerns about whether
	// it has gotten it at cleanup time - and thus who sends on the
	// message's done channel.  To avoid such confusion we keep a different
	// flag and pendingMsgs only contains messages that we have not yet
	// passed to outHandler.
	waiting := false

	// To avoid duplication below.
	queuePacket := func(msg outMsg, list *list.List, waiting bool) bool {
		if !waiting {
			p.sendQueue <- msg
		} else {
			list.PushBack(msg)
		}
		// we are always waiting now.
		return true
	}
out:
	for {
		select {
		case msg := <-p.outputQueue:
			waiting = queuePacket(msg, pendingMsgs, waiting)

		// This channel is notified when a message has been sent across
		// the network socket.
		case <-p.sendDoneQueue:
			// No longer waiting if there are no more messages
			// in the pending messages queue.
			next := pendingMsgs.Front()
			if next == nil {
				waiting = false
				continue
			}

			// Notify the outHandler about the next item to
			// asynchronously send.
			val := pendingMsgs.Remove(next)
			p.sendQueue <- val.(outMsg)

		case <-p.quit:
			break out
		}
	}

	// Drain any wait channels before we go away so we don't leave something
	// waiting for us.
	for e := pendingMsgs.Front(); e != nil; e = pendingMsgs.Front() {
		val := pendingMsgs.Remove(e)
		msg := val.(outMsg)
		if msg.doneChan != nil {
			msg.doneChan <- struct{}{}
		}
	}
cleanup:
	for {
		select {
		case msg := <-p.outputQueue:
			if msg.doneChan != nil {
				msg.doneChan <- struct{}{}
			}
		default:
			break cleanup
		}
	}
	close(p.queueQuit)
}

// outHandler handles all outgoing messages for the peer.  It must be run as a
// goroutine.  It uses a buffered channel to serialize output messages while
// allowing the sender to continue running asynchronously.
func (p *Peer) outHandler() {
out:
	for {
		select {
		case smsg := <-p.sendQueue:
			switch smsg.msg.(type) {
			case *msg.Ping:
				// Update last ping time
				p.statsMtx.Lock()
				p.lastPingTime = time.Now()
				p.statsMtx.Unlock()
			}

			p.stallControl <- StallControlMsg{SCCSendMessage, smsg.msg}

			err := p.writeMessage(smsg.msg)
			if err != nil {
				p.Disconnect()
				if p.shouldHandleIOError(err) {
					log.Errorf("Failed to send message to %s: %v", p, err)
				}
				if smsg.doneChan != nil {
					smsg.doneChan <- struct{}{}
				}
				continue
			}

			// At this point, the message was successfully sent, so
			// update the last send time, signal the sender of the
			// message that it has been sent (if requested), and
			// signal the send queue to deliver the next queued message.
			atomic.StoreInt64(&p.lastSend, time.Now().Unix())
			if smsg.doneChan != nil {
				smsg.doneChan <- struct{}{}
			}
			p.sendDoneQueue <- struct{}{}

		case <-p.quit:
			break out
		}
	}

	<-p.queueQuit

	// Drain any wait channels before going away so there is nothing left
	// waiting on this goroutine.
cleanup:
	for {
		select {
		case msg := <-p.sendQueue:
			if msg.doneChan != nil {
				msg.doneChan <- struct{}{}
			}
			// no need to send on sendDoneQueue since queueHandler
			// has been waited on and already exited.
		default:
			break cleanup
		}
	}
	close(p.outQuit)
}

// pingHandler periodically pings the peer.  It must be run as a goroutine.
func (p *Peer) pingHandler() {
	pingTicker := time.NewTicker(pingInterval)
	defer pingTicker.Stop()

out:
	for {
		select {
		case <-pingTicker.C:
			p.QueueMessage(msg.NewPing(p.cfg.BestHeight()), nil)

		case <-p.quit:
			break out
		}
	}
}

// QueueMessage adds the passed bitcoin message to the peer send queue.
//
// This function is safe for concurrent access.
func (p *Peer) QueueMessage(msg p2p.Message, doneChan chan<- struct{}) {
	// Avoid risk of deadlock if goroutine already exited.  The goroutine
	// we will be sending to hangs around until it knows for a fact that
	// it is marked as disconnected and *then* it drains the channels.
	if !p.Connected() {
		if doneChan != nil {
			go func() {
				doneChan <- struct{}{}
			}()
		}
		return
	}
	p.outputQueue <- outMsg{msg: msg, doneChan: doneChan}
}

// Connected returns whether or not the peer is currently connected.
//
// This function is safe for concurrent access.
func (p *Peer) Connected() bool {
	return atomic.LoadInt32(&p.connected) != 0 &&
		atomic.LoadInt32(&p.disconnect) == 0

}

// Disconnect disconnects the peer by closing the connection.  Calling this
// function when the peer is already disconnected or in the process of
// disconnecting will have no effect.
func (p *Peer) Disconnect() {
	if atomic.AddInt32(&p.disconnect, 1) != 1 {
		return
	}

	if atomic.LoadInt32(&p.connected) != 0 {
		p.conn.Close()
	}
	close(p.quit)
}

// handleRemoteVersionMsg is invoked when a version message is received
// from the remote peer.  It will return an error if the remote peer's version
// is not compatible with ours.
func (p *Peer) handleRemoteVersionMsg(msg *msg.Version) error {
	// Detect self connections.
	if p.cfg.IsSelfConnection(p.na.IP, int(msg.Port), msg.Nonce) {
		return errors.New("disconnecting peer connected to self")
	}

	// Updating a bunch of stats including block based stats, and the
	// peer's time offset.
	p.statsMtx.Lock()
	p.height = uint32(msg.Height)
	p.startingHeight = p.height
	p.timeOffset = msg.Timestamp.Unix() - time.Now().Unix()
	p.statsMtx.Unlock()

	// Negotiate the protocol version.
	p.flagsMtx.Lock()
	p.advertisedProtoVer = uint32(msg.Version)
	p.advertisedProtoNodeVer = msg.NodeVersion
	p.protocolVersion = minUint32(p.protocolVersion, p.advertisedProtoVer)
	p.versionKnown = true
	log.Debugf("Negotiated protocol version %d for peer %s", p.protocolVersion, p)

	// Set the peer's ID.
	p.id = msg.Nonce

	// Set the supported services for the peer to what the remote peer advertised.
	p.services = msg.Services
	p.flagsMtx.Unlock()

	return nil
}

// readRemoteVersionMsg waits for the next message to arrive from the remote
// peer.  If the next message is not a version message or the version is not
// acceptable then return an error.
func (p *Peer) readRemoteVersionMsg() error {
	// Read their version message.
	message, err := p.readMessage()
	if err != nil {
		return err
	}

	remoteVerMsg, ok := message.(*msg.Version)
	if !ok {
		elaErr := elaerr.SimpleWithMessage(elaerr.ErrP2pRejectMalformed,
			nil, "A version message must precede all others")
		log.Error(elaErr)

		rejectMsg := msg.NewReject(message.CMD(), elaErr)
		return p.writeMessage(rejectMsg)
	}

	if err := p.handleRemoteVersionMsg(remoteVerMsg); err != nil {
		return err
	}

	p.handleMessage(p, remoteVerMsg)
	return nil
}

// localVersionMsg creates a version message that can be used to send to the
// remote peer.
func (p *Peer) localVersionMsg() (*msg.Version, error) {
	// Generate a unique nonce for this peer so self connections can be
	// detected.  This is accomplished by adding it to a size-limited map of
	// recently seen nonces.
	nonce := p.cfg.GetVersionNonce()

	var m *msg.Version
	bestHeight := p.cfg.BestHeight()
	var nodeVersion string
	var ver = p.cfg.ProtocolVersion
	if bestHeight >= p.cfg.NewVersionHeight {
		nodeVersion = p.cfg.NodeVersion
		ver = pact.CRProposalVersion
		p.cfg.ProtocolVersion = ver
	}
	// Version message.
	m = msg.NewVersion(ver, p.cfg.DefaultPort,
		p.cfg.Services, nonce, bestHeight, p.cfg.DisableRelayTx, nodeVersion)

	return m, nil
}

// writeLocalVersionMsg writes our version message to the remote peer.
func (p *Peer) writeLocalVersionMsg() error {
	localVerMsg, err := p.localVersionMsg()
	if err != nil {
		return err
	}

	return p.writeMessage(localVerMsg)
}

// negotiateInboundProtocol waits to receive a version message from the peer
// then sends our version message. If the events do not occur in that order then
// it returns an error.
func (p *Peer) negotiateInboundProtocol() error {
	if err := p.readRemoteVersionMsg(); err != nil {
		return err
	}

	return p.writeLocalVersionMsg()
}

// negotiateOutboundProtocol sends our version message then waits to receive a
// version message from the peer.  If the events do not occur in that order then
// it returns an error.
func (p *Peer) negotiateOutboundProtocol() error {
	if err := p.writeLocalVersionMsg(); err != nil {
		return err
	}

	return p.readRemoteVersionMsg()
}

// start begins processing input and output messages.
func (p *Peer) start() error {
	negotiateErr := make(chan error, 1)
	go func() {
		if p.inbound {
			negotiateErr <- p.negotiateInboundProtocol()
		} else {
			negotiateErr <- p.negotiateOutboundProtocol()
		}
	}()

	// Negotiate the protocol within the specified negotiateTimeout.
	select {
	case err := <-negotiateErr:
		if err != nil {
			return err
		}
	case <-time.After(negotiateTimeout):
		return errors.New("protocol negotiation timeout")
	}
	log.Debugf("Connected to %s", p.Addr())

	// The protocol has been negotiated successfully so start processing input
	// and output messages.
	go p.stallHandler()
	go p.inHandler()
	go p.queueHandler()
	go p.outHandler()
	go p.pingHandler()

	// Send our verack message now that the IO processing machinery has started.
	p.QueueMessage(msg.NewVerAck(), nil)
	return nil
}

// AssociateConnection associates the given conn to the peer.   Calling this
// function when the peer is already connected will have no effect.
func (p *Peer) AssociateConnection(conn net.Conn) {
	// Already connected?
	if !atomic.CompareAndSwapInt32(&p.connected, 0, 1) {
		return
	}

	p.conn = conn
	p.timeConnected = time.Now()
	go func() {
		if err := p.start(); err != nil {
			log.Debugf("Cannot negotiate peer %v: %v", p, err)
			p.Disconnect()
		}
	}()
}

// WaitForDisconnect waits until the peer has completely disconnected and all
// resources are cleaned up.  This will happen if either the local or remote
// side has been disconnected or the peer is forcibly disconnected via Disconnect.
func (p *Peer) WaitForDisconnect() {
	<-p.quit
}

// newPeerBase returns a new base peer based on the inbound flag.  This
// is used by the NewInboundPeer and NewOutboundPeer functions to perform base
// setup needed by both types of peers.
func newPeerBase(origCfg *Config, inbound bool) *Peer {
	cfg := *origCfg // Copy to avoid mutating caller.

	p := Peer{
		inbound:         inbound,
		stallControl:    make(chan StallControlMsg, 1), // nonblocking sync
		outputQueue:     make(chan outMsg, outputBufferSize),
		sendQueue:       make(chan outMsg, 1),   // nonblocking sync
		sendDoneQueue:   make(chan struct{}, 1), // nonblocking sync
		inQuit:          make(chan struct{}),
		queueQuit:       make(chan struct{}),
		outQuit:         make(chan struct{}),
		quit:            make(chan struct{}),
		cfg:             cfg, // Copy so caller can't mutate.
		services:        cfg.Services,
		protocolVersion: cfg.ProtocolVersion,
	}
	p.AddMessageFunc(cfg.MessageFunc)
	return &p
}

// NewInboundPeer returns a new inbound peer. Use Start to begin
// processing incoming and outgoing messages.
func NewInboundPeer(cfg *Config) *Peer {
	return newPeerBase(cfg, true)
}

// NewOutboundPeer returns a new outbound peer.
func NewOutboundPeer(cfg *Config, addr string) (*Peer, error) {
	p := newPeerBase(cfg, false)
	p.addr = addr

	host, portStr, err := net.SplitHostPort(addr)
	if err != nil {
		return nil, err
	}

	port, err := strconv.ParseUint(portStr, 10, 16)
	if err != nil {
		return nil, err
	}

	if cfg.HostToNetAddress != nil {
		na, err := cfg.HostToNetAddress(host, uint16(port), cfg.Services)
		if err != nil {
			return nil, err
		}
		p.na = na
	} else {
		p.na = p2p.NewNetAddressIPPort(net.ParseIP(host), uint16(port),
			cfg.Services)
	}

	return p, nil
}
