package sdk

import (
	"fmt"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/log"
	"github.com/elastos/Elastos.ELA.SPV/net"
	"github.com/elastos/Elastos.ELA.SPV/store"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA/bloom"
	ela "github.com/elastos/Elastos.ELA/core"
)

const (
	SendTxTimeout     = 10
	MaxFalsePositives = 7
)

type downloadTx struct {
	mutex sync.RWMutex
	queue map[common.Uint256]struct{}
}

func newDownloadTx() *downloadTx {
	return &downloadTx{queue: make(map[common.Uint256]struct{})}
}

func (d *downloadTx) queueTx(txId common.Uint256) {
	d.mutex.Lock()
	defer d.mutex.Unlock()
	d.queue[txId] = struct{}{}
}

func (d *downloadTx) dequeueTx(txId common.Uint256) bool {
	d.mutex.Lock()
	defer d.mutex.Unlock()
	_, ok := d.queue[txId]
	if !ok {
		return false
	}
	delete(d.queue, txId)
	return true
}

type downloadBlock struct {
	mutex sync.RWMutex
	*msg.MerkleBlock
	txQueue map[common.Uint256]struct{}
	txs     []*ela.Transaction
}

func newDownloadBlock() *downloadBlock {
	return &downloadBlock{txQueue: make(map[common.Uint256]struct{})}
}

func (d *downloadBlock) queueTx(txId common.Uint256) {
	d.mutex.Lock()
	defer d.mutex.Unlock()
	d.txQueue[txId] = struct{}{}
}

func (d *downloadBlock) dequeueTx(txId common.Uint256) bool {
	d.mutex.Lock()
	defer d.mutex.Unlock()
	_, ok := d.txQueue[txId]
	if !ok {
		return false
	}
	delete(d.txQueue, txId)
	return true
}

func (d *downloadBlock) finished() bool {
	d.mutex.RLock()
	defer d.mutex.RUnlock()
	return len(d.txQueue) == 0
}

// The SPV service implementation
type SPVServiceImpl struct {
	SPVClient
	chain     *Blockchain
	pendingTx common.Uint256
	txAccept  chan *common.Uint256
	txReject  chan *msg.Reject
	handler   SPVHandler
}

// Create a instance of SPV service implementation.
func NewSPVServiceImpl(client SPVClient, foundation string, headerStore store.HeaderStore, handler SPVHandler) (*SPVServiceImpl, error) {
	var err error
	service := new(SPVServiceImpl)
	// Set spv client
	service.SPVClient = client
	// Initialize blockchain
	service.chain, err = NewBlockchain(foundation, headerStore)
	if err != nil {
		return nil, err
	}

	// Set SPV handler implement
	service.handler = handler

	// Initialize local peer height
	service.updateLocalHeight(service.chain.Height())

	// Set p2p message handler
	service.SPVClient.SetMessageHandler(service.newSpvMsgHandler)

	return service, nil
}

// Update local peer height with current chain height
func (s *SPVServiceImpl) updateLocalHeight(height uint32) {
	log.Info("LocalChain height:", height)
	s.PeerManager().Local().SetHeight(uint64(height))
}

func (s *SPVServiceImpl) newSpvMsgHandler() SPVMessageHandler {
	handler := new(spvMsgHandler)
	handler.service = s

	// Block downloading and commit
	handler.blockQueue = make(chan common.Uint256, p2p.MaxBlocksPerMsg*2)
	handler.downloading = newDownloadBlock()

	// Transaction downloading
	handler.downloadTx = newDownloadTx()

	return handler
}

func (s *SPVServiceImpl) Start() {
	s.SPVClient.Start()
	go s.keepUpdate()
	log.Info("SPV service started...")
}

func (s *SPVServiceImpl) Stop() {
	s.chain.Close()
	log.Info("SPV service stopped...")
}

func (s *SPVServiceImpl) ChainState() ChainState {
	return s.chain.state
}

func (s *SPVServiceImpl) ReloadFilter() {
	log.Debug()
	s.PeerManager().Broadcast(BuildBloomFilter(s.handler.GetData()).GetFilterLoadMsg())
}

func (s *SPVServiceImpl) SendTransaction(tx ela.Transaction) (*common.Uint256, error) {
	log.Debug()

	if s.PeerManager().Peers.PeersCount() == 0 {
		return nil, fmt.Errorf("method not available, no peers connected")
	}

	s.txAccept = make(chan *common.Uint256, 1)
	s.txReject = make(chan *msg.Reject, 1)

	finish := func() {
		close(s.txAccept)
		close(s.txReject)
		s.txAccept = nil
		s.txReject = nil
	}
	// Set transaction in pending
	s.pendingTx = tx.Hash()
	// Broadcast transaction to neighbor peers
	s.PeerManager().Broadcast(msg.NewTx(&tx))
	// Query neighbors mempool see if transaction was successfully added to mempool
	s.PeerManager().Broadcast(new(msg.MemPool))

	// Wait for result
	timer := time.NewTimer(time.Second * SendTxTimeout)
	select {
	case <-timer.C:
		finish()
		return nil, fmt.Errorf("Send transaction timeout")
	case <-s.txAccept:
		timer.Stop()
		finish()
		// commit unconfirmed transaction to db
		_, err := s.handler.CommitTx(&tx, 0)
		return &s.pendingTx, err
	case msg := <-s.txReject:
		timer.Stop()
		finish()
		return nil, fmt.Errorf("Transaction rejected Code: %s, Reason: %s", msg.Code.String(), msg.Reason)
	}
}

func (s *SPVServiceImpl) keepUpdate() {
	ticker := time.NewTicker(time.Second * net.InfoUpdateDuration)
	defer ticker.Stop()
	for range ticker.C {
		// Keep synchronizing blocks
		s.syncBlocks()
	}
}

func (s *SPVServiceImpl) needSync() bool {
	bestPeer := s.PeerManager().GetBestPeer()
	if bestPeer == nil { // no peers connected, return false
		return false
	}
	return bestPeer.Height() > uint64(s.chain.Height())
}

func (s *SPVServiceImpl) syncBlocks() {
	// Check if blockchain need sync
	if s.needSync() {
		// Return if already in syncing
		if s.chain.IsSyncing() {
			return
		}
		// Set blockchain state to syncing
		s.chain.SetChainState(SYNCING)
		// Request blocks
		s.requestBlocks()
	} else {
		if !s.chain.IsSyncing() {
			return
		}
		// Set blockchain state to waiting
		s.chain.SetChainState(WAITING)
		// Remove sync peer
		s.PeerManager().SetSyncPeer(nil)
		// Update bloom filter
		s.ReloadFilter()
	}
}

func (s *SPVServiceImpl) requestBlocks() {
	// Get sync peer
	syncPeer := s.PeerManager().GetSyncPeer()
	if syncPeer == nil {
		// If sync peer is nil at this point, that meas no peer connected
		log.Info("SyncManager no sync peer connected")
		return
	}
	// Request blocks returns a inventory message which contains block hashes
	getBlocks := msg.NewGetBlocks(s.chain.GetBlockLocatorHashes(), common.EmptyHash)

	syncPeer.Send(getBlocks)
}

func (s *SPVServiceImpl) changeSyncPeer() {
	log.Debug("Change sync peer")
	syncPeer := s.PeerManager().GetSyncPeer()
	if syncPeer != nil {
		// Disconnect current sync peer
		syncPeer.Disconnect()
		s.PeerManager().RemovePeer(syncPeer.ID())

		// Restart
		s.requestBlocks()
	}
}

type spvMsgHandler struct {
	peer        *net.Peer
	service     *SPVServiceImpl
	blockQueue  chan common.Uint256
	downloading *downloadBlock
	downloadTx  *downloadTx
	fPositives  int
}

func (h *spvMsgHandler) isSyncPeer() bool {
	return h.service.chain.IsSyncing() && h.service.PeerManager().GetSyncPeer() != nil &&
		h.service.PeerManager().GetSyncPeer().ID() == h.peer.ID()
}

func (h *spvMsgHandler) updateBloomFilter() {
	bloomFilter := BuildBloomFilter(h.service.handler.GetData())
	h.peer.Send(bloomFilter.GetFilterLoadMsg())
}

func (h *spvMsgHandler) OnPeerEstablish(peer *net.Peer) {
	log.Debug(peer)
	// Set handler's peer
	h.peer = peer
	// Send filterload message
	h.updateBloomFilter()
}

func (h *spvMsgHandler) OnInventory(peer *net.Peer, m *msg.Inventory) error {
	log.Debug(peer)
	gData := msg.NewGetData()

	for _, inv := range m.InvList {
		switch inv.Type {
		case msg.InvTypeBlock:
			// Filter duplicated block
			if h.service.chain.IsKnownHeader(&inv.Hash) {
				continue
			}

			// Kind of lame to send separate getData messages but this allows us
			// to take advantage of the timeout on the upper layer. Otherwise we
			// need separate timeout handling.
			inv.Type = msg.InvTypeFilteredBlock
			gData.AddInvVect(inv)
			if h.isSyncPeer() {
				h.blockQueue <- inv.Hash
			}
		case msg.InvTypeTx:
			if h.service.txAccept != nil && h.service.pendingTx.IsEqual(inv.Hash) {
				h.service.txAccept <- nil
				continue
			}
			gData.AddInvVect(inv)
			h.downloadTx.queueTx(inv.Hash)
		default:
			continue
		}
	}

	if len(gData.InvList) > 0 {
		// Start sync timer
		peer.Send(gData)
	}
	return nil
}

func (h *spvMsgHandler) OnMerkleBlock(peer *net.Peer, block *msg.MerkleBlock) error {
	log.Debug(peer)
	blockHash := block.Header.(*ela.Header).Hash()

	// Merkleblock from sync peer
	if h.isSyncPeer() {
		queueHash := <-h.blockQueue
		if !blockHash.IsEqual(queueHash) {
			h.changeSyncPeer()
			return fmt.Errorf("peer %d is sending us blocks out of order", peer.ID())
		}
	}

	txIds, err := bloom.CheckMerkleBlock(*block)
	if err != nil {
		return fmt.Errorf("invalid merkleblock received %s", err.Error())
	}

	// Save block as download block
	h.downloading.MerkleBlock = block

	// No transactions to download, just finish it
	if len(txIds) == 0 {
		h.finishDownload()
		return nil
	}

	// Download transactions of this block
	for _, txId := range txIds {
		h.downloading.queueTx(*txId)
	}

	return nil
}

func (h *spvMsgHandler) OnTx(peer *net.Peer, msg *msg.Tx) error {
	log.Debug(peer)
	tx := msg.Transaction.(*ela.Transaction)
	if h.downloadTx.dequeueTx(tx.Hash()) {
		// commit unconfirmed transaction
		_, err := h.service.handler.CommitTx(tx, 0)
		if err == nil {
			h.updateBloomFilter()
		}
		return err
	}

	if !h.downloading.dequeueTx(tx.Hash()) {
		h.downloading = newDownloadBlock()
		return fmt.Errorf("Transaction not found in download queue %s", tx.Hash().String())
	}

	// Add tx to download
	h.downloading.txs = append(h.downloading.txs, tx)

	// All transactions of the download block have been received, commit the download block
	if h.downloading.finished() {
		h.finishDownload()
	}

	return nil
}

func (h *spvMsgHandler) OnNotFound(peer *net.Peer, notFound *msg.NotFound) error {
	log.Debug(peer)
	for _, iv := range notFound.InvList {
		log.Warnf("Data not found type %s, hash %s", iv.Type.String(), iv.Hash.String())
		switch iv.Type {
		case msg.InvTypeTx:
			if h.downloadTx.dequeueTx(iv.Hash) {
			}
			if h.downloading.dequeueTx(iv.Hash) {
				h.downloading = newDownloadBlock()
			}
		case msg.InvTypeBlock:
			// reset downloading block
			h.downloading = newDownloadBlock()

			if h.isSyncPeer() {
				h.changeSyncPeer()
			}
		}
	}
	return nil
}

func (h *spvMsgHandler) OnReject(peer *net.Peer, msg *msg.Reject) error {
	log.Debug(peer)
	if h.service.pendingTx.IsEqual(msg.Hash); h.service.txReject != nil {
		h.service.txReject <- msg
		return nil
	}
	return fmt.Errorf("Received reject message from peer %d: Code: %s, Hash %s, Reason: %s",
		peer.ID(), msg.Code.String(), msg.Hash.String(), msg.Reason)
}

func (h *spvMsgHandler) changeSyncPeer() {
	// Reset downloading block
	h.downloading = newDownloadBlock()
	// Reset downloading transaction
	h.downloadTx = newDownloadTx()

	// Clear download block queue
	for len(h.blockQueue) > 0 {
		<-h.blockQueue
	}

	// Change sync peer
	h.service.changeSyncPeer()
}

func (h *spvMsgHandler) finishDownload() {
	// Commit downloaded block
	h.commitBlock(h.downloading)
	h.downloading = newDownloadBlock()
	// Request next block list when in syncing
	if h.isSyncPeer() && len(h.blockQueue) == 0 {
		// Request more blocks
		h.service.requestBlocks()
	}
}

func (h *spvMsgHandler) commitBlock(block *downloadBlock) {
	header := block.Header.(*ela.Header)
	newTip, reorgFrom, err := h.service.chain.CommitHeader(*header)
	if err != nil {
		log.Errorf("Commit header failed %s", err.Error())
		// If a syncing peer send us bad block, disconnect it.
		if h.isSyncPeer() {
			h.changeSyncPeer()
		}
		return
	}
	if !newTip {
		return
	}

	newHeight := h.service.chain.Height()
	if reorgFrom > 0 {
		for i := reorgFrom; i > newHeight; i-- {
			if err = h.service.handler.OnRollback(i); err != nil {
				log.Errorf("Rollback transaction at height %d failed %s", i, err.Error())
				return
			}
		}

		if !h.service.chain.IsSyncing() {
			h.service.syncBlocks()
			return
		}
	}

	for _, tx := range block.txs {
		falsePositive, err := h.service.handler.CommitTx(tx, header.Height)
		if err != nil {
			log.Errorf("Commit transaction %s failed %s", tx.Hash().String(), err.Error())
			return
		}

		if falsePositive {
			h.fPositives++
			if h.fPositives > MaxFalsePositives {
				// Broadcast filterload message to connected peers
				h.updateBloomFilter()
				h.fPositives = 0
			}
			continue
		}
	}

	h.service.updateLocalHeight(newHeight)
	h.service.handler.OnBlockCommitted(block.MerkleBlock, block.txs)
}
