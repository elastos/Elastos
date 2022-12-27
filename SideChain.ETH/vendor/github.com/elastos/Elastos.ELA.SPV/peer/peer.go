package peer

import (
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/bloom"
	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/p2p"
	"github.com/elastos/Elastos.ELA/p2p/msg"
	"github.com/elastos/Elastos.ELA/p2p/peer"
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
)

// Config is the struct to hold configuration options useful to Peer.
type Config struct {
	// OnVersion is invoked when a peer receives a version message.
	OnVersion func(*Peer, *msg.Version)

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

	stallControl chan interface{}
	blockQueue   chan interface{}
	quit         chan struct{}
}

func (p *Peer) handleMessage(peer *peer.Peer, message p2p.Message) {
	// Return if peer already disconnected.
	if !p.Connected() {
		return
	}

	switch m := message.(type) {
	case *msg.Version:
		p.cfg.OnVersion(p, m)

	case *msg.Inv:
		p.cfg.OnInv(p, m)

	case *msg.MerkleBlock:
		p.blockQueue <- m

	case *msg.Tx:
		p.blockQueue <- m

	case *msg.NotFound:
		p.cfg.OnNotFound(p, m)

	case *msg.Reject:
		p.cfg.OnReject(p, m)
	}
}

// stallHandler handles stall detection for the peer.  This entails keeping
// track of expected responses and assigning them deadlines while accounting for
// the time spent in callbacks.  It must be run as a goroutine.
func (p *Peer) stallHandler() {
	// lastActive tracks the last active sync message.
	var lastActive time.Time

	// pendingResponses tracks the expected responses.
	pendingResponses := make(map[string]struct{})

	// stallTicker is used to periodically check pending responses that have
	// exceeded the expected deadline and disconnect the peer due to stalling.
	stallTicker := time.NewTicker(stallTickInterval)
	defer stallTicker.Stop()

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
				// Remove received transaction from expected response map.
				for _, iv := range m.InvList {
					delete(pendingResponses, iv.Hash.String())
				}

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

			log.Debugf("peer %v appears to be stalled or misbehaving,"+
				" response timeout -- disconnecting", p)
			p.Disconnect()

		case <-p.quit:
			break out
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
					log.Debugf("peer %v send us new block before"+
						" previous block download finished -- disconnecting", p)
					p.Disconnect()
					continue
				}

				// Check block
				txIds, err := bloom.CheckMerkleBlock(*m)
				if err != nil {
					log.Debugf("peer %v send us invalid merkleblock"+
						" -- disconnecting", p)
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

		case <-p.quit:
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

func (p *Peer) StallClear() {
	p.stallControl <- stallClearMsg{}
}

func NewPeer(orgPeer *peer.Peer, cfg *Config) *Peer {
	p := Peer{
		Peer:         orgPeer,
		cfg:          *cfg,
		stallControl: make(chan interface{}, 1),
		blockQueue:   make(chan interface{}, 1),
		quit:         make(chan struct{}),
	}

	// Append message handler to the peer.
	p.AddMessageFunc(p.handleMessage)

	// Set stall handler to the peer.
	p.SetStallHandler(func(msg peer.StallControlMsg) {
		p.stallControl <- msg.MSG
	})

	go p.stallHandler()
	go p.blockHandler()

	// Wait for supper class peer quit.
	go func() {
		p.WaitForDisconnect()
		close(p.quit)
	}()
	return &p
}
