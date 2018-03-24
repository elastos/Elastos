package spv

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
	SyncReqTimeout    = 15
	MaxRetryTimes     = 10
	MaxFalsePositives = 7
)

type SyncManager struct {
	*MemCache

	dataLock       sync.RWMutex
	blockLocator   []*Uint256
	startHash      *Uint256
	stopHash       *Uint256
	queueLock      sync.RWMutex
	requestQueue   map[Uint256]time.Time
	blockTxns      map[Uint256][]Uint256
	receivedBlocks map[Uint256]*MerkleBlock
	receivedTxns   map[Uint256]*Txn
	retryTimes     int
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
	// Clear data first
	sm.clear()
	// Check if blockchain need sync
	if sm.needSync() {
		// Set blockchain state to syncing
		spv.chain.SetChainState(db.SYNCING)
		// Request blocks
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

	needSync := bestPeer.Height() > uint64(spv.chain.Height())

	log.Trace("Need Syn:", needSync)
	return needSync
}

func (sm *SyncManager) requestBlocks() {
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

	go syncPeer.Send(NewBlocksReq(sm.blockLocator, Uint256{}))
}

func (sm *SyncManager) HandleBlockInvMsg(peer *Peer, inv *Inventory) error {
	log.Trace(">>>> Handle inv msg data count:", inv.Count, ", length:", len(inv.Data))

	if sm.blockLocator == nil || spv.pm.GetSyncPeer() == nil || spv.pm.GetSyncPeer().ID() != peer.ID() {

		sm.ChangeSyncPeerAndRestart()
		return errors.New("Receive message from non sync peer, disconnect")
	}

	dataLen := len(inv.Data)
	if dataLen != int(inv.Count)*UINT256SIZE {
		log.Error("Invalid block inventory data size:", dataLen)
		sm.ChangeSyncPeerAndRestart()
		return errors.New(fmt.Sprint("Invalid block inventory data size:", dataLen))
	}

	var reqList []Uint256
	for i := 0; i < dataLen; i += UINT256SIZE {
		var blockHash Uint256
		err := blockHash.Deserialize(bytes.NewReader(inv.Data[i:i+UINT256SIZE]))
		if err != nil {
			log.Error("Deserialize block hash error,", err)
			sm.ChangeSyncPeerAndRestart()
			return err
		}
		// Add block hashes to request queue
		sm.addToRequestQueue(blockHash)

		// Save start and stop block for connect headers method
		if sm.startHash == nil {
			sm.startHash = &blockHash
		}
		sm.stopHash = &blockHash

		log.Trace("Add block request:", blockHash.String())
		reqList = append(reqList, blockHash)
	}

	// Send request message
	for _, hash := range reqList {
		// Check if block already in orphan pool
		if block, ok := spv.IsOrphanBlock(hash); ok {
			sm.BlockReceived(&hash, block)
			err := sm.RequestBlockTxns(peer, block)
			if err != nil {
				return err
			}
		}
		sm.sendRequest(peer, BLOCK, hash)
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
		log.Trace("Request finished submit data")
		return sm.CommitData()
	}

	var blockTxns = make([]Uint256, len(txIds))
	for i, txId := range txIds {
		blockTxns[i] = *txId
		log.Trace("Add transaction request:", txId.String())
		sm.addToRequestQueue(*txId)
	}
	sm.blockTxns[*block.BlockHeader.Hash()] = blockTxns

	for _, txId := range blockTxns {
		if txn, ok := sm.IsOrphanTxn(txId); ok {
			sm.TxnReceived(&txId, txn)
		} else {
			sm.sendRequest(peer, TRANSACTION, txId)
		}
	}

	return nil
}

func (sm *SyncManager) addToRequestQueue(hash Uint256) {
	// Add to request queue
	sm.queueLock.Lock()
	sm.requestQueue[hash] = time.Now()
	sm.queueLock.Unlock()
}

func (sm *SyncManager) sendRequest(peer *Peer, msgType byte, hash Uint256) error {
	go peer.Send(NewDataReq(msgType, hash))
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
					log.Error("Request timeout, disconnect")
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
	log.Trace("SyncManager change sync peer, disconnect ", syncPeer)
	spv.pm.DisconnectPeer(syncPeer)

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

func (sm *SyncManager) BlockReceived(blockHash *Uint256, block *MerkleBlock) {
	sm.queueLock.Lock()
	delete(spv.requestQueue, *blockHash)
	spv.receivedBlocks[*blockHash] = block
	sm.queueLock.Unlock()
}

func (sm *SyncManager) TxnReceived(txId *Uint256, txn *Txn) {
	sm.queueLock.Lock()
	delete(spv.requestQueue, *txId)
	spv.receivedTxns[*txId] = txn
	sm.queueLock.Unlock()
}

func (sm *SyncManager) RequestFinished() bool {
	sm.queueLock.RLock()
	defer sm.queueLock.RUnlock()

	finished := len(sm.requestQueue) == 0
	log.Trace("Block request finished: ", finished)
	return finished
}

func (sm *SyncManager) CommitData() error {
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
	// Continue syncing blocks
	sm.startSync()

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
		txnHashes := sm.blockTxns[hash]
		txns := make([]tx.Transaction, len(txnHashes))
		for i, hash := range txnHashes {
			txn := sm.receivedTxns[hash]
			txns[i] = txn.Transaction
		}

		// Commit block data to blockchain
		fPositives, err := spv.chain.CommitBlock(header, block.Proof, txns)
		if err != nil {
			return err
		}
		sm.handleFPositive(fPositives)
	}
	return nil
}

func (sm *SyncManager) handleFPositive(fPositives int) {
	sm.fPositives += fPositives
	if sm.fPositives > MaxFalsePositives {
		spv.broadcastFilterLoad()
		sm.fPositives = 0
	}
}
