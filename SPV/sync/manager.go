package sync

import (
	"sync/atomic"

	"github.com/elastos/Elastos.ELA.SPV/blockchain"
	"github.com/elastos/Elastos.ELA.SPV/fprate"
	"github.com/elastos/Elastos.ELA.SPV/peer"
	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/p2p/msg"
)

const (
	// minPendingRequests is the minimum number of block hashes in request
	// queue.
	minPendingRequests = msg.MaxInvPerMsg

	// minInFlightBlocks is the minimum number of blocks that should be
	// in the request queue before requesting more.
	minInFlightBlocks = 10

	// maxBadBlockRate is the maximum bad blocks rate of received blocks.
	maxBadBlockRate float64 = 0.001

	// maxRequestedBlocks is the maximum number of requested block
	// hashes to store in memory.
	maxRequestedBlocks = msg.MaxInvPerMsg

	// maxRequestedTxns is the maximum number of requested transactions
	// hashes to store in memory.
	maxRequestedTxns = msg.MaxInvPerMsg
)

// zeroHash is the zero value hash (all zeros).  It is defined as a convenience.
var zeroHash common.Uint256

// newPeerMsg signifies a newly connected peer to the block handler.
type newPeerMsg struct {
	peer *peer.Peer
}

// donePeerMsg signifies a newly disconnected peer to the block handler.
type donePeerMsg struct {
	peer *peer.Peer
}

// invMsg packages a bitcoin inv message and the peer it came from together
// so the block handler has access to that information.
type invMsg struct {
	inv  *msg.Inv
	peer *peer.Peer
}

// blockMsg packages a block message and the peer it came from
// together so the block handler has access to that information.
type blockMsg struct {
	block *util.Block
	peer  *peer.Peer
	reply chan struct{}
}

// txMsg packages a bitcoin tx message and the peer it came from together
// so the block handler has access to that information.
type txMsg struct {
	tx    util.Transaction
	peer  *peer.Peer
	reply chan struct{}
}

// getSyncPeerMsg is a message type to be sent across the message channel for
// retrieving the current sync peer.
type getSyncPeerMsg struct {
	reply chan uint64
}

// isCurrentMsg is a message type to be sent across the message channel for
// requesting whether or not the sync manager believes it is synced with the
// currently connected peers.
type isCurrentMsg struct {
	reply chan bool
}

// pauseMsg is a message type to be sent across the message channel for
// pausing the sync manager.  This effectively provides the caller with
// exclusive access over the manager until a receive is performed on the
// unpause channel.
type pauseMsg struct {
	unpause <-chan struct{}
}

// peerSyncState stores additional information that the SyncManager tracks
// about a peer.
type peerSyncState struct {
	syncCandidate   bool
	requestQueue    []*msg.InvVect
	requestedTxns   map[common.Uint256]struct{}
	requestedBlocks map[common.Uint256]struct{}
	receivedBlocks  uint32
	badBlocks       uint32
	fpRate          *fprate.FpRate
}

func (s *peerSyncState) badBlockRate() float64 {
	return float64(s.badBlocks) / float64(s.receivedBlocks)
}

// SyncManager is used to communicate block related messages with peers. The
// SyncManager is started as by executing Start() in a goroutine. Once started,
// it selects peers to sync from and starts the initial block download. Once the
// chain is in sync, the SyncManager handles incoming block and header
// notifications and relays announcements of new blocks to peers.
type SyncManager struct {
	started  int32
	shutdown int32
	cfg      Config
	msgChan  chan interface{}
	quit     chan struct{}

	// These fields should only be accessed from the blockHandler thread
	requestedTxns   map[common.Uint256]struct{}
	requestedBlocks map[common.Uint256]struct{}
	txMemPool       map[common.Uint256]struct{}
	syncPeer        *peer.Peer
	peerStates      map[*peer.Peer]*peerSyncState
}

// current returns true if we believe we are synced with our peers, false if we
// still have blocks to check
func (sm *SyncManager) current() bool {
	// if blockChain thinks we are current and we have no syncPeer it
	// is probably right.
	if sm.syncPeer == nil {
		return true
	}

	// No matter what chain thinks, if we are below the block we are syncing
	// to we are not current.
	if sm.cfg.Chain.BestHeight() < sm.syncPeer.Height() {
		return false
	}
	return true
}

// startSync will choose the best peer among the available candidate peers to
// download/sync the blockchain from.  When syncing is already running, it
// simply returns.  It also examines the candidates for any which are no longer
// candidates and removes them as needed.
func (sm *SyncManager) startSync() {
	// Return now if we're already syncing.
	if sm.syncPeer != nil {
		return
	}

	bestHeight := sm.cfg.Chain.BestHeight()
	var bestPeer *peer.Peer
	for peer, state := range sm.peerStates {
		if !state.syncCandidate {
			continue
		}

		// Remove sync candidate peers that are no longer candidates due
		// to passing their latest known block.  NOTE: The < is
		// intentional as opposed to <=.  While technically the peer
		// doesn't have a later block when it's equal, it will likely
		// have one soon so it is a reasonable choice.  It also allows
		// the case where both are at 0 such as during regression test.
		if peer.Height() < bestHeight {
			state.syncCandidate = false
			continue
		}

		// Just pick the first available candidate.
		bestPeer = peer
		break
	}

	// Start syncing from the best peer if one was selected.
	if bestPeer != nil {
		// Do not start syncing if we have the same height with best peer.
		if bestPeer.Height() == bestHeight {
			return
		}

		sm.syncWith(bestPeer)
	} else {
		log.Warnf("No sync peer candidates available")
	}
}

func (sm *SyncManager) syncWith(peer *peer.Peer) {
	// Clear the requestedBlocks if the sync peer changes, otherwise we
	// may ignore blocks we need that the last sync peer failed to send.
	sm.requestedBlocks = make(map[common.Uint256]struct{})

	log.Infof("Syncing to block height %d from peer %v", peer.Height(),
		peer.Addr())

	locator := sm.cfg.Chain.LatestBlockLocator()
	peer.PushGetBlocksMsg(locator, &zeroHash)
	sm.syncPeer = peer
}

// isSyncCandidate returns whether or not the peer is a candidate to consider
// syncing from.
func (sm *SyncManager) isSyncCandidate(peer *peer.Peer) bool {
	services := peer.Services()
	for _, flag := range sm.cfg.CandidateFlags {
		if services&flag != flag {
			return false
		}
	}
	// Candidate if all checks passed.
	return true
}

// getSyncCandidates returns the peers that are sync candidate.
func (sm *SyncManager) getSyncCandidates() []*peer.Peer {
	candidates := make([]*peer.Peer, 0, len(sm.peerStates))
	for peer, state := range sm.peerStates {
		candidate := *peer
		if state.syncCandidate {
			candidates = append(candidates, &candidate)
		}
	}
	return candidates
}

// pushBloomFilter update and send the bloom filter to the given peer.
func (sm *SyncManager) pushBloomFilter(p *peer.Peer) {
	p.QueueMessage(sm.cfg.GetTxFilter(), nil)
}

// handleNewPeerMsg deals with new peers that have signalled they may
// be considered as a sync peer (they have already successfully negotiated).  It
// also starts syncing if needed.  It is invoked from the syncHandler goroutine.
func (sm *SyncManager) handleNewPeerMsg(peer *peer.Peer) {
	// Ignore if in the process of shutting down.
	if atomic.LoadInt32(&sm.shutdown) != 0 {
		return
	}

	log.Infof("New valid peer %s", peer)

	// Initialize the peer state
	isSyncCandidate := sm.isSyncCandidate(peer)
	sm.peerStates[peer] = &peerSyncState{
		syncCandidate:   isSyncCandidate,
		requestedTxns:   make(map[common.Uint256]struct{}),
		requestedBlocks: make(map[common.Uint256]struct{}),
		fpRate:          fprate.NewFpRate(),
	}

	if isSyncCandidate {
		// Update bloom filter for the candidate peer.
		sm.pushBloomFilter(peer)

		// Start syncing by choosing the best candidate if needed.
		if sm.syncPeer == nil {
			sm.startSync()
		}
	}
}

// handleDonePeerMsg deals with peers that have signalled they are done.  It
// removes the peer as a candidate for syncing and in the case where it was
// the current sync peer, attempts to select a new best peer to sync from.  It
// is invoked from the syncHandler goroutine.
func (sm *SyncManager) handleDonePeerMsg(peer *peer.Peer) {
	state, exists := sm.peerStates[peer]
	if !exists {
		log.Warnf("Received done peer message for unknown peer %s", peer)
		return
	}

	// Remove the peer from the list of candidate peers.
	delete(sm.peerStates, peer)

	log.Infof("Lost peer %s", peer)

	// Remove requested transactions from the global map so that they will
	// be fetched from elsewhere next time we get an inv.
	for txHash := range state.requestedTxns {
		delete(sm.requestedTxns, txHash)
	}

	// Remove requested blocks from the global map so that they will be
	// fetched from elsewhere next time we get an inv.
	for blockHash := range state.requestedBlocks {
		delete(sm.requestedBlocks, blockHash)
	}

	// Attempt to find a new peer to sync from if the quitting peer is the
	// sync peer.
	if sm.syncPeer == peer {
		sm.syncPeer = nil
		sm.startSync()
	}
}

// handleTxMsg handles transaction messages from all peers.
func (sm *SyncManager) handleTxMsg(tmsg *txMsg) {
	peer := tmsg.peer
	state, exists := sm.peerStates[peer]
	if !exists {
		log.Warnf("Received tx message from unknown peer %s", peer)
		return
	}

	txHash := tmsg.tx.Hash()

	_, ok := state.requestedTxns[txHash]
	if !ok {
		log.Warnf("Peer %s is sending us transactions we didn't request", peer)
		peer.Disconnect()
		return
	}
	sm.txMemPool[txHash] = struct{}{}

	// Remove transaction from request maps. Either the mempool/chain
	// already knows about it and as such we shouldn't have any more
	// instances of trying to fetch it, or we failed to insert and thus
	// we'll retry next time we get an inv.
	delete(state.requestedTxns, txHash)
	delete(sm.requestedTxns, txHash)

	if sm.cfg.TransactionAnnounce != nil {
		sm.cfg.TransactionAnnounce(tmsg.tx)
	}
}

// handleBlockMsg handles block messages from all peers.  Blocks are requested
// in response to inv packets both during initial sync and after.
func (sm *SyncManager) handleBlockMsg(bmsg *blockMsg) {
	peer := bmsg.peer

	// We don't need to process blocks when we're syncing. They wont connect anyway
	if peer != sm.syncPeer && !sm.current() {
		log.Warnf("Received block from %s when we aren't current", peer)
		return
	}
	state, exists := sm.peerStates[peer]
	if !exists {
		log.Warnf("Received block message from unknown peer %s", peer)
		peer.Disconnect()
		return
	}

	// If we didn't ask for this block then the peer is misbehaving.
	block := bmsg.block
	blockHash := block.Hash()
	if _, exists = state.requestedBlocks[blockHash]; !exists {
		log.Warnf("Received unrequested block from peer %s", peer)
		peer.Disconnect()
		return
	}

	// Remove block from request maps. Either chain will know about it and
	// so we shouldn't have any more instances of trying to fetch it, or we
	// will fail the insert and thus we'll retry next time we get an inv.
	state.receivedBlocks++
	delete(state.requestedBlocks, blockHash)
	delete(sm.requestedBlocks, blockHash)

	newBlock, reorg, newHeight, fps, err := sm.cfg.Chain.CommitBlock(block)
	// If this is an orphan block which doesn't connect to the chain, it's possible
	// that we might be synced on the longest chain, but not the most-work chain like
	// we should be. To make sure this isn't the case, let's sync from the peer who
	// sent us this orphan block.
	if err == blockchain.OrphanBlockError && sm.current() {
		log.Debug("Received orphan header, checking peer for more blocks")
		state.requestQueue = []*msg.InvVect{}
		state.requestedBlocks = make(map[common.Uint256]struct{})
		sm.requestedBlocks = make(map[common.Uint256]struct{})
		sm.syncWith(peer)
		return
	}

	// The sync peer sent us an orphan header in the middle of a sync. This could
	// just be the last block in the batch which represents the tip of the chain.
	// In either case let's adjust the score for this peer downwards. If it goes
	// negative it means he's slamming us with blocks that don't fit in our chain
	// so disconnect.
	if err == blockchain.OrphanBlockError && !sm.current() {
		state.badBlocks++
		if state.badBlockRate() > maxBadBlockRate {
			log.Warnf("Disconnecting from peer %s because he sent us too many bad blocks", peer)
			peer.Disconnect()
		}
		return
	}

	// Log other error message and return.
	if err != nil {
		log.Error(err)
		return
	}

	// We can exit here if the block is already known
	if !newBlock {
		log.Debugf("Received duplicate block %s", blockHash.String())
		return
	}

	// Check false positive rate.
	fpRate := state.fpRate.Update(block, fps)
	if fpRate > fprate.DefaultFalsePositiveRate*10 {
		log.Warnf("bloom filter false positive rate %f too high,"+
			" disconnecting...", fpRate)
		peer.Disconnect()
		return
	}
	if newHeight+500 < peer.Height() && fpRate > fprate.DefaultFalsePositiveRate {
		sm.pushBloomFilter(peer)
		state.fpRate.Reset()
	}

	log.Infof("Received block %s at height %d", blockHash.String(), newHeight)

	// Check reorg
	if reorg && sm.current() {
		// Clear request state for new sync
		state.requestQueue = []*msg.InvVect{}
		state.requestedBlocks = make(map[common.Uint256]struct{})
		sm.requestedBlocks = make(map[common.Uint256]struct{})
	}

	// Clear mempool
	sm.txMemPool = make(map[common.Uint256]struct{})

	// If we're current now, nothing more to do.
	if sm.current() {
		// When we are current, the last getblocks message we sent will get
		// stalled, so we cancel it to prevent peer from stall disconnection.
		peer.StallClear()
		peer.UpdateHeight(newHeight)
		return
	}

	// Request more blocks if in flight blocks is getting short. This can make
	// syncing progress a little bit faster then request more blocks after the
	// last requested block received.
	if len(state.requestedBlocks) < minInFlightBlocks {
		sm.requestQueuedInv(peer, state)
	}
}

// haveInventory returns whether or not the inventory represented by the passed
// inventory vector is known.  This includes checking all of the various places
// inventory can be when it is in different states such as blocks that are part
// of the main chain, on a side chain, in the orphan pool, and transactions that
// are in the memory pool (either the main pool or orphan pool).
func (sm *SyncManager) haveInventory(invVect *msg.InvVect) bool {
	switch invVect.Type {
	case msg.InvTypeBlock:
		// Ask chain if the block is known to it in any form (main
		// chain, side chain, or orphan).
		return sm.cfg.Chain.HaveBlock(&invVect.Hash)

	case msg.InvTypeTx:
		// Is transaction already in mempool
		_, ok := sm.txMemPool[invVect.Hash]
		return ok
	}

	// The requested inventory is is an unsupported type, so just claim
	// it is known to avoid requesting it.
	return true
}

// handleInvMsg handles inv messages from all peers.
// We examine the inventory advertised by the remote peer and act accordingly.
func (sm *SyncManager) handleInvMsg(imsg *invMsg) {
	peer := imsg.peer
	state, exists := sm.peerStates[peer]
	if !exists {
		log.Warnf("Received inv message from unknown peer %s", peer)
		return
	}

	// Attempt to find the final block in the inventory list.  There may
	// not be one.
	var lastBlock *msg.InvVect
	invVects := imsg.inv.InvList
	for i := len(invVects) - 1; i >= 0; i-- {
		if invVects[i].Type == msg.InvTypeBlock {
			lastBlock = invVects[i]
			break
		}
	}

	// Ignore invs from peers that aren't the sync if we are not current.
	// Helps prevent fetching a mass of orphans.
	if peer != sm.syncPeer && !sm.current() {
		return
	}

	// Request the advertised inventory if we don't already have it.
	for _, iv := range invVects {
		// Ignore unsupported inventory types.
		switch iv.Type {
		case msg.InvTypeBlock:
		case msg.InvTypeTx:
		default:
			continue
		}

		// Request the inventory if we don't already have it.
		if !sm.haveInventory(iv) {
			// Add it to the request queue.
			state.requestQueue = append(state.requestQueue, iv)
			continue
		}
	}

	// Check if we are in syncing mode and the request queue is not long enough.
	if !sm.current() && len(state.requestQueue) < minPendingRequests {
		if lastBlock != nil {
			locator := []*common.Uint256{&lastBlock.Hash}
			peer.PushGetBlocksMsg(locator, &zeroHash)
		}
	}

	// If there are any queued inventory, just request them.
	sm.requestQueuedInv(peer, state)
}

func (sm *SyncManager) requestQueuedInv(peer *peer.Peer, state *peerSyncState) {
	// Request as much as possible at once.  Anything that won't fit into
	// the request will be requested on the next inv message.
	numRequested := 0
	gdmsg := msg.NewGetData()
	requestQueue := state.requestQueue
	for len(requestQueue) != 0 {
		iv := requestQueue[0]
		requestQueue[0] = nil
		requestQueue = requestQueue[1:]

		switch iv.Type {
		case msg.InvTypeBlock:
			// Request the block if there is not already a pending
			// request.
			if _, exists := sm.requestedBlocks[iv.Hash]; !exists {
				sm.requestedBlocks[iv.Hash] = struct{}{}
				sm.limitMap(sm.requestedBlocks, maxRequestedBlocks)
				state.requestedBlocks[iv.Hash] = struct{}{}

				iv.Type = msg.InvTypeFilteredBlock
				gdmsg.AddInvVect(iv)
				numRequested++
			}

		case msg.InvTypeTx:
			// Request the transaction if there is not already a
			// pending request.
			if _, exists := sm.requestedTxns[iv.Hash]; !exists {
				sm.requestedTxns[iv.Hash] = struct{}{}
				sm.limitMap(sm.requestedTxns, maxRequestedTxns)
				state.requestedTxns[iv.Hash] = struct{}{}

				gdmsg.AddInvVect(iv)
				numRequested++
			}
		}

		if numRequested >= msg.MaxInvPerMsg {
			break
		}
	}
	state.requestQueue = requestQueue
	if len(gdmsg.InvList) > 0 {
		log.Debugf("QueueMessage getdata size %d", len(gdmsg.InvList))
		peer.QueueMessage(gdmsg, nil)
	}
}

// limitMap is a helper function for maps that require a maximum limit by
// evicting a random transaction if adding a new value would cause it to
// overflow the maximum allowed.
func (sm *SyncManager) limitMap(m map[common.Uint256]struct{}, limit int) {
	if len(m)+1 > limit {
		// Remove a random entry from the map.  For most compilers, Go's
		// range statement iterates starting at a random item although
		// that is not 100% guaranteed by the spec.  The iteration order
		// is not important here because an adversary would have to be
		// able to pull off preimage attacks on the hashing function in
		// order to target eviction of specific entries anyways.
		for txHash := range m {
			delete(m, txHash)
			return
		}
	}
}

// blockHandler is the main handler for the sync manager.  It must be run as a
// goroutine.  It processes block and inv messages in a separate goroutine
// from the peer handlers so the block (MsgBlock) messages are handled by a
// single thread without needing to lock memory data structures.  This is
// important because the sync manager controls which blocks are needed and how
// the fetching should proceed.
func (sm *SyncManager) blockHandler() {
out:
	for {
		select {
		case m := <-sm.msgChan:
			switch msg := m.(type) {
			case *newPeerMsg:
				sm.handleNewPeerMsg(msg.peer)

			case *txMsg:
				sm.handleTxMsg(msg)
				msg.reply <- struct{}{}

			case *blockMsg:
				sm.handleBlockMsg(msg)
				msg.reply <- struct{}{}

			case *invMsg:
				sm.handleInvMsg(msg)

			case *donePeerMsg:
				sm.handleDonePeerMsg(msg.peer)

			case getSyncPeerMsg:
				var peerID uint64
				if sm.syncPeer != nil {
					peerID = sm.syncPeer.ID()
				}
				msg.reply <- peerID

			case isCurrentMsg:
				msg.reply <- sm.current()

			case pauseMsg:
				// Wait until the sender unpauses the manager.
				<-msg.unpause

			default:
				log.Warnf("Invalid message type in block "+
					"handler: %T", msg)
			}

		case <-sm.quit:
			break out
		}
	}

cleanup:
	for {
		select {
		case <-sm.msgChan:
		default:
			break cleanup
		}
	}
}

// NewPeer informs the sync manager of a newly active peer.
func (sm *SyncManager) NewPeer(peer *peer.Peer) {
	// Ignore if we are shutting down.
	if atomic.LoadInt32(&sm.shutdown) != 0 {
		return
	}
	sm.msgChan <- &newPeerMsg{peer: peer}
}

// QueueTx adds the passed transaction message and peer to the block handling
// queue. Responds to the done channel argument after the tx message is
// processed.
func (sm *SyncManager) QueueTx(tx util.Transaction, peer *peer.Peer, done chan struct{}) {
	// Don't accept more transactions if we're shutting down.
	if atomic.LoadInt32(&sm.shutdown) != 0 {
		done <- struct{}{}
		return
	}

	sm.msgChan <- &txMsg{tx: tx, peer: peer, reply: done}
}

// QueueBlock adds the passed block message and peer to the block handling
// queue. Responds to the done channel argument after the block message is
// processed.
func (sm *SyncManager) QueueBlock(block *util.Block, peer *peer.Peer, done chan struct{}) {
	// Don't accept more blocks if we're shutting down.
	if atomic.LoadInt32(&sm.shutdown) != 0 {
		done <- struct{}{}
		return
	}

	sm.msgChan <- &blockMsg{block: block, peer: peer, reply: done}
}

// QueueInv adds the passed inv message and peer to the block handling queue.
func (sm *SyncManager) QueueInv(inv *msg.Inv, peer *peer.Peer) {
	// No channel handling here because peers do not need to block on inv
	// messages.
	if atomic.LoadInt32(&sm.shutdown) != 0 {
		return
	}

	sm.msgChan <- &invMsg{inv: inv, peer: peer}
}

// DonePeer informs the blockmanager that a peer has disconnected.
func (sm *SyncManager) DonePeer(peer *peer.Peer) {
	// Ignore if we are shutting down.
	if atomic.LoadInt32(&sm.shutdown) != 0 {
		return
	}

	sm.msgChan <- &donePeerMsg{peer: peer}
}

// Start begins the core block handler which processes block and inv messages.
func (sm *SyncManager) Start() {
	// Already started?
	if atomic.AddInt32(&sm.started, 1) != 1 {
		return
	}

	go sm.blockHandler()
}

// Stop gracefully shuts down the sync manager by stopping all asynchronous
// handlers and waiting for them to finish.
func (sm *SyncManager) Stop() error {
	if atomic.AddInt32(&sm.shutdown, 1) != 1 {
		log.Warnf("Sync manager is already in the process of " +
			"shutting down")
		return nil
	}

	log.Infof("Sync manager shutting down")
	close(sm.quit)
	return nil
}

// SyncPeerID returns the ID of the current sync peer, or 0 if there is none.
func (sm *SyncManager) SyncPeerID() uint64 {
	reply := make(chan uint64)
	sm.msgChan <- getSyncPeerMsg{reply: reply}
	return <-reply
}

// IsCurrent returns whether or not the sync manager believes it is synced with
// the connected peers.
func (sm *SyncManager) IsCurrent() bool {
	reply := make(chan bool)
	sm.msgChan <- isCurrentMsg{reply: reply}
	return <-reply
}

// Pause pauses the sync manager until the returned channel is closed.
//
// Note that while paused, all peer and block processing is halted.  The
// message sender should avoid pausing the sync manager for long durations.
func (sm *SyncManager) Pause() chan<- struct{} {
	c := make(chan struct{})
	sm.msgChan <- pauseMsg{c}
	return c
}

// New constructs a new SyncManager. Use Start to begin processing asynchronous
// block, tx, and inv updates.
func New(cfg *Config) (*SyncManager, error) {
	sm := SyncManager{
		cfg:             *cfg,
		txMemPool:       make(map[common.Uint256]struct{}),
		requestedTxns:   make(map[common.Uint256]struct{}),
		requestedBlocks: make(map[common.Uint256]struct{}),
		peerStates:      make(map[*peer.Peer]*peerSyncState),
		msgChan:         make(chan interface{}, cfg.MaxPeers*3),
		quit:            make(chan struct{}),
	}

	return &sm, nil
}
