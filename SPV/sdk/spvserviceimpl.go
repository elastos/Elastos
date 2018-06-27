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
	sync.Mutex
	SPVClient
	chain       *Blockchain
	blockQueue  chan common.Uint256
	downloading *downloadBlock
	commitQueue chan *downloadBlock
	downloadTx  *downloadTx
	pendingTx   common.Uint256
	txAccept    chan *common.Uint256
	txReject    chan *msg.Reject
	handler     SPVHandler
	fPositives  int
}

// Create a instance of SPV service implementation.
func NewSPVServiceImpl(client SPVClient, headerStore store.HeaderStore, handler SPVHandler) (*SPVServiceImpl, error) {
	var err error
	service := new(SPVServiceImpl)
	// Set spv client
	service.SPVClient = client
	// Initialize blockchain
	service.chain, err = NewBlockchain(headerStore)
	if err != nil {
		return nil, err
	}

	// Block downloading and commit
	service.blockQueue = make(chan common.Uint256, p2p.MaxBlocksPerMsg*3)
	service.downloading = newDownloadBlock()
	service.commitQueue = make(chan *downloadBlock, p2p.MaxBlocksPerMsg*2)

	// Transaction downloading
	service.downloadTx = newDownloadTx()

	// Initialize local peer height
	service.updateLocalHeight(service.chain.Height())

	// Set p2p message handler
	service.SPVClient.SetMessageHandler(service)

	// Set SPV handler implement
	service.handler = handler

	go service.startBlockCommitQueue()
	return service, nil
}

func (s *SPVServiceImpl) OnPeerEstablish(peer *net.Peer) {
	// Send filterload message
	s.updateFilterAndSend(peer)
}

func (s *SPVServiceImpl) Start() {
	s.SPVClient.Start()
	go s.keepUpdate()
	log.Info("SPV service started...")
}

func (s *SPVServiceImpl) Stop() {
	s.stopSyncing()
	s.chain.Close()
	log.Info("SPV service stopped...")
}

func (s *SPVServiceImpl) ChainState() ChainState {
	return s.chain.state
}

func (s *SPVServiceImpl) ReloadFilter() {
	s.PeerManager().Broadcast(BuildBloomFilter(s.handler.GetData()).GetFilterLoadMsg())
}

func (s *SPVServiceImpl) SendTransaction(tx ela.Transaction) (*common.Uint256, error) {
	s.Lock()
	defer s.Unlock()

	if s.PeerManager().Peers.PeersCount() == 0 {
		return nil, fmt.Errorf("method not available, no peers connected")
	}

	txId := tx.Hash()
	s.txAccept = make(chan *common.Uint256, 1)
	s.txReject = make(chan *msg.Reject, 1)

	finish := func(txId common.Uint256) {
		close(s.txAccept)
		close(s.txReject)
		s.txAccept = nil
		s.txReject = nil
	}
	// Set transaction in pending
	s.pendingTx = txId
	// Broadcast transaction to neighbor peers
	s.PeerManager().Broadcast(msg.NewTx(&tx))
	// Query neighbors mempool see if transaction was successfully added to mempool
	s.PeerManager().Broadcast(new(msg.MemPool))

	// Wait for result
	timer := time.NewTimer(time.Second * SendTxTimeout)
	select {
	case <-timer.C:
		finish(txId)
		return nil, fmt.Errorf("Send transaction timeout")
	case <-s.txAccept:
		timer.Stop()
		finish(txId)
		// commit unconfirmed transaction to db
		_, err := s.handler.CommitTx(&tx, 0)
		return &txId, err
	case msg := <-s.txReject:
		timer.Stop()
		finish(txId)
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

func (s *SPVServiceImpl) updateFilterAndSend(peer *net.Peer) {
	peer.Send(BuildBloomFilter(s.handler.GetData()).GetFilterLoadMsg())
}

func (s *SPVServiceImpl) needSync() bool {
	bestPeer := s.PeerManager().GetBestPeer()
	if bestPeer == nil { // no peers connected, return false
		return false
	}
	return bestPeer.Height() > uint64(s.chain.Height())
}

func (s *SPVServiceImpl) syncBlocks() {
	// Return if current download not finished
	if !s.downloading.finished() {
		return
	}
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
		s.stopSyncing()
	}
}

func (s *SPVServiceImpl) stopSyncing() {
	if s.chain.IsSyncing() {
		// Set blockchain state to waiting
		s.chain.SetChainState(WAITING)
		// Remove sync peer
		s.PeerManager().SetSyncPeer(nil)

		// Clear block queue
		for len(s.blockQueue) > 0 {
			<-s.blockQueue
		}
		// Reset download block
		s.downloading = newDownloadBlock()
		// Clear commit queue
		for len(s.commitQueue) > 0 {
			<-s.commitQueue
		}

		// Reset download transaction
		s.downloadTx = newDownloadTx()
	}
}

func (s *SPVServiceImpl) requestBlocks() {
	// Get sync peer
	syncPeer := s.PeerManager().GetSyncPeer()
	if syncPeer == nil {
		// If sync peer is nil at this point, that meas no peer connected
		fmt.Println("SyncManager no sync peer connected")
		return
	}
	// Request blocks returns a inventory message which contains block hashes
	getBlocks := msg.NewGetBlocks(s.chain.GetBlockLocatorHashes(), common.EmptyHash)

	syncPeer.Send(getBlocks)
}

func (s *SPVServiceImpl) startBlockCommitQueue() {
	for block := range s.commitQueue {
		s.commitBlock(block)
	}
}

func (s *SPVServiceImpl) changeSyncPeerAndRestart() {
	log.Debug("Change sync peer and restart")
	// Disconnect current sync peer
	syncPeer := s.PeerManager().GetSyncPeer()
	s.PeerManager().OnDisconnected(syncPeer)

	s.stopSyncing()
	// Restart
	s.syncBlocks()
}

func (s *SPVServiceImpl) OnInventory(peer *net.Peer, m *msg.Inventory) error {
	gData := msg.NewGetData()

	for _, inv := range m.InvList {
		switch inv.Type {
		case msg.InvTypeBlock:
			// Kind of lame to send separate getData messages but this allows us
			// to take advantage of the timeout on the upper layer. Otherwise we
			// need separate timeout handling.
			inv.Type = msg.InvTypeFilteredBlock
			gData.AddInvVect(inv)
			if s.chain.IsSyncing() &&
				s.PeerManager().GetSyncPeer() != nil && s.PeerManager().GetSyncPeer().ID() == peer.ID() {
				s.blockQueue <- inv.Hash
			}
		case msg.InvTypeTx:
			if s.txAccept != nil && s.pendingTx.IsEqual(inv.Hash) {
				s.txAccept <- nil
				continue
			}
			gData.AddInvVect(inv)
			s.downloadTx.queueTx(inv.Hash)
		default:
			continue
		}
	}

	if len(gData.InvList) > 0 {
		peer.Send(gData)
	}
	return nil
}

func (s *SPVServiceImpl) OnMerkleBlock(peer *net.Peer, block *msg.MerkleBlock) error {
	s.Lock()
	defer s.Unlock()

	blockHash := block.Header.(*ela.Header).Hash()

	// Merkleblock from sync peer
	if s.chain.IsSyncing() &&
		s.PeerManager().GetSyncPeer() != nil && s.PeerManager().GetSyncPeer().ID() == peer.ID() {
		queueHash := <-s.blockQueue
		if !blockHash.IsEqual(queueHash) {
			s.changeSyncPeerAndRestart()
			return fmt.Errorf("peer %d is sending us blocks out of order", peer.ID())
		}

		// Request next block list
		if len(s.blockQueue) == 0 {
			peer.Send(msg.NewGetBlocks([]*common.Uint256{&queueHash}, common.EmptyHash))
		}
	}

	txIds, err := bloom.CheckMerkleBlock(*block)
	if err != nil {
		return fmt.Errorf("invalid merkleblock received %s", err.Error())
	}

	// Save block as download block
	s.downloading.MerkleBlock = block

	// No transactions to download, just commit the block
	if len(txIds) == 0 {
		s.commitQueue <- s.downloading
		s.downloading = newDownloadBlock()
		return nil
	}

	// Download transactions of this block
	for _, txId := range txIds {
		s.downloading.queueTx(*txId)
	}

	return nil
}

func (s *SPVServiceImpl) OnTx(peer *net.Peer, msg *msg.Tx) error {
	s.Lock()
	defer s.Unlock()

	tx := msg.Transaction.(*ela.Transaction)
	if s.downloadTx.dequeueTx(tx.Hash()) {
		// commit unconfirmed transaction
		_, err := s.handler.CommitTx(tx, 0)
		if err == nil {
			s.updateFilterAndSend(peer)
		}
		return err
	}

	if !s.downloading.dequeueTx(tx.Hash()) {
		s.downloading = newDownloadBlock()
		return fmt.Errorf("Transaction not found in download queue %s", tx.Hash().String())
	}

	// Add tx to download
	s.downloading.txs = append(s.downloading.txs, tx)

	// All transactions of the download block have been received, commit the download block
	if s.downloading.finished() {
		s.commitQueue <- s.downloading
		s.downloading = newDownloadBlock()
	}

	return nil
}

func (s *SPVServiceImpl) commitBlock(block *downloadBlock) {
	header := block.Header.(*ela.Header)
	newTip, reorgFrom, err := s.chain.CommitHeader(*header)
	if err != nil {
		log.Errorf("Commit header failed %s", err.Error())
		return
	}
	if !newTip {
		return
	}

	newHeight := s.chain.Height()
	if reorgFrom > 0 {
		for i := reorgFrom; i > newHeight; i-- {
			if err = s.handler.OnRollback(i); err != nil {
				fmt.Printf("Rollback transaction at height %d failed %s", i, err.Error())
				return
			}
		}

		if !s.chain.IsSyncing() {
			s.syncBlocks()
			return
		}
	}

	for _, tx := range block.txs {
		falsePositive, err := s.handler.CommitTx(tx, header.Height)
		if err != nil {
			fmt.Printf("Commit transaction %s failed %s", tx.Hash().String(), err.Error())
			return
		}

		if falsePositive {
			s.fPositives++
			if s.fPositives > MaxFalsePositives {
				// Broadcast filterload message to connected peers
				s.ReloadFilter()
				s.fPositives = 0
			}
			continue
		}
	}

	s.updateLocalHeight(newHeight)
	s.handler.OnBlockCommitted(block.MerkleBlock, block.txs)
}

func (s *SPVServiceImpl) OnNotFound(peer *net.Peer, notFound *msg.NotFound) error {
	s.Lock()
	defer s.Unlock()
	for _, iv := range notFound.InvList {
		log.Warnf("Data not found type %s, hash %s", iv.Type.String(), iv.Hash.String())
		switch iv.Type {
		case msg.InvTypeTx:
			if s.downloadTx.dequeueTx(iv.Hash) {
			}
			if s.downloading.dequeueTx(iv.Hash) {
				s.downloading = newDownloadBlock()
			}
		case msg.InvTypeBlock:
			if s.chain.IsSyncing() &&
				s.PeerManager().GetSyncPeer() != nil && s.PeerManager().GetSyncPeer().ID() == peer.ID() {
				s.changeSyncPeerAndRestart()
			}
		}
	}
	return nil
}

func (s *SPVServiceImpl) OnReject(peer *net.Peer, msg *msg.Reject) error {
	if s.pendingTx.IsEqual(msg.Hash); s.txReject != nil {
		s.txReject <- msg
		return nil
	}
	return fmt.Errorf("Received reject message from peer %d: Code: %s, Hash %s, Reason: %s",
		peer.ID(), msg.Code.String(), msg.Hash.String(), msg.Reason)
}

// Update local peer height with current chain height
func (s *SPVServiceImpl) updateLocalHeight(height uint32) {
	log.Info("LocalChain height:", height)
	s.PeerManager().Local().SetHeight(uint64(height))
}
