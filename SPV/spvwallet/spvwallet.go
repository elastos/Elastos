package spvwallet

import (
	"bytes"
	"errors"
	"fmt"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/bloom"
	. "github.com/elastos/Elastos.ELA.SPV/common"
	"github.com/elastos/Elastos.ELA.SPV/core"
	tx "github.com/elastos/Elastos.ELA.SPV/core/transaction"
	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/log"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/db"
	"github.com/elastos/Elastos.ELA.SPV/msg"
	"github.com/elastos/Elastos.ELA.SPV/p2p"
	"sync"
)

const (
	MaxRequests       = 100
	MaxFalsePositives = 7
)

func Init(clientId uint64, seeds []string) (*SPVWallet, error) {
	var err error
	wallet := new(SPVWallet)
	// Initialize blockchain
	wallet.chain, err = NewBlockchain()
	if err != nil {
		return nil, err
	}

	// Initialize P2P network client
	client, err := sdk.GetSPVClient(sdk.TypeMainNet, clientId, seeds)
	if err != nil {
		return nil, err
	}
	// Set p2p message handler
	client.SetMessageHandler(wallet)

	wallet.SPVClient = client

	// Initialize request queue
	wallet.queue = NewRequestQueue(MaxRequests, wallet)

	wallet.chain.OnBlockCommitted = OnBlockCommit

	return wallet, nil
}

type SPVWallet struct {
	sync.Mutex
	sdk.SPVClient
	chain      *Blockchain
	queue      *RequestQueue
	fPositives int
}

func (wallet *SPVWallet) OnPeerEstablish(peer *p2p.Peer) {
	// Send filterload message
	peer.Send(wallet.chain.GetBloomFilter().GetFilterLoadMsg())
}

func (wallet *SPVWallet) Start() {
	wallet.SPVClient.Start()
	go wallet.keepUpdate()
	log.Info("SPV service started...")
}

func (wallet *SPVWallet) Stop() {
	wallet.stopSyncing()
	wallet.chain.Close()
	log.Info("SPV service stopped...")
}

func (wallet *SPVWallet) BlockChain() *Blockchain {
	return wallet.chain
}

func (wallet *SPVWallet) keepUpdate() {
	ticker := time.NewTicker(time.Second * p2p.InfoUpdateDuration)
	defer ticker.Stop()
	for range ticker.C {
		// Keep synchronizing blocks
		wallet.syncBlocks()
	}
}

func (wallet *SPVWallet) needSync() bool {
	bestPeer := wallet.PeerManager().GetBestPeer()
	if bestPeer == nil { // no peers connected, return false
		return false
	}
	chainHeight := uint64(wallet.chain.Height())
	log.Info("Chain height:", chainHeight)
	log.Info("Best peer height:", bestPeer.Height())

	return bestPeer.Height() > chainHeight
}

func (wallet *SPVWallet) syncBlocks() {
	// Check if blockchain need sync
	if wallet.needSync() {
		// Check if blockchain is in syncing state
		if wallet.chain.IsSyncing() || wallet.queue.IsRunning() {
			return
		}
		// Set blockchain state to syncing
		wallet.chain.SetChainState(SYNCING)
		// Request blocks
		wallet.requestBlocks()
	} else {
		wallet.stopSyncing()
	}
}

func (wallet *SPVWallet) stopSyncing() {
	if wallet.chain.IsSyncing() {
		// Clear request queue
		wallet.queue.Clear()
		// Set blockchain state to waiting
		wallet.chain.SetChainState(WAITING)
		// Remove sync peer
		wallet.PeerManager().SetSyncPeer(nil)
	}
}

func (wallet *SPVWallet) requestBlocks() {
	// Get sync peer
	syncPeer := wallet.PeerManager().GetSyncPeer()
	if syncPeer == nil {
		// If sync peer is nil at this point, that meas no peer connected
		log.Warn("SyncManager no sync peer connected")
		return
	}
	// Request blocks returns a inventory message which contains block hashes
	request := wallet.NewBlocksReq(wallet.chain.GetBlockLocatorHashes(), Uint256{})

	go syncPeer.Send(request)
}

func (wallet *SPVWallet) changeSyncPeerAndRestart() {
	// Disconnect current sync peer
	syncPeer := wallet.PeerManager().GetSyncPeer()
	wallet.PeerManager().DisconnectPeer(syncPeer)

	wallet.stopSyncing()
	// Restart
	wallet.syncBlocks()
}

func (wallet *SPVWallet) OnSendRequest(peer *p2p.Peer, reqType uint8, hash Uint256) {
	name := "BLOCK"
	if reqType == sdk.TRANSACTION {
		name = "TRANSACTION"
	}
	log.Debug("Send ", name, " request hash: ", hash.String())
	peer.Send(wallet.NewDataReq(reqType, hash))
}

func (wallet *SPVWallet) OnRequestError(err error) {
	wallet.Lock()
	defer wallet.Unlock()

	wallet.changeSyncPeerAndRestart()
}

func (wallet *SPVWallet) OnRequestFinished(pool *FinishedReqPool) {
	wallet.Lock()
	defer wallet.Unlock()

	// By default, last pop from FinishedReqPool is the current, otherwise get chain tip as current
	var current = pool.lastPop
	if current == nil {
		current = wallet.chain.ChainTip().Hash()
	}

	var fPositives int
	for request, ok := pool.Next(*current); ok; request, ok = pool.Next(*request.block.BlockHeader.Hash()) {
		// Try to commit next block
		reorg, fp, err := wallet.chain.CommitBlock(
			request.block.BlockHeader, GetProof(request.block), request.receivedTxs)
		if err != nil {
			log.Error(err)
			wallet.changeSyncPeerAndRestart()
			return
		}
		// Update local height after block committed
		wallet.updateLocalHeight()

		// If we meet a reorganize, restart sync process
		if reorg {
			log.Warn("Wallet handle reorganize, restart sync")
			wallet.stopSyncing()
			wallet.syncBlocks()
			return
		}
		fPositives += fp
	}

	go wallet.handleFPositive(fPositives)
}

func (wallet *SPVWallet) handleFPositive(fPositives int) {
	wallet.fPositives += fPositives
	if wallet.fPositives > MaxFalsePositives {
		// Broadcast filterload message to connected peers
		wallet.PeerManager().Broadcast(wallet.chain.GetBloomFilter().GetFilterLoadMsg())
		wallet.fPositives = 0
	}
}

func (wallet *SPVWallet) OnInventory(peer *p2p.Peer, inv *msg.Inventory) error {
	switch inv.Type {
	case sdk.TRANSACTION:
		// Do nothing, transaction inventory is not supported
	case sdk.BLOCK:
		return wallet.HandleBlockInvMsg(peer, inv)
	}
	return nil
}

func (wallet *SPVWallet) HandleBlockInvMsg(peer *p2p.Peer, inv *msg.Inventory) error {
	log.Debug("Receive inventory hash count: ", inv.Count)

	if !wallet.chain.IsSyncing() {
		peer.Disconnect()
		return errors.New("receive inventory message in non syncing mode")
	}

	// If no more blocks, return
	if inv.Count == 0 {
		return nil
	}

	dataLen := len(inv.Data)
	if dataLen != int(inv.Count)*UINT256SIZE {

		wallet.changeSyncPeerAndRestart()
		return fmt.Errorf("invalid block inventory data size: %d\n", dataLen)
	}

	var hashes = make([]Uint256, 0, inv.Count)
	for i := 0; i < dataLen; i += UINT256SIZE {
		var blockHash Uint256
		err := blockHash.Deserialize(bytes.NewReader(inv.Data[i:i+UINT256SIZE]))
		if err != nil {
			wallet.changeSyncPeerAndRestart()
			return fmt.Errorf("deserialize block hash error %s\n", err.Error())
		}
		hashes = append(hashes, blockHash)
	}

	// Put hashes to request queue
	wallet.queue.PushHashes(peer, hashes)

	// Request more blocks
	locator := []*Uint256{&hashes[len(hashes)-1]}
	request := wallet.NewBlocksReq(locator, Uint256{})

	go peer.Send(request)

	return nil
}

func (wallet *SPVWallet) OnMerkleBlock(peer *p2p.Peer, block *bloom.MerkleBlock) error {
	blockHash := block.BlockHeader.Hash()
	log.Debug("Receive merkle block hash: ", blockHash.String())

	err := wallet.chain.CheckProofOfWork(&block.BlockHeader)
	if err != nil {
		return err
	}

	txIds, err := bloom.CheckMerkleBlock(*block)
	if err != nil {
		return errors.New("Invalid merkle block received: " + err.Error())
	}

	if wallet.chain.IsSyncing() { // When blockchain in syncing mode
		if wallet.PeerManager().GetSyncPeer() != nil && wallet.PeerManager().GetSyncPeer().ID() != peer.ID() {
			peer.Disconnect()
			return fmt.Errorf("receive message from non sync peer: %d\n", peer.ID())
		}

		// Add block to sync queue
		err = wallet.queue.OnBlockReceived(block, txIds)
		if err != nil {
			wallet.changeSyncPeerAndRestart()
			return err
		}
	} else {

		// Just request block transactions.
		// After transactions are received, the block will be put into finished blocks pool
		wallet.queue.StartBlockTxsRequest(peer, block, txIds)
	}

	return nil
}

func (wallet *SPVWallet) OnTxn(peer *p2p.Peer, txn *msg.Txn) error {
	log.Debug("Receive transaction hash: ", txn.Hash().String())

	if wallet.chain.IsSyncing() && wallet.PeerManager().GetSyncPeer() != nil &&
		wallet.PeerManager().GetSyncPeer().ID() != peer.ID() {

		peer.Disconnect()
		return fmt.Errorf("receive message from non sync peer: %d\n", peer.ID())
	}

	if wallet.chain.IsSyncing() || wallet.queue.IsRunning() {
		// Add transaction to queue
		err := wallet.queue.OnTxReceived(&txn.Transaction)
		if err != nil {
			wallet.changeSyncPeerAndRestart()
			return err
		}
	} else {
		isFPositive, err := wallet.chain.CommitUnconfirmedTxn(txn.Transaction)
		if err != nil {
			return err
		}

		if isFPositive {
			wallet.handleFPositive(1)
		}
	}

	return nil
}

func (wallet *SPVWallet) OnNotFound(peer *p2p.Peer, msg *msg.NotFound) error {
	wallet.changeSyncPeerAndRestart()
	return nil
}

func (wallet *SPVWallet) updateLocalHeight() {
	wallet.PeerManager().Local().SetHeight(uint64(wallet.chain.Height()))
}

func (wallet *SPVWallet) NotifyNewAddress(hash []byte) error {
	// Reload address filter to include new address
	wallet.chain.Addrs().ReloadAddrFilter()
	// Broadcast filterload message to connected peers
	wallet.PeerManager().Broadcast(wallet.chain.GetBloomFilter().GetFilterLoadMsg())
	return nil
}

func (wallet *SPVWallet) SendTransaction(tx tx.Transaction) error {
	// Broadcast transaction to connected peers
	wallet.PeerManager().Broadcast(wallet.NewTxn(tx))
	return nil
}

func (wallet *SPVWallet) SetOnBlockCommitListener(listener func(core.Header, db.Proof, []tx.Transaction)) {
	if listener == nil {
		return
	}
	wallet.chain.OnBlockCommitted = listener
}

func OnBlockCommit(core.Header, db.Proof, []tx.Transaction) {}

func GetProof(msg bloom.MerkleBlock) db.Proof {
	return db.Proof{
		BlockHash:    *msg.BlockHeader.Hash(),
		Height:       msg.BlockHeader.Height,
		Transactions: msg.Transactions,
		Hashes:       msg.Hashes,
		Flags:        msg.Flags,
	}
}
