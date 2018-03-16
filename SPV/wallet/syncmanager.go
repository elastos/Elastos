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
)

const (
	SyncReqTimeout = 15
	MaxRetryTimes  = 10
)

type SyncManager struct {
	sync.Mutex
	TxnMemCache

	blockLocator   []*Uint256
	startHash      *Uint256
	stopHash       *Uint256
	requestQueue   map[Uint256]time.Time
	blockTxns      map[Uint256][]Uint256
	receivedBlocks map[Uint256]*MerkleBlock
	receivedTxns   map[Uint256]*Txn
	retryTimes     int
}

func (sm *SyncManager) SyncBlocks() {
	sm.Lock()
	defer sm.Unlock()

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
	return bestPeer.Height() > uint64(spv.chain.Height())
}

func (sm *SyncManager) requestBlocks() {
	// Current blocks request was not finished
	if sm.blockLocator != nil {
		return
	}
	// Get sync peer
	syncPeer := spv.pm.GetSyncPeer()
	if syncPeer == nil {
		// If sync peer is nil at this point, that meas no peer connected
		return
	}
	// Request blocks returns a inventory message witch contains block hashes
	sm.blockLocator = spv.chain.GetBlockLocatorHashes()
	msg, err := NewBlocksReqMsg(sm.blockLocator, Uint256{})
	if err != nil {
		fmt.Println("Sync blocks new blocks request message failed, ", err)
		return
	}

	go syncPeer.Send(msg)
}

func (sm *SyncManager) HandleBlockInvMsg(inv *Inventory, peer *Peer) error {
	sm.Lock()
	defer sm.Unlock()

	if sm.blockLocator == nil || spv.pm.GetSyncPeer() == nil ||
		spv.pm.GetSyncPeer().ID() != peer.ID() {
		sm.changeSyncPeerAndRestart()
		return errors.New("Receive message from non sync peer, disconnect")
	}

	dataLen := len(inv.Data)
	if dataLen != int(inv.Count)*UINT256SIZE {
		return errors.New("Invalid block inventory data size")
	}

	for i := 0; i < dataLen; i += UINT256SIZE {
		var blockHash Uint256
		blockHash.Deserialize(bytes.NewReader(inv.Data[i:i+UINT256SIZE]))
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

func (sm *SyncManager) sendRequest(peer *Peer, msgType byte, hash Uint256) error {
	// Add to request queue
	sm.requestQueue[hash] = time.Now()

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
					sm.changeSyncPeerAndRestart()
					return
				}
				sm.retryTimes++
			}
		}
	}
}

func (sm *SyncManager) changeSyncPeerAndRestart() {
	sm.Lock()
	defer sm.Unlock()

	// Disconnect sync peer
	syncPeer := spv.pm.GetSyncPeer()
	spv.pm.DisconnectPeer(syncPeer)

	// Restart
	sm.startSync()
}

func (sm *SyncManager) inRequestQueue(hash Uint256) bool {
	_, ok := sm.requestQueue[hash]
	return ok
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
		sm.changeSyncPeerAndRestart()
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
			sm.changeSyncPeerAndRestart()
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
