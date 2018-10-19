package peer

import (
	"container/list"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/bloom"
	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA.Utility/p2p/peer"
)

const (
	// stallTickInterval is the interval of time between each check for
	// stalled peers.
	stallTickInterval = 5 * time.Second

	// stallResponseTimeout is the base maximum amount of time messages that
	// expect a response will wait before disconnecting the peer for
	// stalling.  The deadlines are adjusted for callback running times and
	// only checked on each stall tick interval.
	stallResponseTimeout = 15 * time.Second

	// outputBufferSize is the number of elements the output channels use.
	outputBufferSize = 50
)

// Config is the struct to hold configuration options useful to Peer.
type Config struct {
	// After send a blocks request message, this inventory message
	// will return with a bunch of block hashes, then you can use them
	// to request all the blocks by send data requests.
	OnInv func(*Peer, *msg.Inv)

	// This method will be invoked when a merkleblock and transactions
	// within it has been downloaded.
	OnBlock func(*Peer, *util.Block)

	// After sent a data request with invType TRANSACTION, a txn message will return through this method.
	// these transactions are matched to the bloom filter you have sent with the filterload message.
	OnTx func(*Peer, util.Transaction)

	// If the BLOCK or TRANSACTION requested by the data request message can not be found,
	// notfound message with requested data hash will return through this method.
	OnNotFound func(*Peer, *msg.NotFound)

	// If the submitted transaction was rejected, this message will return.
	OnReject func(*Peer, *msg.Reject)
}

// outMsg is used to house a message to be sent along with a channel to signal
// when the message has been sent (or won't be sent due to things such as
// shutdown)
type outMsg struct {
	msg      p2p.Message
	doneChan chan<- struct{}
}

// stallClearMsg is used to clear current stalled messages.  This is useful when
// we are synced to current but a getblocks message is stalled, we need to cancel
// it.
type stallClearMsg struct{}

type Peer struct {
	*peer.Peer
	cfg Config

	prevGetBlocksMtx   sync.Mutex
	prevGetBlocksBegin *common.Uint256
	prevGetBlocksStop  *common.Uint256

	stallControl  chan interface{}
	blockQueue    chan interface{}
	outputQueue   chan outMsg
	sendDoneQueue chan struct{}
}

func NewPeer(peer *peer.Peer, cfg *Config) *Peer {
	p := Peer{
		Peer:          peer,
		cfg:           *cfg,
		stallControl:  make(chan interface{}, 1),
		blockQueue:    make(chan interface{}, 1),
		sendDoneQueue: make(chan struct{}, 1),
		outputQueue:   make(chan outMsg, outputBufferSize),
	}
	peer.AddMessageFunc(p.handleMessage)
	peer.OnSendDone(p.sendDoneQueue)

	go p.stallHandler()
	go p.queueHandler()
	go p.blockHandler()

	return &p
}

func (p *Peer) sendMessage(out outMsg) {
	switch out.msg.(type) {
	case *msg.GetBlocks, *msg.GetData:
		p.stallControl <- out.msg
	}
	p.SendMessage(out.msg, out.doneChan)
}

func (p *Peer) handleMessage(peer *peer.Peer, message p2p.Message) {
	// Return if peer already disconnected.
	if !p.Connected() {
		return
	}

	switch m := message.(type) {
	case *msg.Inv:
		p.stallControl <- message
		p.cfg.OnInv(p, m)

	case *msg.MerkleBlock:
		p.stallControl <- message
		p.blockQueue <- m

	case *msg.Tx:
		p.stallControl <- message
		p.blockQueue <- m

	case *msg.NotFound:
		p.stallControl <- message
		p.cfg.OnNotFound(p, m)

	case *msg.Reject:
		p.stallControl <- message
		p.cfg.OnReject(p, m)
	}
}

func (p *Peer) stallHandler() {
	// lastActive tracks the last active sync message.
	var lastActive time.Time

	// pendingResponses tracks the expected responses.
	pendingResponses := make(map[string]struct{})

	// stallTicker is used to periodically check pending responses that have
	// exceeded the expected deadline and disconnect the peer due to stalling.
	stallTicker := time.NewTicker(stallTickInterval)
	defer stallTicker.Stop()

	// ioStopped is used to detect when both the input and output handler
	// goroutines are done.
	var ioStopped bool
out:
	for {
		select {
		case ctrMsg := <-p.stallControl:
			// update last active time
			lastActive = time.Now()

			switch m := ctrMsg.(type) {
			case *msg.GetBlocks:
				// Add expected response
				pendingResponses[p2p.CmdInv] = struct{}{}

			case *msg.Inv:
				// Remove inventory from expected response map.
				delete(pendingResponses, p2p.CmdInv)

			case *msg.GetData:
				// Add expected responses
				for _, iv := range m.InvList {
					pendingResponses[iv.Hash.String()] = struct{}{}
				}

			case *msg.MerkleBlock:
				// Remove received merkleblock from expected response map.
				delete(pendingResponses, m.Header.(util.BlockHeader).Hash().String())

			case *msg.Tx:
				// Remove received transaction from expected response map.
				delete(pendingResponses, m.Serializable.(util.Transaction).Hash().String())

			case *msg.NotFound:
				// NotFound should not received from sync peer
				p.Disconnect()

			case stallClearMsg:
				// Clear pending responses.
				pendingResponses = make(map[string]struct{})
			}

		case <-stallTicker.C:
			// There are no pending responses
			if len(pendingResponses) == 0 {
				continue
			}

			// Disconnect the peer if any of the pending responses
			// don't arrive by their adjusted deadline.
			if time.Now().Before(lastActive.Add(stallResponseTimeout)) {
				continue
			}

			log.Debugf("peer %v appears to be stalled or misbehaving, response timeout -- disconnecting", p)
			p.Disconnect()

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
	log.Tracef("Peer stall handler done for %v", p)
}

// blockHandler handles the downloading of merkleblock and the transactions
// within it.  We handle a merkleblock and it's transactions as one block.
// We do not notify this new block until the downloading has been completed.
func (p *Peer) blockHandler() {
	// Data caches for the downloading block.
	var header *util.Header
	var pendingTxs map[common.Uint256]struct{}
	var txs []util.Transaction

	// NotifyOnBlock message and clear cached data.
	notifyBlock := func() {
		// Return if peer already disconnected.
		if !p.Connected() {
			return
		}

		// Notify OnBlock.
		p.cfg.OnBlock(p, &util.Block{
			Header:       *header,
			Transactions: txs,
		})

		// Clear cached data.
		header = nil
		pendingTxs = nil
		txs = nil
	}

out:
	for {
		select {
		case bmsg := <-p.blockQueue:
			switch m := bmsg.(type) {
			case *msg.MerkleBlock:
				// If header is not nil, the previous block download was not
				// finished, that means the peer is misbehaving, disconnect it.
				if header != nil {
					log.Debugf("peer %v send us new block before previous"+
						" block download finished -- disconnecting", p)
					p.Disconnect()
					continue
				}

				// Check block
				txIds, err := bloom.CheckMerkleBlock(*m)
				if err != nil {
					log.Debugf("peer %v send us invalid merkleblock -- disconnecting", p)
					p.Disconnect()
					continue
				}

				// Set current downloading block
				header = &util.Header{
					BlockHeader: m.Header.(util.BlockHeader),
					NumTxs:      m.Transactions,
					Hashes:      m.Hashes,
					Flags:       m.Flags,
				}

				// No transaction included in this block, so just notify block
				// downloading completed.
				if len(txIds) == 0 {
					notifyBlock()
					continue
				}

				// Save pending transactions to cache.
				pendingTxs = make(map[common.Uint256]struct{})
				for _, txId := range txIds {
					pendingTxs[*txId] = struct{}{}
				}

				// Initiate transactions cache.
				txs = make([]util.Transaction, 0, len(pendingTxs))

			case *msg.Tx:
				// Not in block downloading mode, just notify new transaction.
				tx := m.Serializable.(util.Transaction)
				if header == nil {
					p.cfg.OnTx(p, tx)
					continue
				}

				txId := tx.Hash()

				// When downloading block, received transactions can only by
				// those within the block.
				if _, ok := pendingTxs[txId]; !ok {
					log.Debugf("peer %v send us invalid transaction -- disconnecting", p)
					p.Disconnect()
					continue
				}

				// Save downloaded transaction to cache.
				txs = append(txs, tx)

				// Remove transaction from pending list.
				delete(pendingTxs, txId)

				// Block download completed, notify OnBlock.
				if len(pendingTxs) == 0 {
					notifyBlock()
				}
			}

		case <-p.Quit():
			break out
		}
	}

	// Drain any wait channels before going away so there is nothing left
	// waiting on this goroutine.
cleanup:
	for {
		select {
		case <-p.blockQueue:
		default:
			break cleanup
		}
	}
	log.Tracef("Peer block handler done for %v", p)
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
		default:
			break cleanup
		}
	}
	log.Tracef("Peer queue handler done for %s", p)
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
		log.Tracef("Filtering duplicate [getblocks] with begin "+
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

func (p *Peer) StallClear() {
	p.stallControl <- stallClearMsg{}
}

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
