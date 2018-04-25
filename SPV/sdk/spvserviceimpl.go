package sdk

import (
	"errors"
	"fmt"
	"time"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/db"
	"github.com/elastos/Elastos.ELA.SPV/net"
	"github.com/elastos/Elastos.ELA.SPV/log"

	"github.com/elastos/Elastos.ELA/bloom"
	"github.com/elastos/Elastos.ELA/core"
	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

const (
	MaxRequests       = 100
	MaxFalsePositives = 7
)

// The SPV service implementation
type SPVServiceImpl struct {
	sync.Mutex
	SPVClient
	chain      *Blockchain
	queue      *RequestQueue
	getFilter  func() *bloom.Filter
	fPositives int
}

// Create a instance of SPV service implementation.
func NewSPVServiceImpl(client SPVClient, database db.DataStore, getBloomFilter func() *bloom.Filter) (*SPVServiceImpl, error) {
	var err error
	service := new(SPVServiceImpl)
	// Set spv client
	service.SPVClient = client
	// Initialize blockchain
	service.chain, err = NewBlockchain(database)
	if err != nil {
		return nil, err
	}
	// Initialize local peer height
	service.updateLocalHeight()

	// Set p2p message handler
	service.SPVClient.SetMessageHandler(service)

	// Initialize request queue
	service.queue = NewRequestQueue(MaxRequests, service)

	// Set get bloom filter method
	service.getFilter = getBloomFilter

	return service, nil
}

func (service *SPVServiceImpl) OnPeerEstablish(peer *net.Peer) {
	// Send filterload message
	peer.Send(service.getFilter().GetFilterLoadMsg())
}

func (service *SPVServiceImpl) Start() {
	service.SPVClient.Start()
	go service.keepUpdate()
	log.Info("SPV service started...")
}

func (service *SPVServiceImpl) Stop() {
	service.stopSyncing()
	service.chain.Close()
	log.Info("SPV service stopped...")
}

func (service *SPVServiceImpl) Blockchain() *Blockchain {
	return service.chain
}

func (service *SPVServiceImpl) BroadCastMessage(message p2p.Message) {
	service.PeerManager().Broadcast(message)
}

func (service *SPVServiceImpl) keepUpdate() {
	ticker := time.NewTicker(time.Second * net.InfoUpdateDuration)
	defer ticker.Stop()
	for range ticker.C {
		// Keep synchronizing blocks
		service.syncBlocks()
	}
}

func (service *SPVServiceImpl) needSync() bool {
	bestPeer := service.PeerManager().GetBestPeer()
	if bestPeer == nil { // no peers connected, return false
		return false
	}
	chainHeight := uint64(service.chain.Height())
	log.Info("Chain height:", chainHeight)
	log.Info("Best peer height:", bestPeer.Height())

	return bestPeer.Height() > chainHeight
}

func (service *SPVServiceImpl) syncBlocks() {
	// Check if blockchain need sync
	if service.needSync() {
		// Check if blockchain is in syncing state
		if service.chain.IsSyncing() || service.queue.IsRunning() {
			return
		}
		// Set blockchain state to syncing
		service.chain.SetChainState(SYNCING)
		// Request blocks
		service.requestBlocks()
	} else {
		service.stopSyncing()
	}
}

func (service *SPVServiceImpl) stopSyncing() {
	if service.chain.IsSyncing() {
		// Clear request queue
		service.queue.Clear()
		// Set blockchain state to waiting
		service.chain.SetChainState(WAITING)
		// Remove sync peer
		service.PeerManager().SetSyncPeer(nil)
	}
}

func (service *SPVServiceImpl) requestBlocks() {
	// Get sync peer
	syncPeer := service.PeerManager().GetSyncPeer()
	if syncPeer == nil {
		// If sync peer is nil at this point, that meas no peer connected
		fmt.Println("SyncManager no sync peer connected")
		return
	}
	// Request blocks returns a inventory message which contains block hashes
	request := msg.NewBlocksReq(service.chain.GetBlockLocatorHashes(), Uint256{})

	go syncPeer.Send(request)
}

func (service *SPVServiceImpl) changeSyncPeerAndRestart() {
	log.Debug("Change sync peer and restart")
	// Disconnect current sync peer
	syncPeer := service.PeerManager().GetSyncPeer()
	service.PeerManager().DisconnectPeer(syncPeer)

	service.stopSyncing()
	// Restart
	service.syncBlocks()
}

func (service *SPVServiceImpl) OnSendRequest(peer *net.Peer, reqType uint8, hash Uint256) {
	peer.Send(msg.NewDataReq(reqType, hash))
}

func (service *SPVServiceImpl) OnRequestError(err error) {
	service.Lock()
	defer service.Unlock()

	service.changeSyncPeerAndRestart()
}

func (service *SPVServiceImpl) OnRequestFinished(pool *FinishedReqPool) {
	service.Lock()
	defer service.Unlock()

	// By default, last pop from FinishedReqPool is the current, otherwise get chain tip as current
	var current = pool.LastPop()
	if current == nil {
		current = new(Uint256)
		*current = service.chain.ChainTip().Hash()
	}

	var fPositives int
	for request, ok := pool.Next(*current); ok; request, ok = pool.Next(request.Block.Header.Hash()) {
		// Try to commit next block
		reorg, fp, err := service.chain.CommitBlock(request.Block, request.Txs)
		if err != nil {
			fmt.Println(err)
			service.changeSyncPeerAndRestart()
			return
		}
		// Update local height after block committed
		service.updateLocalHeight()

		// If we meet a reorganize, restart sync process
		if reorg {
			log.Warn("service handle reorganize, restart sync")
			service.stopSyncing()
			service.syncBlocks()
			return
		}
		fPositives += fp
	}

	go service.handleFPositive(fPositives)
}

func (service *SPVServiceImpl) handleFPositive(fPositives int) {
	service.fPositives += fPositives
	if service.fPositives > MaxFalsePositives {
		// Broadcast filterload message to connected peers
		service.PeerManager().Broadcast(service.getFilter().GetFilterLoadMsg())
		service.fPositives = 0
	}
}

func (service *SPVServiceImpl) OnInventory(peer *net.Peer, inv *msg.Inventory) error {
	switch inv.Type {
	case p2p.TxData:
		// Do nothing, transaction inventory is not supported
	case p2p.BlockData:
		return service.HandleBlockInvMsg(peer, inv)
	}
	return nil
}

func (service *SPVServiceImpl) HandleBlockInvMsg(peer *net.Peer, inv *msg.Inventory) error {
	if !service.chain.IsSyncing() {
		peer.Disconnect()
		return errors.New("receive inventory message in non syncing mode")
	}

	// If no more blocks, return
	if len(inv.Hashes) == 0 {
		return nil
	}

	// Put hashes to request queue
	service.queue.PushHashes(peer, inv.Hashes)

	// Request more blocks
	locator := []*Uint256{inv.Hashes[len(inv.Hashes)-1]}
	go peer.Send(msg.NewBlocksReq(locator, Uint256{}))

	return nil
}

func (service *SPVServiceImpl) OnMerkleBlock(peer *net.Peer, block *bloom.MerkleBlock) error {
	blockHash := block.Header.Hash()
	log.Debug("Receive merkle block hash: ", blockHash.String())

	header := block.Header
	err := service.chain.CheckProofOfWork(header)
	if err != nil {
		return err
	}

	txIds, err := bloom.CheckMerkleBlock(*block)
	if err != nil {
		return errors.New("Invalid merkle block received: " + err.Error())
	}

	if service.chain.IsSyncing() { // When blockchain in syncing mode
		if service.PeerManager().GetSyncPeer() != nil && service.PeerManager().GetSyncPeer().ID() != peer.ID() {
			peer.Disconnect()
			return fmt.Errorf("receive message from non sync peer: %d\n", peer.ID())
		}

		// Add block to sync queue
		err = service.queue.OnBlockReceived(block, txIds)
		if err != nil {
			service.changeSyncPeerAndRestart()
			return err
		}
	} else {

		// Just request block transactions.
		// After transactions are received, the block will be put into finished blocks pool
		service.queue.StartBlockTxsRequest(peer, block, txIds)
	}

	return nil
}

func (service *SPVServiceImpl) OnTxn(peer *net.Peer, txn *core.Transaction) error {
	log.Debug("Receive transaction hash: ", txn.Hash().String())

	if service.chain.IsSyncing() && service.PeerManager().GetSyncPeer() != nil &&
		service.PeerManager().GetSyncPeer().ID() != peer.ID() {

		peer.Disconnect()
		return fmt.Errorf("receive message from non sync peer: %d\n", peer.ID())
	}

	if service.chain.IsSyncing() || service.queue.IsRunning() {
		// Add transaction to queue
		err := service.queue.OnTxReceived(txn)
		if err != nil {
			service.changeSyncPeerAndRestart()
			return err
		}
	} else {
		isFPositive, err := service.chain.CommitTx(*txn)
		if err != nil {
			return err
		}

		if isFPositive {
			service.handleFPositive(1)
		}
	}

	return nil
}

func (service *SPVServiceImpl) OnNotFound(peer *net.Peer, msg *msg.NotFound) error {
	log.Debug("Receive not found: ", msg.Hash.String())

	service.changeSyncPeerAndRestart()
	return nil
}

// Update local peer height with current chain height
func (service *SPVServiceImpl) updateLocalHeight() {
	service.PeerManager().Local().SetHeight(uint64(service.chain.Height()))
}
