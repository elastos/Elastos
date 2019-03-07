package peer

import (
	"container/list"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/elanet/pact"
	"github.com/elastos/Elastos.ELA/p2p"
	"github.com/elastos/Elastos.ELA/p2p/msg"
	"github.com/elastos/Elastos.ELA/p2p/peer"
	"github.com/elastos/Elastos.ELA/p2p/server"
)

const (
	// trickleInterval is the min time between attempts to send an
	// inv message to a peer.
	trickleInterval = 10 * time.Second

	// outputBufferSize is the number of elements the output channels use.
	outputBufferSize = 50

	// invTrickleSize is the maximum amount of inventory to send in a single
	// message when trickling inventory to remote peers.
	maxInvTrickleSize = 1000

	// maxKnownInventory is the maximum number of items to keep in the known
	// inventory cache.
	maxKnownInventory = 1000

	// stallTickInterval is the interval of time between each check for
	// stalled peers.
	stallTickInterval = 15 * time.Second

	// stallResponseTimeout is the base maximum amount of time messages that
	// expect a response will wait before disconnecting the peer for
	// stalling.  The deadlines are adjusted for callback running times and
	// only checked on each stall tick interval.
	stallResponseTimeout = 30 * time.Second
)

var (
	// zeroHash is the zero value hash (all zeros).  It is defined as a
	// convenience.
	zeroHash common.Uint256
)

type Listeners struct {
	// OnMemPool is invoked when a peer receives a mempool message.
	OnMemPool func(p *Peer, msg *msg.MemPool)

	// OnTx is invoked when a peer receives a tx message.
	OnTx func(p *Peer, msg *msg.Tx)

	// OnBlock is invoked when a peer receives a block message.
	OnBlock func(p *Peer, msg *msg.Block)

	// OnInv is invoked when a peer receives an inv message.
	OnInv func(p *Peer, msg *msg.Inv)

	// OnNotFound is invoked when a peer receives a notfound
	// message.
	OnNotFound func(p *Peer, msg *msg.NotFound)

	// OnGetData is invoked when a peer receives a getdata message.
	OnGetData func(p *Peer, msg *msg.GetData)

	// OnGetBlocks is invoked when a peer receives a getblocks
	// message.
	OnGetBlocks func(p *Peer, msg *msg.GetBlocks)

	// OnFilterAdd is invoked when a peer receives a filteradd message.
	OnFilterAdd func(p *Peer, msg *msg.FilterAdd)

	// OnFilterClear is invoked when a peer receives a filterclear
	// message.
	OnFilterClear func(p *Peer, msg *msg.FilterClear)

	// OnFilterLoad is invoked when a peer receives a filterload
	// message.
	OnFilterLoad func(p *Peer, msg *msg.FilterLoad)

	// OnTxFilterLoad is invoked when a peer receives a txfilter message.
	OnTxFilterLoad func(p *Peer, msg *msg.TxFilterLoad)

	// OnReject is invoked when a peer receives a reject message.
	OnReject func(p *Peer, msg *msg.Reject)
}

// outMsg is used to house a message to be sent along with a channel to signal
// when the message has been sent (or won't be sent due to things such as
// shutdown)
type outMsg struct {
	msg      p2p.Message
	doneChan chan<- struct{}
}

// stallControlCmd represents the command of a stall control message.
type stallControlCmd uint8

// Constants for the command of a stall control message.
const (
	// sccSendMessage indicates a message is being sent to the remote peer.
	sccSendMessage stallControlCmd = iota

	// sccReceiveMessage indicates a message has been received from the
	// remote peer.
	sccReceiveMessage

	// sccHandlerStart indicates a callback handler is about to be invoked.
	sccHandlerStart

	// sccHandlerStart indicates a callback handler has completed.
	sccHandlerDone
)

// stallControlMsg is used to signal the stall handler about specific events
// so it can properly detect and handle stalled remote peers.
type stallControlMsg struct {
	command stallControlCmd
	message p2p.Message
}

type Peer struct {
	*peer.Peer
	server.IPeer

	relayMtx       sync.Mutex
	disableRelayTx bool

	knownInventory     *mruInventoryMap
	prevGetBlocksMtx   sync.Mutex
	prevGetBlocksBegin *common.Uint256
	prevGetBlocksStop  *common.Uint256

	stallControl  chan stallControlMsg
	outputQueue   chan outMsg
	sendDoneQueue chan struct{}
	outputInvChan chan *msg.InvVect
}

// Services returns the services flag of the remote peer.
//
// This function is safe for concurrent access.
func (p *Peer) Services() pact.ServiceFlag {
	return pact.ServiceFlag(p.Peer.Services())
}

// setDisableRelayTx toggles relaying of transactions for the given peer.
// It is safe for concurrent access.
func (p *Peer) SetDisableRelayTx(disable bool) {
	p.relayMtx.Lock()
	p.disableRelayTx = disable
	p.relayMtx.Unlock()
}

// relayTxDisabled returns whether or not relaying of transactions for the given
// peer is disabled.
// It is safe for concurrent access.
func (p *Peer) RelayTxDisabled() bool {
	p.relayMtx.Lock()
	isDisabled := p.disableRelayTx
	p.relayMtx.Unlock()

	return isDisabled
}

// AddKnownInventory adds the passed inventory to the cache of known inventory
// for the peer.
//
// This function is safe for concurrent access.
func (p *Peer) AddKnownInventory(invVect *msg.InvVect) {
	p.knownInventory.Add(invVect)
}

// PushGetBlocksMsg sends a getblocks message for the provided block locator
// and stop hash.  It will ignore back-to-back duplicate requests.
//
// This function is safe for concurrent access.
func (p *Peer) PushGetBlocksMsg(locator []*common.Uint256, stopHash *common.Uint256) error {
	// Extract the begin hash from the block locator, if one was specified,
	// to use for filtering duplicate getblocks requests.
	var beginHash *common.Uint256
	if len(locator) > 0 {
		beginHash = locator[0]
	}

	// Filter duplicate getblocks requests.
	p.prevGetBlocksMtx.Lock()
	isDuplicate := p.prevGetBlocksStop != nil && p.prevGetBlocksBegin != nil &&
		beginHash != nil && stopHash.IsEqual(*p.prevGetBlocksStop) &&
		beginHash.IsEqual(*p.prevGetBlocksBegin)
	p.prevGetBlocksMtx.Unlock()

	if isDuplicate {
		log.Debugf("Filtering duplicate [getblocks] with begin "+
			"hash %v, stop hash %v", beginHash, stopHash)
		return nil
	}

	// Construct the getblocks request and queue it to be sent.
	msg := msg.NewGetBlocks(locator, *stopHash)
	p.QueueMessage(msg, nil)

	// Update the previous getblocks request information for filtering
	// duplicates.
	p.prevGetBlocksMtx.Lock()
	p.prevGetBlocksBegin = beginHash
	p.prevGetBlocksStop = stopHash
	p.prevGetBlocksMtx.Unlock()
	return nil
}

// PushRejectMsg sends a reject message for the provided command, reject code,
// reject reason, and hash.  The hash will only be used when the command is a tx
// or block and should be nil in other cases.  The wait parameter will cause the
// function to block until the reject message has actually been sent.
//
// This function is safe for concurrent access.
func (p *Peer) PushRejectMsg(command string, code msg.RejectCode, reason string, hash *common.Uint256, wait bool) {
	msg := msg.NewReject(command, code, reason)
	if command == p2p.CmdTx || command == p2p.CmdBlock {
		if hash == nil {
			log.Warnf("Sending a reject message for command "+
				"type %v which should have specified a hash "+
				"but does not", command)
			hash = &zeroHash
		}
		msg.Hash = *hash
	}

	// Send the message without waiting if the caller has not requested it.
	if !wait {
		p.QueueMessage(msg, nil)
		return
	}

	// Send the message and block until it has been sent before returning.
	doneChan := make(chan struct{}, 1)
	p.QueueMessage(msg, doneChan)
	<-doneChan
}

// maybeAddDeadline potentially adds a deadline for the appropriate expected
// response for the passed wire protocol command to the pending responses map.
func (p *Peer) maybeAddDeadline(pendingResponses map[string]time.Time, msgCmd string) {
	// Setup a deadline for each message being sent that expects a response.
	//
	// NOTE: Pings are intentionally ignored here since they are typically
	// sent asynchronously and as a result of a long backlock of messages,
	// such as is typical in the case of initial block download, the
	// response won't be received in time.
	deadline := time.Now().Add(stallResponseTimeout)
	switch msgCmd {
	case p2p.CmdVersion:
		// Expects a verack message.
		pendingResponses[p2p.CmdVerAck] = deadline

	case p2p.CmdMemPool:
		// Expects an inv message.
		pendingResponses[p2p.CmdInv] = deadline

	case p2p.CmdGetBlocks:
		// Expects an inv message.
		pendingResponses[p2p.CmdInv] = deadline

	case p2p.CmdGetData:
		// Expects a block, merkleblock, tx, or notfound message.
		pendingResponses[p2p.CmdBlock] = deadline
		pendingResponses[p2p.CmdMerkleBlock] = deadline
		pendingResponses[p2p.CmdTx] = deadline
		pendingResponses[p2p.CmdNotFound] = deadline
	}
}

// stallHandler handles stall detection for the peer.  This entails keeping
// track of expected responses and assigning them deadlines while accounting for
// the time spent in callbacks.  It must be run as a goroutine.
func (p *Peer) stallHandler() {
	// These variables are used to adjust the deadline times forward by the
	// time it takes callbacks to execute.  This is done because new
	// messages aren't read until the previous one is finished processing
	// (which includes callbacks), so the deadline for receiving a response
	// for a given message must account for the processing time as well.
	var handlerActive bool
	var handlersStartTime time.Time
	var deadlineOffset time.Duration

	// pendingResponses tracks the expected response deadline times.
	pendingResponses := make(map[string]time.Time)

	// stallTicker is used to periodically check pending responses that have
	// exceeded the expected deadline and disconnect the peer due to
	// stalling.
	stallTicker := time.NewTicker(stallTickInterval)
	defer stallTicker.Stop()

	// ioStopped is used to detect when both the input and output handler
	// goroutines are done.
	var ioStopped bool
out:
	for {
		select {
		case msg := <-p.stallControl:
			switch msg.command {
			case sccSendMessage:
				// Add a deadline for the expected response
				// message if needed.
				p.maybeAddDeadline(pendingResponses,
					msg.message.CMD())

			case sccReceiveMessage:
				// Remove received messages from the expected
				// response map.  Since certain commands expect
				// one of a group of responses, remove
				// everything in the expected group accordingly.
				switch msgCmd := msg.message.CMD(); msgCmd {
				case p2p.CmdBlock:
					fallthrough
				case p2p.CmdMerkleBlock:
					fallthrough
				case p2p.CmdTx:
					fallthrough
				case p2p.CmdNotFound:
					delete(pendingResponses, p2p.CmdBlock)
					delete(pendingResponses, p2p.CmdMerkleBlock)
					delete(pendingResponses, p2p.CmdTx)
					delete(pendingResponses, p2p.CmdNotFound)

				default:
					delete(pendingResponses, msgCmd)
				}

			case sccHandlerStart:
				// Warn on unbalanced callback signalling.
				if handlerActive {
					log.Warn("Received handler start " +
						"control command while a " +
						"handler is already active")
					continue
				}

				handlerActive = true
				handlersStartTime = time.Now()

			case sccHandlerDone:
				// Warn on unbalanced callback signalling.
				if !handlerActive {
					log.Warn("Received handler done " +
						"control command when a " +
						"handler is not already active")
					continue
				}

				// Extend active deadlines by the time it took
				// to execute the callback.
				duration := time.Since(handlersStartTime)
				deadlineOffset += duration
				handlerActive = false

			default:
				log.Warnf("Unsupported message command %v",
					msg.command)
			}

		case <-stallTicker.C:
			// Calculate the offset to apply to the deadline based
			// on how long the handlers have taken to execute since
			// the last tick.
			now := time.Now()
			offset := deadlineOffset
			if handlerActive {
				offset += now.Sub(handlersStartTime)
			}

			// Disconnect the peer if any of the pending responses
			// don't arrive by their adjusted deadline.
			for command, deadline := range pendingResponses {
				if now.Before(deadline.Add(offset)) {
					continue
				}

				log.Debugf("Peer %s appears to be stalled or "+
					"misbehaving, %s timeout -- "+
					"disconnecting", p, command)
				p.Disconnect()
				break
			}

			// Reset the deadline offset for the next tick.
			deadlineOffset = 0

		case <-p.InQuit():
			// The stall handler can exit once both the input and
			// output handler goroutines are done.
			if ioStopped {
				break out
			}
			ioStopped = true

		case <-p.OutQuit():
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

func (p *Peer) sendMessage(msg outMsg) {
	p.stallControl <- stallControlMsg{sccSendMessage, msg.msg}
	p.SendMessage(msg.msg, msg.doneChan)
}

// queueHandler handles the queuing of outgoing data for the peer. This runs as
// a muxer for various sources of input so we can ensure that server and peer
// handlers will not block on us sending a message.  That data is then passed on
// to outHandler to be actually written.
func (p *Peer) queueHandler() {
	pendingMsgs := list.New()
	invSendQueue := list.New()
	trickleTicker := time.NewTicker(trickleInterval)
	defer trickleTicker.Stop()

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
			p.sendMessage(msg)
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
			p.sendMessage(val.(outMsg))

		case iv := <-p.outputInvChan:
			// No handshake?  They'll find out soon enough.
			if p.VersionKnown() {
				// If this is a new block, then we'll blast it
				// out immediately, sipping the inv trickle
				// queue.
				if iv.Type == msg.InvTypeBlock || iv.Type == msg.InvTypeConfirmedBlock {
					invMsg := msg.NewInvSize(1)
					invMsg.AddInvVect(iv)
					waiting = queuePacket(outMsg{msg: invMsg},
						pendingMsgs, waiting)
				} else {
					invSendQueue.PushBack(iv)
				}
			}

		case <-trickleTicker.C:
			// Don't send anything if we're disconnecting or there
			// is no queued inventory.
			// version is known if send queue has any entries.
			if !p.Connected() || invSendQueue.Len() == 0 {
				continue
			}

			// Create and send as many inv messages as needed to
			// drain the inventory send queue.
			invMsg := msg.NewInvSize(uint(invSendQueue.Len()))
			for e := invSendQueue.Front(); e != nil; e = invSendQueue.Front() {
				iv := invSendQueue.Remove(e).(*msg.InvVect)

				// Don't send inventory that became known after
				// the initial check.
				if p.knownInventory.Exists(iv) {
					continue
				}

				invMsg.AddInvVect(iv)
				if len(invMsg.InvList) >= maxInvTrickleSize {
					waiting = queuePacket(
						outMsg{msg: invMsg},
						pendingMsgs, waiting)
					invMsg = msg.NewInvSize(uint(invSendQueue.Len()))
				}

				// Add the inventory that is being relayed to
				// the known inventory for the peer.
				p.AddKnownInventory(iv)
			}
			if len(invMsg.InvList) > 0 {
				waiting = queuePacket(outMsg{msg: invMsg},
					pendingMsgs, waiting)
			}

		case <-p.Quit():
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
		case <-p.outputInvChan:
			// Just drain channel
			// sendDoneQueue is buffered so doesn't need draining.
		default:
			break cleanup
		}
	}
}

// QueueMessage adds the passed message to the peer send queue.
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

// QueueInventory adds the passed inventory to the inventory send queue which
// might not be sent right away, rather it is trickled to the peer in batches.
// Inventory that the peer is already known to have is ignored.
//
// This function is safe for concurrent access.
func (p *Peer) QueueInventory(invVect *msg.InvVect) {
	// Don't add the inventory to the send queue if the peer is already
	// known to have it.
	if p.knownInventory.Exists(invVect) {
		return
	}

	// Avoid risk of deadlock if goroutine already exited.  The goroutine
	// we will be sending to hangs around until it knows for a fact that
	// it is marked as disconnected and *then* it drains the channels.
	if !p.Connected() {
		return
	}

	p.outputInvChan <- invVect
}

// start begins processing input and output messages.
func (p *Peer) start() *Peer {
	// The protocol has been negotiated successfully so start processing input
	// and output messages.
	go p.stallHandler()
	go p.queueHandler()

	return p
}

func New(orgPeer server.IPeer, listeners *Listeners) *Peer {
	p := &Peer{
		Peer:           orgPeer.ToPeer(),
		IPeer:          orgPeer,
		knownInventory: newMruInventoryMap(maxKnownInventory),
		stallControl:   make(chan stallControlMsg, 1), // nonblocking sync
		outputQueue:    make(chan outMsg, outputBufferSize),
		sendDoneQueue:  make(chan struct{}, 1),
		outputInvChan:  make(chan *msg.InvVect, outputBufferSize),
	}

	p.OnSendDone(p.sendDoneQueue)
	p.AddMessageFunc(func(_ *peer.Peer, m p2p.Message) {
		p.stallControl <- stallControlMsg{sccReceiveMessage, m}

		// Handle each supported message type.
		p.stallControl <- stallControlMsg{sccHandlerStart, m}
		switch m := m.(type) {
		case *msg.Version:
			// Disconnect full node peers that do not support DPOS protocol.
			if m.Relay && p.ProtocolVersion() < pact.DPOSStartVersion {
				p.Disconnect()
				return
			}

			p.SetDisableRelayTx(!m.Relay)

		case *msg.MemPool:
			listeners.OnMemPool(p, m)

		case *msg.Tx:
			listeners.OnTx(p, m)

		case *msg.Block:
			listeners.OnBlock(p, m)

		case *msg.Inv:
			listeners.OnInv(p, m)

		case *msg.NotFound:
			listeners.OnNotFound(p, m)

		case *msg.GetData:
			listeners.OnGetData(p, m)

		case *msg.GetBlocks:
			listeners.OnGetBlocks(p, m)

		case *msg.FilterAdd:
			listeners.OnFilterAdd(p, m)

		case *msg.FilterClear:
			listeners.OnFilterClear(p, m)

		case *msg.FilterLoad:
			listeners.OnFilterLoad(p, m)

		case *msg.Reject:
			listeners.OnReject(p, m)

		case *msg.VerAck, *msg.GetAddr, *msg.Addr, *msg.Ping, *msg.Pong:
		//	Basic messages have been handled, ignore them.

		default:
			log.Debugf("Received unhandled message of type %v "+
				"from %v", m.CMD(), p)
		}
		p.stallControl <- stallControlMsg{sccHandlerDone, m}
	})

	return p.start()
}
