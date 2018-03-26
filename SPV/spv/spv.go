package spv

import (
	"errors"
	"fmt"
	"time"
	"strings"

	"SPVWallet/config"
	tx "SPVWallet/core/transaction"
	"SPVWallet/db"
	"SPVWallet/log"
	"SPVWallet/p2p"
	"SPVWallet/msg"
)

var spv *SPV

func InitSPV(clientId uint64) (*SPV, error) {
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

	// Set Magic number of the P2P network
	p2p.Magic = config.Config().Magic
	// Convert seed addresses to SPVServerPort according to the SPV protocol
	seeds := toSPVAddr(config.Config().SeedList)
	// Create peer manager of the P2P network
	spv.pm = p2p.InitPeerManager(clientId, SPVClientPort, seeds)

	// Register callbacks
	p2p.OnMakeMessage(spv.makeMessage)
	p2p.OnHandleVersion(spv.handleVersion)
	p2p.OnPeerConnected(spv.peerConnected)
	p2p.OnHandleMessage(spv.handleMessage)
	return spv, nil
}

func toSPVAddr(seeds []string) []string {
	var addrs = make([]string, len(seeds))
	for i, seed := range seeds {
		portIndex := strings.LastIndex(seed, ":")
		if portIndex > 0 {
			addrs[i] = fmt.Sprint(string([]byte(seed)[:portIndex]), ":", SPVServerPort)
		} else {
			addrs[i] = fmt.Sprint(seed, ":", SPVServerPort)
		}
	}
	return addrs
}

type SPV struct {
	*SyncManager
	chain *db.Blockchain
	pm    *p2p.PeerManager
}

func (spv *SPV) makeMessage(cmd string) (message p2p.Message, err error) {
	switch cmd {
	case "ping":
		message = new(msg.Ping)
	case "pong":
		message = new(msg.Pong)
	case "inv":
		message = new(msg.Inventory)
	case "tx":
		message = new(msg.Txn)
	case "merkleblock":
		message = new(msg.MerkleBlock)
	case "notfound":
		message = new(msg.NotFound)
	default:
		return nil, errors.New("Received unsupported message, CMD " + cmd)
	}
	return message, nil
}

func (spv *SPV) handleVersion(v *p2p.Version) error {

	if v.Version < p2p.ProtocolVersion {
		log.Error("SPV disconnect peer, To support SPV protocol, peer version must greater than ", p2p.ProtocolVersion)
		return errors.New(fmt.Sprint("To support SPV protocol, peer version must greater than ", p2p.ProtocolVersion))
	}

	if v.Services/ServiveSPV&1 == 0 {
		log.Error("SPV disconnect peer, spv service not enabled on connected peer")
		return errors.New("SPV service not enabled on connected peer")
	}

	return nil
}

func (spv *SPV) peerConnected(peer *p2p.Peer) {
	// Send filterload message
	peer.Send(msg.NewFilterLoad(spv.chain.GetBloomFilter()))
}

func (spv *SPV) handleMessage(peer *p2p.Peer, message p2p.Message) error {
	switch msg := message.(type) {
	case *msg.Ping:
		return spv.OnPing(peer, msg)
	case *msg.Pong:
		return spv.OnPong(peer, msg)
	case *msg.Inventory:
		return spv.OnInventory(peer, msg)
	case *msg.MerkleBlock:
		return spv.OnMerkleBlock(peer, msg)
	case *msg.Txn:
		return spv.OnTxn(peer, msg)
	case *msg.NotFound:
		return spv.OnNotFound(peer, msg)
	default:
		return errors.New("unknown handle message type")
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

func (spv *SPV) OnPing(peer *p2p.Peer, p *msg.Ping) error {
	peer.SetHeight(p.Height)
	go peer.Send(msg.NewPong(spv.chain.Height()))
	return nil
}

func (spv *SPV) OnPong(peer *p2p.Peer, p *msg.Pong) error {
	peer.SetHeight(p.Height)
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

func (spv *SPV) NotifyNewAddress(hash []byte) error {
	// Reload address filter to include new address
	spv.chain.Addrs().ReloadAddrFilter()
	// Broadcast filterload message to connected peers
	spv.pm.Broadcast(msg.NewFilterLoad(spv.chain.GetBloomFilter()))
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

func (spv *SPV) updateLocalHeight() {
	spv.pm.Local().SetHeight(uint64(spv.chain.Height()))
}
