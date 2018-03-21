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
	"sync"
)

var spv *SPV

func InitSPV(walletId uint64) (*SPV, error) {
	var err error
	spv = new(SPV)
	spv.msgLock = new(sync.Mutex)
	spv.chain, err = db.NewBlockchain()
	if err != nil {
		return nil, err
	}
	spv.SyncManager = NewSyncManager()
	spv.pm = p2p.NewPeerManager(walletId)

	// Init listeners
	p2p.SetListeners(&p2p.Listeners{
		OnVersion:     spv.OnVersion,
		OnVerAck:      spv.OnVerAck,
		OnPing:        spv.OnPing,
		OnPong:        spv.OnPong,
		OnAddrs:       spv.OnAddrs,
		OnAddrsReq:    spv.OnAddrsReq,
		OnInventory:   spv.OnInventory,
		OnMerkleBlock: spv.OnMerkleBlock,
		OnTxn:         spv.OnTxn,
		OnNotFound:    spv.OnNotFound,
		OnDisconnect:  spv.OnDisconnect,
	})
	return spv, nil
}

type SPV struct {
	*SyncManager
	msgLock *sync.Mutex
	chain   *db.Blockchain
	pm      *p2p.PeerManager
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
				msg, err := msg.NewPingMsg(spv.chain.Height())
				if err != nil {
					fmt.Println("Failed to build ping message, ", err)
					return
				}
				go peer.Send(msg)
			}
		}

		// Keep connections
		spv.pm.ConnectPeers()

		// Keep synchronizing blocks
		spv.SyncBlocks()
	}
}

func (spv *SPV) OnVersion(peer *p2p.Peer, v *msg.Version) error {
	spv.msgLock.Lock()
	defer spv.msgLock.Unlock()

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

	var buf []byte
	if peer.State() == p2p.INIT {
		peer.SetState(p2p.HANDSHAKE)
		peer.SetHeight(uint64(spv.chain.Height()))
		buf, _ = msg.NewVersionMsg(p2p.NewVersionData(peer))
	} else if peer.State() == p2p.HAND {
		peer.SetState(p2p.HANDSHAKED)
		buf, _ = msg.NewVerAckMsg()
	}

	go peer.Send(buf)

	return nil
}

func (spv *SPV) OnVerAck(peer *p2p.Peer, va *msg.VerAck) error {
	spv.msgLock.Lock()
	defer spv.msgLock.Unlock()

	if peer.State() != p2p.HANDSHAKE && peer.State() != p2p.HANDSHAKED {
		return errors.New("Unknow status to received verack")
	}

	if peer.State() == p2p.HANDSHAKE {
		msg, _ := msg.NewVerAckMsg()
		go peer.Send(msg)
	}

	peer.SetState(p2p.ESTABLISH)

	// Remove from connecting list
	spv.pm.RemoveFromConnectingList(peer)

	buf, _ := msg.NewFilterLoadMsg(spv.chain.GetFilter())
	go peer.Send(buf)

	if spv.pm.NeedMorePeers() {
		buf, _ := msg.NewAddrsReqMsg()
		go peer.Send(buf)
	}

	return nil
}

func (spv *SPV) OnPing(peer *p2p.Peer, p *msg.Ping) error {
	spv.msgLock.Lock()
	defer spv.msgLock.Unlock()

	peer.SetHeight(p.Height)

	msg, err := msg.NewPongMsg(spv.chain.Height())
	if err != nil {
		fmt.Println("Failed to build pong message")
		return err
	}

	go peer.Send(msg)

	return nil
}

func (spv *SPV) OnPong(peer *p2p.Peer, p *msg.Pong) error {
	spv.msgLock.Lock()
	defer spv.msgLock.Unlock()

	peer.SetHeight(p.Height)
	return nil
}

func (spv *SPV) OnAddrs(peer *p2p.Peer, addrs *msg.Addrs) error {
	spv.msgLock.Lock()
	defer spv.msgLock.Unlock()

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
	spv.msgLock.Lock()
	defer spv.msgLock.Unlock()

	addrs := spv.pm.RandPeerAddrs()
	msg, err := msg.NewAddrsMsg(addrs)
	if err != nil {
		return err
	}

	go peer.Send(msg)

	return nil
}

func (spv *SPV) OnInventory(peer *p2p.Peer, inv *msg.Inventory) error {
	spv.msgLock.Lock()
	defer spv.msgLock.Unlock()

	switch inv.Type {
	case msg.TRANSACTION:
		// Do nothing, transaction inventory is not supported
	case msg.BLOCK:
		log.Info("SPV receive block inventory")
		return spv.HandleBlockInvMsg(peer, inv)
	}
	return nil
}

func (spv *SPV) SendTransaction(txn tx.Transaction) error {
	txnMsg, err := msg.NewTxnMsg(txn)
	if err != nil {
		return err
	}

	// Broadcast transaction to connected peers
	spv.pm.Broadcast(txnMsg)

	return nil
}

func (spv *SPV) OnMerkleBlock(peer *p2p.Peer, block *msg.MerkleBlock) error {
	spv.msgLock.Lock()
	defer spv.msgLock.Unlock()

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
		log.Trace("Add block to orphan pool hash:", blockHash.String())
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
		// Maybe need sync or meet a reorganize condition, restart sync
		if !tip.Hash().IsEqual(&block.BlockHeader.Previous) {
			spv.startSync()
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

	spv.BlockReceived(blockHash, block)

	return spv.RequestBlockTxns(peer, block)
}

func (spv *SPV) OnTxn(peer *p2p.Peer, txn *msg.Txn) error {
	spv.msgLock.Lock()
	defer spv.msgLock.Unlock()

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
		return spv.chain.CommitUnconfirmedTxn(txn.Transaction)

	} else if spv.blockLocator == nil || spv.pm.GetSyncPeer() == nil || spv.pm.GetSyncPeer().ID() != peer.ID() {

		log.Error("Receive message from non sync peer, disconnect")
		spv.ChangeSyncPeerAndRestart()
		return errors.New("Receive message from non sync peer, disconnect")
	}

	spv.TxnReceived(txId, txn)

	// All request finished, submit received block and txn data
	if spv.RequestFinished() {
		log.Trace("Request finished submit data")
		return spv.SubmitData()
	}

	return nil
}

func (spv *SPV) OnNotFound(peer *p2p.Peer, msg *msg.NotFound) error {
	spv.msgLock.Lock()
	defer spv.msgLock.Unlock()

	log.Error("Receive not found message, disconnect")
	spv.ChangeSyncPeerAndRestart()
	return nil
}

func (spv *SPV) OnDisconnect(peer *p2p.Peer) {
	spv.pm.DisconnectPeer(peer)
}
