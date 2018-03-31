package spvwallet

import (
	"bytes"
	"errors"
	"fmt"
	"sync"
	"time"

	. "github.com/elastos/Elastos.ELA.SPV/common"
	tx "github.com/elastos/Elastos.ELA.SPV/core/transaction"
	. "github.com/elastos/Elastos.ELA.SPV/p2p"
	. "github.com/elastos/Elastos.ELA.SPV/spvwallet/msg"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/log"
	"github.com/elastos/Elastos.ELA.SPV/bloom"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/db"
)

const (
	RequestTimeout    = 15
	MaxRetryTimes     = 6
	MaxFalsePositives = 7
)

type SyncManager struct {
	*MemCache

	dataLock       sync.RWMutex
	blockLocator   []*Uint256
	startHash      *Uint256
	stopHash       *Uint256
	queueLock      sync.RWMutex
	requestQueue   map[Uint256]*request
	blockTxns      map[Uint256][]*request
	receivedBlocks map[Uint256]*bloom.MerkleBlock
	receivedTxns   map[Uint256]*tx.Transaction
	fPositives     int
}

func NewSyncManager() *SyncManager {
	sm := new(SyncManager)
	sm.MemCache = NewMemCache()
	sm.clear()
	return sm
}

func (sm *SyncManager) SyncBlocks() {
	// Check if blockchain is in syncing state
	if spvWallet.chain.IsSyncing() {
		return
	}

	sm.startSync()
}

func (sm *SyncManager) clear() {
	sm.blockLocator = nil
	sm.startHash = nil
	sm.stopHash = nil
	sm.clearRequests()
	sm.requestQueue = make(map[Uint256]*request)
	sm.blockTxns = make(map[Uint256][]*request)
	sm.receivedBlocks = make(map[Uint256]*bloom.MerkleBlock)
	sm.receivedTxns = make(map[Uint256]*tx.Transaction)
}

func (sm *SyncManager) clearRequests() {
	sm.queueLock.Lock()
	if sm.requestQueue != nil {
		for _, request := range sm.requestQueue {
			request.finish()
		}
	}
	sm.queueLock.Unlock()
}

func (sm *SyncManager) startSync() {
	log.Info("SyncManger start sync")
	// Clear data first
	sm.clear()
	// Check if blockchain need sync
	if sm.needSync() {
		// Set blockchain state to syncing
		spvWallet.chain.SetChainState(SYNCING)
		// Request blocks
		sm.requestBlocks()
	} else {
		spvWallet.chain.SetChainState(WAITING)
		spvWallet.PeerManager().SetSyncPeer(nil)
	}
}

func (sm *SyncManager) needSync() bool {
	bestPeer := spvWallet.PeerManager().GetBestPeer()
	if bestPeer == nil { // no peers connected, return true
		return true
	}
	log.Info("Best peer height:", bestPeer.Height())

	needSync := bestPeer.Height() > uint64(spvWallet.chain.Height())

	log.Trace("Need Sync:", needSync)
	return needSync
}

func (sm *SyncManager) requestBlocks() {
	// Current blocks request was not finished
	if sm.blockLocator != nil {
		log.Error("SyncManager current request not finished")
		return
	}
	// Get sync peer
	syncPeer := spvWallet.PeerManager().GetSyncPeer()
	if syncPeer == nil {
		// If sync peer is nil at this point, that meas no peer connected
		log.Error("SyncManager no sync peer connected")
		go func() {
			// Restart sync after a while
			time.Sleep(time.Second * RequestTimeout)
			sm.startSync()
		}()
		return
	}
	// Request blocks returns a inventory message witch contains block hashes
	sm.blockLocator = spvWallet.chain.GetBlockLocatorHashes()

	go syncPeer.Send(NewBlocksReq(sm.blockLocator, Uint256{}))
}

func (sm *SyncManager) HandleBlockInvMsg(peer *Peer, inv *Inventory) error {
	log.Trace(">>>> Handle inv msg data count:", inv.Count, ", length:", len(inv.Data))

	if sm.blockLocator == nil || spvWallet.PeerManager().GetSyncPeer() == nil || spvWallet.PeerManager().GetSyncPeer().ID() != peer.ID() {

		sm.ChangeSyncPeerAndRestart()
		return errors.New("Receive message from non sync peer, disconnect")
	}

	dataLen := len(inv.Data)
	if dataLen != int(inv.Count)*UINT256SIZE {
		log.Error("Invalid block inventory data size:", dataLen)
		sm.ChangeSyncPeerAndRestart()
		return errors.New(fmt.Sprint("Invalid block inventory data size:", dataLen))
	}

	var reqList []*request
	for i := 0; i < dataLen; i += UINT256SIZE {
		var blockHash Uint256
		err := blockHash.Deserialize(bytes.NewReader(inv.Data[i:i+UINT256SIZE]))
		if err != nil {
			log.Error("Deserialize block hash error,", err)
			sm.ChangeSyncPeerAndRestart()
			return err
		}
		// Create request
		request := &request{
			hash:      blockHash,
			reqType:   BLOCK,
			onTimeout: sm.ChangeSyncPeerAndRestart,
		}
		// Add block hashes to request queue
		sm.addToRequestQueue(request)

		// Save start and stop block for connect headers method
		if sm.startHash == nil {
			sm.startHash = &blockHash
		}
		sm.stopHash = &blockHash

		log.Trace("Add block request:", blockHash.String())
		reqList = append(reqList, request)
	}

	// Send request message
	for _, request := range reqList {
		// Check if block already in orphan pool
		if block, ok := spvWallet.IsOrphanBlock(request.hash); ok {
			sm.BlockReceived(request.hash, block)
			err := sm.RequestBlockTxns(peer, block)
			if err != nil {
				return err
			}
		} else {
			request.start()
		}
	}
	return nil
}

func (sm *SyncManager) RequestBlockTxns(peer *Peer, block *bloom.MerkleBlock) error {
	txIds, err := bloom.CheckMerkleBlock(*block)
	if err != nil {
		log.Error("Invalid merkle block received:", err)
		sm.ChangeSyncPeerAndRestart()
		return err
	}

	// If all blocks received and no more txn to request, submit received block and txn data
	if sm.RequestFinished() && len(txIds) == 0 {

		err := sm.CommitData()
		if err != nil {
			return err
		}

		// Continue syncing
		sm.startSync()

		return nil
	}

	var blockTxns = make([]*request, len(txIds))
	for i, txId := range txIds {
		request := &request{
			hash:      *txId,
			reqType:   TRANSACTION,
			onTimeout: sm.ChangeSyncPeerAndRestart,
		}
		blockTxns[i] = request
		sm.addToRequestQueue(request)
	}
	sm.blockTxns[*block.BlockHeader.Hash()] = blockTxns

	for _, request := range blockTxns {
		if txn, ok := sm.IsOrphanTxn(request.hash); ok {
			sm.TxnReceived(request.hash, txn)
		} else {
			request.start()
		}
	}

	return nil
}

func (sm *SyncManager) addToRequestQueue(request *request) {
	// Add to request queue
	sm.queueLock.Lock()
	sm.requestQueue[request.hash] = request
	sm.queueLock.Unlock()
}

func (sm *SyncManager) onRequestTimeout() {
	sm.clear()
	sm.ChangeSyncPeerAndRestart()
}

func (sm *SyncManager) ChangeSyncPeerAndRestart() {
	// Disconnect sync peer
	syncPeer := spvWallet.PeerManager().GetSyncPeer()
	spvWallet.PeerManager().DisconnectPeer(syncPeer)

	// Restart
	sm.startSync()
}

func (sm *SyncManager) InRequestQueue(hash Uint256) bool {
	sm.queueLock.Lock()
	defer sm.queueLock.Unlock()

	_, ok := sm.requestQueue[hash]
	log.Trace("In request queue: ", ok, ", hash: ", hash.String())
	return ok
}

func (sm *SyncManager) BlockReceived(blockHash Uint256, block *bloom.MerkleBlock) {
	sm.queueLock.Lock()
	sm.finishRequest(blockHash)
	sm.receivedBlocks[blockHash] = block
	sm.queueLock.Unlock()
}

func (sm *SyncManager) TxnReceived(txId Uint256, txn *Txn) {
	sm.queueLock.Lock()
	sm.finishRequest(txId)
	sm.receivedTxns[txId] = &txn.Transaction
	sm.queueLock.Unlock()
}

func (sm *SyncManager) finishRequest(hash Uint256) {
	request, ok := sm.requestQueue[hash]
	if ok {
		request.finish()
	}
	delete(sm.requestQueue, hash)
}

func (sm *SyncManager) RequestFinished() bool {
	sm.queueLock.RLock()
	defer sm.queueLock.RUnlock()

	finished := len(sm.requestQueue) == 0
	log.Trace("Request finished: ", finished)
	return finished
}

func (sm *SyncManager) CommitData() error {
	log.Trace("Commit data")
	sm.queueLock.RLock()
	defer sm.queueLock.RUnlock()

	// Connect headers
	connectedHeaders, err := sm.connectHeaders()
	if err != nil {
		return err
	}

	// Save received blocks and txns to database
	err = sm.commitToDB(connectedHeaders)
	if err != nil {
		// If anything goes wrong, change a sync peer and restart
		log.Error("Save headers failed, disconnect")
		sm.ChangeSyncPeerAndRestart()
		return err
	}

	return nil
}

func (sm *SyncManager) connectHeaders() ([]Uint256, error) {
	log.Trace("Connect headers")
	headersCount := len(sm.receivedBlocks)
	connectedHeaders := make([]Uint256, headersCount)
	for {
		headersCount--
		header := sm.receivedBlocks[*sm.stopHash]
		connectedHeaders[headersCount] = *sm.stopHash

		// All headers connected, return connected headers in height ASC order
		if *sm.stopHash == *sm.startHash {
			log.Trace("Headers connected length:", len(connectedHeaders))
			return connectedHeaders, nil
		}

		previousHash := header.BlockHeader.Previous

		if _, ok := sm.receivedBlocks[previousHash]; !ok {
			log.Error("Connect received headers failed, disconnect")
			sm.ChangeSyncPeerAndRestart()
			return nil, errors.New("Sync manager connect headers failed")
		}
		// Set stop hash to previous for next loop
		sm.stopHash = &previousHash
	}
}

func (sm *SyncManager) commitToDB(hashes []Uint256) error {
	// Get block txns and put header and txns into database in order,
	// for each block save txns first and save block header
	// if anything goes wrong, try to rollback and return error
	for _, hash := range hashes {
		block := sm.receivedBlocks[hash]
		header := block.BlockHeader
		txnRequests := sm.blockTxns[hash]
		txns := make([]tx.Transaction, len(txnRequests))
		for i, request := range txnRequests {
			txns[i] = *sm.receivedTxns[request.hash]
		}

		// Commit block data to blockchain
		fPositives, err := spvWallet.chain.CommitBlock(header, GetProof(block), txns)
		if err != nil {
			return err
		}
		sm.handleFPositive(fPositives)
	}
	// Update local peer height
	spvWallet.updateLocalHeight()
	return nil
}

func (sm *SyncManager) handleFPositive(fPositives int) {
	sm.fPositives += fPositives
	if sm.fPositives > MaxFalsePositives {
		// Broadcast filterload message to connected peers
		spvWallet.PeerManager().Broadcast(spvWallet.chain.GetBloomFilter().GetFilterLoadMsg())
		sm.fPositives = 0
	}
}

func GetProof(msg *bloom.MerkleBlock) db.Proof {
	return db.Proof{
		BlockHash:    *msg.BlockHeader.Hash(),
		Height:       msg.BlockHeader.Height,
		Transactions: msg.Transactions,
		Hashes:       msg.Hashes,
		Flags:        msg.Flags,
	}
}
