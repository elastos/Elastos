package wallet

import (
	"errors"
	"fmt"
	"time"

	"SPVWallet/core"
	"SPVWallet/db"
	"SPVWallet/p2p"
	"SPVWallet/p2p/msg"
	tx "SPVWallet/core/transaction"
	"SPVWallet/log"
)

var spv *SPV

func InitSPV(walletId uint64) (*SPV, error) {
	var err error
	spv = new(SPV)
	spv.chain, err = db.NewBlockchain()
	if err != nil {
		return nil, err
	}
	spv.SyncManager = new(SyncManager)
	spv.pm = p2p.NewPeerManager(walletId)

	return spv, nil
}

type SPV struct {
	chain *db.Blockchain
	pm    *p2p.PeerManager
	*SyncManager
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
	// Check if handshake with itself
	if v.Nonce == spv.pm.Local().ID() {
		peer.Disconnect()
		return errors.New("Peer handshake with itself")
	}

	if v.Version < p2p.PeerVersion {
		peer.Disconnect()
		return errors.New(fmt.Sprint("To support SPV protocol, peer version must greater than ", p2p.PeerVersion))
	}

	if v.Services/p2p.ServiceSPV&1 == 0 {
		peer.Disconnect()
		return errors.New("SPV service not enabled on connected peer")
	}

	if peer.State() != p2p.INIT && peer.State() != p2p.HAND {
		return errors.New("Unknow status to received version")
	}

	// Remove duplicate peer connection
	knownPeer, ok := spv.pm.RemovePeer(v.Nonce)
	if ok {
		fmt.Println("Reconnect peer ", v.Nonce)
		knownPeer.Disconnect()
	}

	// Update peer info with version message
	peer.Update(v)

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
	msg, err := msg.NewAddrsMsg(addrs)
	if err != nil {
		return err
	}

	go peer.Send(msg)

	return nil
}

func (spv *SPV) OnInventory(peer *p2p.Peer, inv *msg.Inventory) error {
	switch inv.Type {
	case msg.TRANSACTION:
		// Do nothing, transaction inventory is not supported
	case msg.BLOCK:
		return spv.HandleBlockInvMsg(inv, peer)
	}
	return nil
}

func (spv *SPV) SendTransaction(txn *tx.Transaction) error {
	txnMsg, err := msg.NewTxnMsg(*txn)
	if err != nil {
		return err
	}

	// Broadcast transaction to connected peers
	spv.pm.Broadcast(txnMsg)

	return nil
}

func (spv *SPV) OnMerkleBlock(peer *p2p.Peer, block *msg.MerkleBlock) error {
	blockHash := block.BlockHeader.Hash()

	if spv.chain.IsKnownBlock(*blockHash) {
		return errors.New(fmt.Sprint("Received block that already known,", blockHash.String()))
	}

	err := spv.chain.CheckProofOfWork(&block.BlockHeader)
	if err != nil {
		return err
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

	} else if spv.blockLocator == nil || spv.pm.GetSyncPeer() == nil ||
		spv.pm.GetSyncPeer().ID() != peer.ID() || !spv.inRequestQueue(*blockHash) {

		spv.changeSyncPeerAndRestart()
		return errors.New("Receive message from non sync peer, disconnect")
	}

	txIds, err := CheckMerkleBlock(block)
	if err != nil {
		return err
	}

	delete(spv.requestQueue, *blockHash)
	spv.receivedBlocks[*blockHash] = block

	// If all blocks received and no more txn to request, submit received block and txn data
	if len(spv.requestQueue) == 0 && len(txIds) == 0 {
		return spv.submitData()
	}

	var blockTxns = make([]core.Uint256, len(txIds))
	for i, txId := range txIds {
		blockTxns[i] = *txId
		spv.sendRequest(peer, msg.TRANSACTION, *txId)
	}

	spv.blockTxns[*blockHash] = blockTxns

	return nil
}

func (spv *SPV) OnTxn(peer *p2p.Peer, txn *msg.Txn) error {
	txId := txn.Transaction.Hash()

	if !spv.chain.IsSyncing() {
		// Check if transaction already received
		if spv.TxnMemCache.Cached(*txId) {
			return errors.New("Received transaction already cached")
		}
		// Put txn into unconfirmed txnpool
		return spv.chain.CommitUnconfirmedTxn(txn.Transaction)

	} else if spv.blockLocator == nil || spv.pm.GetSyncPeer() == nil ||
		spv.pm.GetSyncPeer().ID() != peer.ID() || !spv.inRequestQueue(*txId) {

		spv.changeSyncPeerAndRestart()
		return errors.New("Receive message from non sync peer, disconnect")
	}

	delete(spv.requestQueue, *txId)
	spv.receivedTxns[*txId] = txn

	// All request finished, submit received block and txn data
	if len(spv.requestQueue) == 0 {
		return spv.submitData()
	}

	return nil
}

func (spv *SPV) OnNotFound(peer *p2p.Peer, msg *msg.NotFound) error {
	spv.changeSyncPeerAndRestart()
	return nil
}

func (spv *SPV) OnDisconnect(peer *p2p.Peer) {
	spv.pm.DisconnectPeer(peer)
}
