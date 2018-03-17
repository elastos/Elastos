package wallet

import (
	"bytes"
	"errors"
	"fmt"
	"sync"
	"time"

	. "SPVWallet/core"
	tx "SPVWallet/core/transaction"
	"SPVWallet/db"
	. "SPVWallet/p2p"
	. "SPVWallet/p2p/msg"
	"SPVWallet/log"
)

const (
	SyncReqTimeout = 15
	MaxRetryTimes  = 10
)

type SyncManager struct {
	lock sync.Mutex
	*MemCache

	blockLocator   []*Uint256
	startHash      *Uint256
	stopHash       *Uint256
	queueLock      sync.RWMutex
	requestQueue   map[Uint256]time.Time
	blockTxns      map[Uint256][]Uint256
	receivedBlocks map[Uint256]*MerkleBlock
	receivedTxns   map[Uint256]*Txn
	retryTimes     int
}

func NewSyncManager() *SyncManager {
	sm := new(SyncManager)
	sm.MemCache = NewMemCache()
	sm.clear()
	return sm
}

func (sm *SyncManager) SyncBlocks() {
	sm.lock.Lock()
	defer sm.lock.Unlock()

	// Check if blockchain is in syncing state
	if spv.chain.IsSyncing() {
		return
	}

	sm.startSync()
}

func (sm *SyncManager) clear() {
	sm.blockLocator = nil
	sm.startHash = nil
	sm.stopHash = nil
	sm.requestQueue = make(map[Uint256]time.Time)
	sm.blockTxns = make(map[Uint256][]Uint256)
	sm.receivedBlocks = make(map[Uint256]*MerkleBlock)
	sm.receivedTxns = make(map[Uint256]*Txn)
	sm.retryTimes = 0
}

func (sm *SyncManager) startSync() {
	log.Info("SyncManger start sync")
	// Clear data cache
	sm.clear()
	// Check if blockchain need sync
	if sm.needSync() {
		// Set blockchain state to syncing
		spv.chain.SetChainState(db.SYNCING)
		sm.requestBlocks()
	} else {
		spv.chain.SetChainState(db.WAITING)
		spv.pm.SetSyncPeer(nil)
	}
}

func (sm *SyncManager) needSync() bool {
	bestPeer := spv.pm.GetBestPeer()
	if bestPeer == nil { // no peers connected, return true
		return true
	}
	log.Info("Best peer height:", bestPeer.Height())
	return bestPeer.Height() > uint64(spv.chain.Height())
}

func (sm *SyncManager) requestBlocks() {
	log.Info("SyncManager request blocks")
	// Current blocks request was not finished
	if sm.blockLocator != nil {
		log.Error("SyncManager current request not finished")
		return
	}
	// Get sync peer
	syncPeer := spv.pm.GetSyncPeer()
	if syncPeer == nil {
		// If sync peer is nil at this point, that meas no peer connected
		log.Error("SyncManager no sync peer connected")
		go func() {
			// Restart sync after a while
			time.Sleep(time.Second * SyncReqTimeout)
			sm.startSync()
		}()
		return
	}
	// Request blocks returns a inventory message witch contains block hashes
	sm.blockLocator = spv.chain.GetBlockLocatorHashes()
	msg, err := NewBlocksReqMsg(sm.blockLocator, Uint256{})
	if err != nil {
		log.Error("Sync blocks new blocks request message failed, ", err)
		return
	}

	go syncPeer.Send(msg)
}

func (sm *SyncManager) HandleBlockInvMsg(peer *Peer, inv *Inventory) error {
	sm.lock.Lock()
	defer sm.lock.Unlock()

	if sm.blockLocator == nil || spv.pm.GetSyncPeer() == nil || spv.pm.GetSyncPeer().ID() != peer.ID() {

		log.Error("Receive message from non sync peer, disconnect")
		sm.ChangeSyncPeerAndRestart()
		return errors.New("Receive message from non sync peer, disconnect")
	}

	dataLen := len(inv.Data)
	if dataLen != int(inv.Count)*UINT256SIZE {
		log.Error(">>>>> Invalid block inventory data size:", dataLen)
		sm.ChangeSyncPeerAndRestart()
		return errors.New(fmt.Sprint("Invalid block inventory data size:", dataLen))
	}

	for i := 0; i < dataLen; i += UINT256SIZE {
		var blockHash Uint256
		err := blockHash.Deserialize(bytes.NewReader(inv.Data[i:i+UINT256SIZE]))
		if err != nil {
			log.Error(">>>>> Deserialize block hash error,", err)
			sm.ChangeSyncPeerAndRestart()
			return err
		}
		// Check if block already in orphan pool
		if block, ok := spv.IsOrphanBlock(blockHash); ok {
			sm.RequestBlockTxns(peer, block)
			return nil
		}
		if !spv.chain.IsKnownBlock(blockHash) {
			<-time.After(time.Millisecond * 50)
			// Save start and stop block for connect headers method
			if sm.startHash == nil {
				sm.startHash = &blockHash
			}
			sm.stopHash = &blockHash

			// Send request message
			sm.sendRequest(peer, BLOCK, blockHash)
		}
	}
	return nil
}

func (sm *SyncManager) RequestBlockTxns(peer *Peer, block *MerkleBlock) error {
	txIds, err := CheckMerkleBlock(block)
	if err != nil {
		log.Error("Invalid merkle block received:", err)
		sm.ChangeSyncPeerAndRestart()
		return err
	}

	// If all blocks received and no more txn to request, submit received block and txn data
	if sm.RequestFinished() && len(txIds) == 0 {
		return sm.submitData()
	}

	var blockTxns = make([]Uint256, len(txIds))
	for i, txId := range txIds {
		blockTxns[i] = *txId
	}
	sm.blockTxns[*block.BlockHeader.Hash()] = blockTxns

	for _, txId := range blockTxns {
		if txn, ok := sm.IsOrphanTxn(txId); ok {
			sm.TxnReceived(&txId, txn)
		} else {
			spv.sendRequest(peer, TRANSACTION, txId)
		}
	}

	return nil
}

func (sm *SyncManager) addToRequestQueue(hash Uint256) {
	// Add to request queue
	log.Trace(">>>>> Add to request queue:", hash.String())
	sm.queueLock.Lock()
	sm.requestQueue[hash] = time.Now()
	sm.queueLock.Unlock()
}

func (sm *SyncManager) sendRequest(peer *Peer, msgType byte, hash Uint256) error {
	sm.addToRequestQueue(hash)

	msg, err := NewDataReqMsg(msgType, hash)
	if err != nil {
		return err
	}
	go peer.Send(msg)

	return nil
}

func (sm *SyncManager) checkTimeOut() {
	ticker := time.NewTicker(SyncReqTimeout)
	defer ticker.Stop()
	for range ticker.C {
		// Not requests return
		if len(sm.requestQueue) == 0 {
			return
		}

		for _, startTime := range sm.requestQueue {
			if time.Now().Sub(startTime).Seconds() > SyncReqTimeout {
				// To many retry times, change a sync peer, and restart syncing
				if sm.retryTimes > MaxRetryTimes {
					log.Error(">>>>> Request timeout, disconnect")
					sm.ChangeSyncPeerAndRestart()
					return
				}
				sm.retryTimes++
			}
		}
	}
}

func (sm *SyncManager) ChangeSyncPeerAndRestart() {
	// Disconnect sync peer
	syncPeer := spv.pm.GetSyncPeer()
	log.Trace(">>>>> SyncManager change sync peer, disconnect peer", syncPeer)
	spv.pm.DisconnectPeer(syncPeer)

	// Restart
	sm.startSync()
}

func (sm *SyncManager) InRequestQueue(hash Uint256) bool {
	sm.queueLock.Lock()
	defer sm.queueLock.Unlock()

	_, ok := sm.requestQueue[hash]
	log.Trace(">>>>> In request queue", ok, ", hash:", hash.String())
	return ok
}

func (sm *SyncManager) BlockReceived(blockHash *Uint256, block *MerkleBlock) {
	spv.queueLock.Lock()
	delete(spv.requestQueue, *blockHash)
	spv.receivedBlocks[*blockHash] = block
	spv.queueLock.Unlock()
}

func (sm *SyncManager) TxnReceived(txId *Uint256, txn *Txn) {
	spv.queueLock.Lock()
	delete(spv.requestQueue, *txId)
	spv.receivedTxns[*txId] = txn
	spv.queueLock.Unlock()
}

func (sm *SyncManager) RequestFinished() bool {
	spv.queueLock.RLock()
	defer spv.queueLock.RUnlock()

	return len(sm.requestQueue) == 0
}

func (sm *SyncManager) submitData() error {
	// Connect headers
	connectedHeaders, err := sm.connectHeaders()
	if err != nil {
		return err
	}

	// Save received blocks and txns to database
	err = sm.saveToDB(connectedHeaders)
	if err != nil {
		// If anything goes wrong, change a sync peer and restart
		log.Error(">>>>> Save headers failed, disconnect")
		sm.ChangeSyncPeerAndRestart()
		return err
	}

	// Continue syncing blocks
	sm.startSync()

	return nil
}

func (sm *SyncManager) connectHeaders() ([]Uint256, error) {
	headersCount := len(sm.receivedBlocks)
	connectedHeaders := make([]Uint256, headersCount)
	for {
		headersCount--
		header := sm.receivedBlocks[*sm.stopHash]
		connectedHeaders[headersCount] = *sm.stopHash

		// All headers connected, return connected headers in height ASC order
		if *sm.stopHash == *sm.startHash {
			return connectedHeaders, nil
		}

		previousHash := header.BlockHeader.Previous

		if _, ok := sm.receivedBlocks[previousHash]; !ok {
			log.Error(">>>>> Connect received headers failed, disconnect")
			sm.ChangeSyncPeerAndRestart()
			return nil, errors.New("Sync manager connect headers failed")
		}
		// Set stop hash to previous for next loop
		sm.stopHash = &previousHash
	}
}

func (sm *SyncManager) saveToDB(hashes []Uint256) error {
	// Get block txns and put header and txns into database in order,
	// for each block save txns first and save block header
	// if anything goes wrong, try to rollback and return error
	for _, hash := range hashes {
		header := sm.receivedBlocks[hash].BlockHeader
		txnHashes := sm.blockTxns[hash]
		txns := make([]tx.Transaction, len(txnHashes))
		for i, hash := range txnHashes {
			txn := sm.receivedTxns[hash]
			txns[i] = txn.Transaction
		}

		// Commit block data to blockchain
		err := spv.chain.CommitBlock(header, txns)
		if err != nil {
			return err
		}
	}
	return nil
}
