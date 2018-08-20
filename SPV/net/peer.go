package net

import (
	"container/list"
	"fmt"
	"io"
	"net"
	"strings"
	"sync/atomic"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/log"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA.Utility/p2p/rw"
)

const (
	// outputBufferSize is the number of elements the output channels use.
	outputBufferSize = 50

	// idleTimeout is the duration of inactivity before we time out a peer.
	idleTimeout = 2 * time.Minute

	// pingInterval is the interval of time to wait in between sending ping
	// messages.
	pingInterval = 30 * time.Second
)

// outMsg is used to house a message to be sent along with a channel to signal
// when the message has been sent (or won't be sent due to things such as
// shutdown)
type outMsg struct {
	msg      p2p.Message
	doneChan chan<- struct{}
}

type PeerConfig struct {
	PingNonce     func() uint32
	PongNonce     func() uint32
	OnVerAck      func(peer *Peer)
	OnGetAddr     func(peer *Peer)
	OnAddr        func(peer *Peer, addr *msg.Addr)
	OnPing        func(peer *Peer, ping *msg.Ping)
	OnPong        func(peer *Peer, pong *msg.Pong)
	HandleMessage func(peer *Peer, msg p2p.Message)
}

type Peer struct {
	id         uint64
	version    uint32
	services   uint64
	ip16       [16]byte
	port       uint16
	lastActive time.Time
	height     uint64
	relay      uint8 // 1 for true 0 for false

	disconnect int32
	conn       net.Conn

	rw            rw.MessageRW
	config        PeerConfig
	outputQueue   chan outMsg
	sendQueue     chan outMsg
	sendDoneQueue chan struct{}
	inQuit        chan struct{}
	queueQuit     chan struct{}
	outQuit       chan struct{}
	quit          chan struct{}
}

func (p *Peer) String() string {
	return fmt.Sprint(
		"ID:", p.id,
		", Version:", p.version,
		", Services:", p.services,
		", Port:", p.port,
		", LastActive:", p.lastActive,
		", Height:", p.height,
		", Relay:", p.relay,
		", Addr:", p.Addr().String())
}

func (p *Peer) ID() uint64 {
	return p.id
}

func (p *Peer) SetID(id uint64) {
	p.id = id
}

func (p *Peer) Version() uint32 {
	return p.version
}

func (p *Peer) SetVersion(version uint32) {
	p.version = version
}

func (p *Peer) Services() uint64 {
	return p.services
}

func (p *Peer) SetServices(services uint64) {
	p.services = services
}

func (p *Peer) IP16() [16]byte {
	return p.ip16
}

func (p *Peer) Port() uint16 {
	return p.port
}

func (p *Peer) SetPort(port uint16) {
	p.port = port
}

func (p *Peer) LastActive() time.Time {
	return p.lastActive
}

func (p *Peer) Addr() *p2p.NetAddress {
	return p2p.NewNetAddress(p.services, p.ip16, p.port, p.id)
}

func (p *Peer) Relay() uint8 {
	return p.relay
}

func (p *Peer) SetRelay(relay uint8) {
	p.relay = relay
}

func (p *Peer) SetInfo(msg *msg.Version) {
	p.id = msg.Nonce
	p.port = msg.Port
	p.version = msg.Version
	p.services = msg.Services
	p.lastActive = time.Now()
	p.height = msg.Height
	p.relay = msg.Relay
}

func (p *Peer) SetHeight(height uint64) {
	p.height = height
}

func (p *Peer) Height() uint64 {
	return p.height
}

// Connected returns whether or not the peer is currently connected.
//
// This function is safe for concurrent access.
func (p *Peer) Connected() bool {
	return atomic.LoadInt32(&p.disconnect) == 0
}

// Disconnect disconnects the peer by closing the connection.  Calling this
// function when the peer is already disconnected or in the process of
// disconnecting will have no effect.
func (p *Peer) Disconnect() {
	// Return if peer already disconnected
	if atomic.AddInt32(&p.disconnect, 1) != 1 {
		return
	}

	p.conn.Close()
	close(p.quit)
}

func (p *Peer) QuitChan() chan struct{} {
	return p.quit
}

// shouldHandleReadError returns whether or not the passed error, which is
// expected to have come from reading from the remote peer in the inHandler,
// should be logged and responded to with a reject message.
func (p *Peer) shouldHandleReadError(err error) bool {
	// No logging or reject message when the peer is being forcibly
	// disconnected.
	if atomic.LoadInt32(&p.disconnect) != 0 {
		return false
	}

	// No logging or reject message when the remote peer has been
	// disconnected.
	if err == io.EOF {
		return false
	}
	if opErr, ok := err.(*net.OpError); ok && !opErr.Temporary() {
		return false
	}

	return true
}

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
			if p.shouldHandleReadError(err) {
				errMsg := fmt.Sprintf("Can't read message from %s: %v", p, err)
				if err != io.ErrUnexpectedEOF {
					log.Errorf(errMsg)
				}

				// Push a reject message for the malformed message and wait for
				// the message to be sent before disconnecting.
				//
				// NOTE: Ideally this would include the command in the header if
				// at least that much of the message was valid, but that is not
				// currently exposed by wire, so just used malformed for the
				// command.
				rejectMsg := msg.NewReject("malformed", msg.RejectMalformed, errMsg)
				// Send the message and block until it has been sent before returning.
				doneChan := make(chan struct{}, 1)
				p.QueueMessage(rejectMsg, doneChan)
				<-doneChan
			}
			break out
		}
		log.Debugf("-----> inHandler [%s] from [0x%x]", rmsg.CMD(), p.id)

		// Handle each message.
		switch m := rmsg.(type) {
		case *msg.VerAck:
			if p.config.OnVerAck != nil {
				p.config.OnVerAck(p)
			}

		case *msg.GetAddr:
			if p.config.OnGetAddr != nil {
				p.config.OnGetAddr(p)
			}

		case *msg.Addr:
			if p.config.OnAddr != nil {
				p.config.OnAddr(p, m)
			}

		case *msg.Ping:
			if p.config.PongNonce != nil {
				p.QueueMessage(msg.NewPong(p.config.PongNonce()), nil)
			}

			if p.config.OnPing != nil {
				p.config.OnPing(p, m)
			}

		case *msg.Pong:
			if p.config.OnPong != nil {
				p.config.OnPong(p, m)
			}

		default:
			if p.config.HandleMessage != nil {
				p.config.HandleMessage(p, rmsg)
			}
		}

		// A message was received so reset the idle timer.
		idleTimer.Reset(idleTimeout)
	}

	// Ensure the idle timer is stopped to avoid leaking the resource.
	idleTimer.Stop()

	// Ensure connection is closed.
	p.Disconnect()

	close(p.inQuit)
}

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
	log.Tracef("Peer queue handler done for %s", p)
}

func (p *Peer) outHandler() {
out:
	for {
		select {
		case msg := <-p.sendQueue:
			err := p.writeMessage(msg.msg)
			if err != nil {
				p.Disconnect()
				if msg.doneChan != nil {
					msg.doneChan <- struct{}{}
				}
				continue
			}
			log.Debugf("-----> outHandler [%s] to [0x%x]", msg.msg.CMD(), p.id)

			if msg.doneChan != nil {
				msg.doneChan <- struct{}{}
			}
			p.sendDoneQueue <- struct{}{}

		case <-p.quit:
			break out
		}
	}

	<-p.queueQuit

	// Drain any wait channels before we go away so we don't leave something
	// waiting for us. We have waited on queueQuit and thus we can be sure
	// that we will not miss anything sent on sendQueue.
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
	log.Tracef("Peer output handler done for %s", p)
}

// pingHandler periodically pings the peer.  It must be run as a goroutine.
func (p *Peer) pingHandler() {
	pingTicker := time.NewTicker(pingInterval)
	defer pingTicker.Stop()

out:
	for {
		select {
		case <-pingTicker.C:
			p.QueueMessage(msg.NewPing(p.config.PingNonce()), nil)

		case <-p.quit:
			break out
		}
	}
}

func (p *Peer) readMessage() (p2p.Message, error) {
	return p.rw.ReadMessage(p.conn)
}

func (p *Peer) writeMessage(msg p2p.Message) error {
	// Don't do anything if we're disconnecting.
	if atomic.LoadInt32(&p.disconnect) != 0 {
		return nil
	}

	return p.rw.WriteMessage(p.conn, msg)
}

func (p *Peer) QueueMessage(msg p2p.Message, doneChan chan<- struct{}) {
	if atomic.LoadInt32(&p.disconnect) != 0 {
		if doneChan != nil {
			go func() {
				doneChan <- struct{}{}
			}()
		}
		return
	}
	p.outputQueue <- outMsg{msg: msg, doneChan: doneChan}
}

func (p *Peer) start() {
	go p.inHandler()
	go p.queueHandler()
	go p.outHandler()
	go p.pingHandler()
}

func (p *Peer) NewVersionMsg() *msg.Version {
	version := new(msg.Version)
	version.Version = p.Version()
	version.Services = p.Services()
	version.TimeStamp = uint32(time.Now().UnixNano())
	version.Port = p.Port()
	version.Nonce = p.ID()
	version.Height = p.Height()
	version.Relay = p.Relay()
	return version
}

func (p *Peer) SetPeerConfig(config PeerConfig) {
	// Set PingNonce method
	if config.PingNonce != nil {
		p.config.PingNonce = config.PingNonce
	}

	// Set OnVerAck method
	if config.OnVerAck != nil {
		p.config.OnVerAck = config.OnVerAck
	}

	// Set OnGetAddr method
	if config.OnGetAddr != nil {
		p.config.OnGetAddr = config.OnGetAddr
	}

	// Set OnGetAddr method
	if config.OnAddr != nil {
		p.config.OnAddr = config.OnAddr
	}

	// Set OnPing method
	if config.OnPing != nil {
		p.config.OnPing = config.OnPing
	}

	// Set OnPong method
	if config.OnPong != nil {
		p.config.OnPong = config.OnPong
	}

	if config.HandleMessage != nil {
		if p.config.HandleMessage == nil {
			// Set message handler
			p.config.HandleMessage = config.HandleMessage

		} else {
			// Upgrade peer message handler
			previousHandler := p.config.HandleMessage
			p.config.HandleMessage = func(peer *Peer, msg p2p.Message) {
				previousHandler(peer, msg)
				config.HandleMessage(peer, msg)
			}
		}
	}
}

func (p *Peer) SetMessageConfig(config rw.MessageConfig) {
	// Set rw config
	p.rw.SetConfig(config)
}

func NewPeer(magic uint32, conn net.Conn) *Peer {
	p := Peer{
		conn:          conn,
		rw:            rw.GetMesssageRW(magic),
		outputQueue:   make(chan outMsg, outputBufferSize),
		sendQueue:     make(chan outMsg, 1),   // nonblocking sync
		sendDoneQueue: make(chan struct{}, 1), // nonblocking sync
		inQuit:        make(chan struct{}),
		queueQuit:     make(chan struct{}),
		outQuit:       make(chan struct{}),
		quit:          make(chan struct{}),
	}

	copy(p.ip16[:], getIp(conn))
	return &p
}

func getIp(conn net.Conn) []byte {
	addr := conn.RemoteAddr().String()
	portIndex := strings.LastIndex(addr, ":")
	return net.ParseIP(string([]byte(addr)[:portIndex])).To16()
}
