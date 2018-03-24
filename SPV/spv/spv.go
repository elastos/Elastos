package spv

import (
	"errors"
	"fmt"
	"time"

	"SPVWallet/db"
	"SPVWallet/log"
	"SPVWallet/p2p"
	"SPVWallet/p2p/msg"
	tx "SPVWallet/core/transaction"
)

var spv *SPV

func InitSPV(walletId uint64) (*SPV, error) {
	var err error
	spv = new(SPV)
	spv.chain, err = db.NewBlockchain()
	if err != nil {
		return nil, err
	}
	spv.chain.OnTxCommit = OnTxCommit
	spv.chain.OnBlockCommit = OnBlockCommit
	spv.chain.OnRollback = OnRollback
	spv.SyncManager = NewSyncManager()
	spv.pm = p2p.NewPeerManager(walletId)

	// Register message callback
	p2p.RegisterCallback(spv.handleMessage)
	return spv, nil
}

type SPV struct {
	*SyncManager
	chain *db.Blockchain
	pm    *p2p.PeerManager
}

func (spv *SPV) handleMessage(peer *p2p.Peer, message p2p.Message) {
	var err error
	switch message.(type) {
	case *msg.Version:
		err = spv.OnVersion(peer, message.(*msg.Version))
	case *msg.VerAck:
		err = spv.OnVerAck(peer, message.(*msg.VerAck))
	case *msg.Ping:
		err = spv.OnPing(peer, message.(*msg.Ping))
	case *msg.Pong:
		err = spv.OnPong(peer, message.(*msg.Pong))
	case *msg.AddrsReq:
		err = spv.OnAddrsReq(peer, message.(*msg.AddrsReq))
	case *msg.Addrs:
		err = spv.OnAddrs(peer, message.(*msg.Addrs))
	case *msg.Inventory:
		err = spv.OnInventory(peer, message.(*msg.Inventory))
	case *msg.MerkleBlock:
		err = spv.OnMerkleBlock(peer, message.(*msg.MerkleBlock))
	case *msg.Txn:
		err = spv.OnTxn(peer, message.(*msg.Txn))
	case *msg.NotFound:
		err = spv.OnNotFound(peer, message.(*msg.NotFound))
	}

	if err != nil {
		log.Error("Handle message error,", err)
	}
}

func (spv *SPV) Start() {
	spv.pm.Start()
	go spv.keepUpdate()
	log.Info("SPV service started...")
}

func (spv *SPV) Stop() {
	spv.chain.Close()
	log.Info("SPV service stopped...")
}

func (spv *SPV) BlockChain() *db.Blockchain {
	return spv.chain
}

func (spv *SPV) keepUpdate() {
	ticker := time.NewTicker(time.Second * p2p.InfoUpdateDuration)
	defer ticker.Stop()
	for range ticker.C {

		// Update peers info
		for _, peer := range spv.pm.ConnectedPeers() {
			if peer.State() == p2p.ESTABLISH {

				// Disconnect inactive peer
				if peer.LastActive().Before(
					time.Now().Add(-time.Second * p2p.InfoUpdateDuration * p2p.KeepAliveTimeout)) {
					log.Trace("SPV disconnect inactive peer,", peer)
					spv.pm.DisconnectPeer(peer)
					continue
				}

				// Send ping message to peer
				go peer.Send(msg.NewPing(spv.chain.Height()))
			}
		}

		// Keep connections
		spv.pm.ConnectPeers()

		// Keep synchronizing blocks
		spv.SyncBlocks()
	}
}

func (spv *SPV) OnVersion(peer *p2p.Peer, v *msg.Version) error {
	log.Info("SPV OnVersion")
	// Check if handshake with itself
	if v.Nonce == spv.pm.Local().ID() {
		log.Error(">>>> SPV disconnect peer, peer handshake with itself")
		spv.pm.DisconnectPeer(peer)
		spv.pm.OnDiscardAddr(peer.Addr().TCPAddr())
		return errors.New("Peer handshake with itself")
	}

	if peer.State() != p2p.INIT && peer.State() != p2p.HAND {
		log.Error("Unknow status to received version")
		return errors.New("Unknow status to received version")
	}

	// Remove duplicate peer connection
	knownPeer, ok := spv.pm.RemovePeer(v.Nonce)
	if ok {
		log.Trace("Reconnect peer ", v.Nonce)
		knownPeer.Disconnect()
	}

	log.Info("Is known peer:", ok)

	// Set peer info with version message
	peer.SetInfo(v)

	if v.Version < p2p.ProtocolVersion {
		log.Error("SPV disconnect peer, To support SPV protocol, peer version must greater than ", p2p.ProtocolVersion)
		spv.pm.DisconnectPeer(peer)
		return errors.New(fmt.Sprint("To support SPV protocol, peer version must greater than ", p2p.ProtocolVersion))
	}

	if v.Services/p2p.ServiceSPV&1 == 0 {
		log.Error("SPV disconnect peer, spv service not enabled on connected peer")
		spv.pm.DisconnectPeer(peer)
		return errors.New("SPV service not enabled on connected peer")
	}

	// Add to connected peer
	spv.pm.AddConnectedPeer(peer)

	var message p2p.Message
	if peer.State() == p2p.INIT {
		peer.SetState(p2p.HANDSHAKE)
		peer.SetHeight(uint64(spv.chain.Height()))
		message = p2p.NewVersion()
	} else if peer.State() == p2p.HAND {
		peer.SetState(p2p.HANDSHAKED)
		message = new(msg.VerAck)
	}

	go peer.Send(message)

	return nil
}

func (spv *SPV) OnVerAck(peer *p2p.Peer, va *msg.VerAck) error {
	if peer.State() != p2p.HANDSHAKE && peer.State() != p2p.HANDSHAKED {
		return errors.New("Unknow status to received verack")
	}

	if peer.State() == p2p.HANDSHAKE {
		go peer.Send(new(msg.VerAck))
	}

	peer.SetState(p2p.ESTABLISH)

	// Remove from connecting list
	spv.pm.RemoveFromConnectingList(peer)

	spv.broadcastFilterLoad()

	if spv.pm.NeedMorePeers() {
		go peer.Send(new(msg.AddrsReq))
	}

	return nil
}

func (spv *SPV) OnPing(peer *p2p.Peer, p *msg.Ping) error {
	peer.SetHeight(p.Height)
	go peer.Send(msg.NewPong(spv.chain.Height()))
	return nil
}

func (spv *SPV) OnPong(peer *p2p.Peer, p *msg.Pong) error {
	peer.SetHeight(p.Height)
	return nil
}

func (spv *SPV) OnAddrs(peer *p2p.Peer, addrs *msg.Addrs) error {
	for _, addr := range addrs.PeerAddrs {
		// Skip local peer
		if addr.ID == spv.pm.Local().ID() {
			continue
		}
		// Skip peer already connected
		if spv.pm.EstablishedPeer(addr.ID) {
			continue
		}
		// Skip invalid port
		if addr.Port == 0 {
			continue
		}
		// Handle new address
		if spv.pm.NeedMorePeers() {
			spv.pm.ConnectPeer(addr.TCPAddr())
		}
	}

	return nil
}

func (spv *SPV) OnAddrsReq(peer *p2p.Peer, req *msg.AddrsReq) error {
	addrs := spv.pm.RandPeerAddrs()
	go peer.Send(msg.NewAddrs(addrs))
	return nil
}

func (spv *SPV) OnInventory(peer *p2p.Peer, inv *msg.Inventory) error {
	switch inv.Type {
	case msg.TRANSACTION:
		// Do nothing, transaction inventory is not supported
	case msg.BLOCK:
		log.Info("SPV receive block inventory")
		return spv.HandleBlockInvMsg(peer, inv)
	}
	return nil
}

func (spv *SPV) broadcastFilterLoad() {
	// Broadcast filterload message to connected peers
	spv.pm.Broadcast(msg.NewFilterLoad(spv.chain.GetBloomFilter()))
}

func (spv *SPV) NotifyNewAddress(hash []byte) error {
	// Reload address filter to include new address
	spv.chain.Addrs().ReloadAddrFilter()
	spv.broadcastFilterLoad()
	return nil
}

func (spv *SPV) SendTransaction(tx tx.Transaction) error {
	// Broadcast transaction to connected peers
	spv.pm.Broadcast(msg.NewTxn(tx))
	return nil
}

func (spv *SPV) OnMerkleBlock(peer *p2p.Peer, block *msg.MerkleBlock) error {
	spv.dataLock.Lock()
	defer spv.dataLock.Unlock()

	blockHash := block.BlockHeader.Hash()
	log.Trace("Receive merkle block hash:", blockHash.String())

	if spv.chain.IsKnownBlock(*blockHash) {
		return errors.New(fmt.Sprint("Received block that already known,", blockHash.String()))
	}

	err := spv.chain.CheckProofOfWork(&block.BlockHeader)
	if err != nil {
		return err
	}

	if spv.chain.IsSyncing() && !spv.InRequestQueue(*blockHash) {
		// Put non syncing blocks into orphan pool
		spv.AddOrphanBlock(*blockHash, block)
		return nil
	}

	if !spv.chain.IsSyncing() {
		// Check if new block can connect to previous
		tip := spv.chain.ChainTip()
		// If block is already added, return
		if tip.Hash().IsEqual(blockHash) {
			return nil
		}
		// Meet an orphan block
		if !tip.Hash().IsEqual(&block.BlockHeader.Previous) {
			// Put non syncing blocks into orphan pool
			spv.AddOrphanBlock(*blockHash, block)
			return nil
		}
		// Set start hash and stop hash to the same block hash
		spv.startHash = blockHash
		spv.stopHash = blockHash

	} else if spv.blockLocator == nil || spv.pm.GetSyncPeer() == nil || spv.pm.GetSyncPeer().ID() != peer.ID() {

		log.Error("Receive message from non sync peer, disconnect")
		spv.ChangeSyncPeerAndRestart()
		return errors.New("Receive message from non sync peer, disconnect")
	}
	// Mark block as received
	spv.BlockReceived(blockHash, block)

	return spv.RequestBlockTxns(peer, block)
}

func (spv *SPV) OnTxn(peer *p2p.Peer, txn *msg.Txn) error {
	spv.dataLock.Lock()
	defer spv.dataLock.Unlock()

	txId := txn.Transaction.Hash()
	log.Trace("Receive transaction hash: ", txId.String())

	if spv.chain.IsSyncing() && !spv.InRequestQueue(*txId) {
		// Put non syncing txns into orphan pool
		spv.AddOrphanTxn(*txId, txn)
		return nil
	}

	if !spv.chain.IsSyncing() {
		// Check if transaction already received
		if spv.MemCache.TxCached(*txId) {
			return errors.New("Received transaction already cached")
		}
		// Put txn into unconfirmed txnpool
		fPositive, err := spv.chain.CommitUnconfirmedTxn(txn.Transaction)
		if err != nil {
			return err
		}
		if fPositive {
			spv.handleFPositive(1)
		}

	} else if spv.blockLocator == nil || spv.pm.GetSyncPeer() == nil || spv.pm.GetSyncPeer().ID() != peer.ID() {

		log.Error("Receive message from non sync peer, disconnect")
		spv.ChangeSyncPeerAndRestart()
		return errors.New("Receive message from non sync peer, disconnect")
	}

	spv.TxnReceived(txId, txn)

	// All request finished, submit received block and txn data
	if spv.RequestFinished() {
		log.Trace("Request finished submit data")
		return spv.CommitData()
	}

	return nil
}

func (spv *SPV) OnNotFound(peer *p2p.Peer, msg *msg.NotFound) error {
	log.Error("Receive not found message, disconnect")
	spv.ChangeSyncPeerAndRestart()
	return nil
}
