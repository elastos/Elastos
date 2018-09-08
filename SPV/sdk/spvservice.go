package sdk

import (
	"errors"
	"fmt"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/log"
	"github.com/elastos/Elastos.ELA.SPV/net"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA/bloom"
	"github.com/elastos/Elastos.ELA/core"
)

const (
	SendTxTimeout = time.Second * 10
)

// The SPV service implementation
type SPVServiceImpl struct {
	*net.ServerPeer
	syncManager *SyncManager
	chain       *BlockChain
	pendingTx   common.Uint256
	txAccept    chan *common.Uint256
	txReject    chan *msg.Reject
	config      SPVServiceConfig
}

// Create a instance of SPV service implementation.
func NewSPVServiceImpl(config SPVServiceConfig) (*SPVServiceImpl, error) {
	// Initialize blockchain
	chain, err := NewBlockchain(config.Foundation, config.HeaderStore)
	if err != nil {
		return nil, err
	}

	// Create SPV service instance
	service := &SPVServiceImpl{
		ServerPeer: config.Server,
		chain:      chain,
		config:     config,
	}

	// Create sync manager config
	syncConfig := SyncManageConfig{
		LocalHeight: chain.BestHeight,
		GetBlocks:   service.GetBlocks,
	}

	service.syncManager = NewSyncManager(syncConfig)

	// Set manage config
	service.SetConfig(net.PeerManageConfig{
		OnHandshake:     service.OnHandshake,
		OnPeerEstablish: service.OnPeerEstablish,
	})

	return service, nil
}

func (s *SPVServiceImpl) OnHandshake(v *msg.Version) error {
	if v.Services/OpenService&1 == 0 {
		return errors.New("SPV service not enabled on connected peer")
	}

	return nil
}

func (s *SPVServiceImpl) OnPeerEstablish(peer *net.Peer) {
	// Create spv peer config
	config := SPVPeerConfig{
		LocalHeight:   s.LocalHeight,
		OnInventory:   s.OnInventory,
		OnMerkleBlock: s.OnMerkleBlock,
		OnTx:          s.OnTx,
		OnNotFound:    s.OnNotFound,
		OnReject:      s.OnReject,
	}

	s.syncManager.AddNeighborPeer(NewSPVPeer(peer, config))

	// Load bloom filter
	doneChan := make(chan struct{})
	peer.QueueMessage(s.BloomFilter(), doneChan)
	<-doneChan
}

func (s *SPVServiceImpl) LocalHeight() uint32 {
	return uint32(s.ServerPeer.Height())
}

func (s *SPVServiceImpl) Start() {
	s.ServerPeer.Start()
	s.syncManager.start()
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
	s.Broadcast(BuildBloomFilter(s.config.GetFilterData()).GetFilterLoadMsg())
}

func (s *SPVServiceImpl) SendTransaction(tx core.Transaction) (*common.Uint256, error) {
	log.Debug()

	if s.GetNeighborCount() == 0 {
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
	s.Broadcast(msg.NewTx(&tx))
	// Query neighbors mempool see if transaction was successfully added to mempool
	s.Broadcast(new(msg.MemPool))

	// Wait for result
	timer := time.NewTimer(SendTxTimeout)
	select {
	case <-timer.C:
		finish()
		return nil, fmt.Errorf("Send transaction timeout")
	case <-s.txAccept:
		timer.Stop()
		finish()
		// commit unconfirmed transaction to db
		_, err := s.config.CommitTx(&tx, 0)
		return &s.pendingTx, err
	case msg := <-s.txReject:
		timer.Stop()
		finish()
		return nil, fmt.Errorf("Transaction rejected Code: %s, Reason: %s", msg.Code.String(), msg.Reason)
	}
}

func (s *SPVServiceImpl) GetBlocks() *msg.GetBlocks {
	// Get blocks returns a inventory message which contains block hashes
	locator := s.chain.GetBlockLocatorHashes()
	return msg.NewGetBlocks(locator, common.EmptyHash)
}

func (s *SPVServiceImpl) BloomFilter() *msg.FilterLoad {
	bloomFilter := BuildBloomFilter(s.config.GetFilterData())
	return bloomFilter.GetFilterLoadMsg()
}

func (s *SPVServiceImpl) OnInventory(peer *SPVPeer, m *msg.Inventory) error {
	getData := msg.NewGetData()

	for _, inv := range m.InvList {
		switch inv.Type {
		case msg.InvTypeBlock:
			// Filter duplicated block
			if s.chain.IsKnownHeader(&inv.Hash) {
				continue
			}

			// Kind of lame to send separate getData messages but this allows us
			// to take advantage of the timeout on the upper layer. Otherwise we
			// need separate timeout handling.
			inv.Type = msg.InvTypeFilteredBlock
			getData.AddInvVect(inv)
			if s.syncManager.IsSyncPeer(peer) {
				peer.blockQueue <- inv.Hash
			}

		case msg.InvTypeTx:
			if s.txAccept != nil && s.pendingTx.IsEqual(inv.Hash) {
				s.txAccept <- nil
				continue
			}
			getData.AddInvVect(inv)
			peer.EnqueueTx(inv.Hash)

		default:
			continue
		}
	}

	if len(getData.InvList) > 0 {
		peer.QueueMessage(getData)
	}
	return nil
}

func (s *SPVServiceImpl) OnMerkleBlock(peer *SPVPeer, mBlock *msg.MerkleBlock) error {
	blockHash := mBlock.Header.(*core.Header).Hash()

	// Merkleblock from sync peer
	if s.syncManager.IsSyncPeer(peer) {
		queueHash := <-peer.blockQueue
		if !blockHash.IsEqual(queueHash) {
			peer.Disconnect()
			return fmt.Errorf("peer %d is sending us blocks out of order", peer.ID())
		}
	}

	txIds, err := bloom.CheckMerkleBlock(*mBlock)
	if err != nil {
		return fmt.Errorf("invalid merkleblock received %s", err.Error())
	}

	dBlock := peer.EnqueueBlock(mBlock, txIds)
	if dBlock != nil {
		s.commitBlock(peer, dBlock)

		// Try continue sync progress
		s.syncManager.ContinueSync()

	}

	return nil
}

func (s *SPVServiceImpl) OnTx(peer *SPVPeer, msg *msg.Tx) error {
	tx := msg.Transaction.(*core.Transaction)

	obj, ok := peer.DequeueTx(tx)
	if ok {
		switch obj := obj.(type) {
		case *block:
			// commit block
			s.commitBlock(peer, obj)

			// Try continue sync progress
			s.syncManager.ContinueSync()

		case *core.Transaction:
			// commit unconfirmed transaction
			_, err := s.config.CommitTx(tx, 0)
			if err == nil {
				// Update bloom filter
				peer.SendMessage(s.BloomFilter())
			}

			return err
		}
	}

	return fmt.Errorf("Transaction not found in download queue %s", tx.Hash().String())
}

func (s *SPVServiceImpl) OnNotFound(peer *SPVPeer, notFound *msg.NotFound) error {
	for _, iv := range notFound.InvList {
		log.Warnf("Data not found type %s, hash %s", iv.Type.String(), iv.Hash.String())
	}
	return nil
}

func (s *SPVServiceImpl) OnReject(peer *SPVPeer, msg *msg.Reject) error {
	if s.pendingTx.IsEqual(msg.Hash); s.txReject != nil {
		s.txReject <- msg
		return nil
	}
	return fmt.Errorf("Received reject message from peer %d: Code: %s, Hash %s, Reason: %s",
		peer.ID(), msg.Code.String(), msg.Hash.String(), msg.Reason)
}

func (s *SPVServiceImpl) commitBlock(peer *SPVPeer, block *block) {
	header := block.Header.(*core.Header)
	newTip, reorgFrom, err := s.chain.CommitHeader(*header)
	if err != nil {
		log.Errorf("Commit header failed %s", err.Error())
		return
	}
	if !newTip {
		return
	}

	newHeight := s.chain.BestHeight()
	if reorgFrom > 0 {
		for i := reorgFrom; i > newHeight; i-- {
			if err = s.config.OnRollback(i); err != nil {
				log.Errorf("Rollback transaction at height %d failed %s", i, err.Error())
				return
			}
		}

		if !s.chain.IsSyncing() {
			s.syncManager.StartSync()
			return
		}
	}

	for _, tx := range block.txs {
		// Increase received transaction count
		peer.receivedTxs++

		falsePositive, err := s.config.CommitTx(tx, header.Height)
		if err != nil {
			log.Errorf("Commit transaction %s failed %s", tx.Hash().String(), err.Error())
			return
		}

		// Increase false positive count
		if falsePositive {
			peer.fPositives++
		}
	}

	// Refresh bloom filter if false positives meet target rate
	if peer.GetFalsePositiveRate() > FalsePositiveRate {
		// Reset false positives
		peer.ResetFalsePositives()

		// Update bloom filter
		peer.SendMessage(s.BloomFilter())
	}

	s.ServerPeer.SetHeight(uint64(newHeight))

	// Notify block committed
	go s.config.OnBlockCommitted(block.MerkleBlock, block.txs)
}
