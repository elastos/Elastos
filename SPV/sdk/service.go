package sdk

import (
	"fmt"
	"os"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/blockchain"
	speer "github.com/elastos/Elastos.ELA.SPV/peer"
	"github.com/elastos/Elastos.ELA.SPV/sync"
	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/elanet/pact"
	"github.com/elastos/Elastos.ELA/p2p"
	"github.com/elastos/Elastos.ELA/p2p/msg"
	"github.com/elastos/Elastos.ELA/p2p/peer"
	"github.com/elastos/Elastos.ELA/p2p/server"
)

const (
	defaultDataDir        = "./"
	defaultMaxPeers       = 25
	txExpireTime          = time.Hour * 24
	txRebroadcastDuration = time.Minute * 15
)

// newPeerMsg represents a new peer connected.
type newPeerMsg struct {
	*peer.Peer
	reply chan struct{}
}

// donePeerMsg represents a peer disconnected.
type donePeerMsg struct {
	*peer.Peer
	reply chan struct{}
}

type sendTxMsg struct {
	tx     util.Transaction
	expire time.Time
}

type txInvMsg struct {
	iv *msg.InvVect
}

type txRejectMsg struct {
	iv *msg.InvVect
}

type blockMsg struct {
	block *util.Block
}

// The SPV service implementation
type service struct {
	server.IServer
	cfg         Config
	syncManager *sync.SyncManager

	peerQueue chan interface{}
	txQueue   chan interface{}
	quit      chan struct{}
	// The following chans are used to sync blockmanager and server.
	txProcessed    chan struct{}
	blockProcessed chan struct{}
}

// Create a instance of SPV service implementation.
func newService(cfg *Config) (*service, error) {
	// Initialize blockchain
	chain, err := blockchain.New(cfg.GenesisHeader, cfg.ChainStore)
	if err != nil {
		return nil, err
	}

	// Create SPV service instance
	service := &service{
		cfg:            *cfg,
		peerQueue:      make(chan interface{}, defaultMaxPeers),
		txQueue:        make(chan interface{}, 3),
		quit:           make(chan struct{}),
		txProcessed:    make(chan struct{}, 1),
		blockProcessed: make(chan struct{}, 1),
	}

	// Create sync manager instance.
	syncCfg := sync.NewDefaultConfig(chain, cfg.CandidateFlags, cfg.GetTxFilter)
	syncCfg.MaxPeers = defaultMaxPeers
	if cfg.StateNotifier != nil {
		syncCfg.TransactionAnnounce = cfg.StateNotifier.TransactionAnnounce
	}
	syncManager, err := sync.New(syncCfg)
	if err != nil {
		return nil, err
	}
	service.syncManager = syncManager

	// Initiate P2P server configuration
	dataDir := defaultDataDir
	if len(cfg.DataDir) > 0 {
		dataDir = cfg.DataDir
	}
	_, err = os.Stat(dataDir)
	if os.IsNotExist(err) {
		os.MkdirAll(dataDir, os.ModePerm)
	}

	params := cfg.ChainParams
	svrCfg := server.NewDefaultConfig(
		params.Magic, pact.DPOSStartVersion, 0,
		params.DefaultPort, params.DNSSeeds, nil,
		service.newPeer, service.donePeer, service.makeEmptyMessage,
		func() uint64 { return uint64(chain.BestHeight()) },
	)
	svrCfg.DataDir = dataDir
	svrCfg.MaxPeers = defaultMaxPeers
	svrCfg.DisableListen = true
	svrCfg.DisableRelayTx = true
	svrCfg.PermanentPeers = cfg.PermanentPeers

	// Create P2P server.
	server, err := server.NewServer(svrCfg)
	if err != nil {
		return nil, err
	}
	service.IServer = server

	return service, nil
}

func (s *service) start() {
	go s.peerHandler()
	go s.txHandler()
}

func (s *service) makeEmptyMessage(cmd string) (p2p.Message, error) {
	var message p2p.Message
	switch cmd {
	case p2p.CmdInv:
		message = new(msg.Inv)

	case p2p.CmdGetData:
		message = new(msg.GetData)

	case p2p.CmdNotFound:
		message = new(msg.NotFound)

	case p2p.CmdTx:
		message = msg.NewTx(s.cfg.NewTransaction())

	case p2p.CmdMerkleBlock:
		message = msg.NewMerkleBlock(s.cfg.NewBlockHeader())

	case p2p.CmdReject:
		message = new(msg.Reject)

	default:
		return nil, fmt.Errorf("unhandled command [%s]", cmd)
	}
	return message, nil
}

func (s *service) newPeer(peer server.IPeer) {
	reply := make(chan struct{})
	s.peerQueue <- newPeerMsg{Peer: peer.ToPeer(), reply: reply}
	<-reply
}

func (s *service) donePeer(peer server.IPeer) {
	reply := make(chan struct{})
	s.peerQueue <- donePeerMsg{Peer: peer.ToPeer(), reply: reply}
	<-reply
}

// peerHandler handles new peers and done peers from P2P server.
// When comes new peer, create a spv peer warpper for it
func (s *service) peerHandler() {
	peers := make(map[*peer.Peer]*speer.Peer)

out:
	for {
		select {
		// Deal with peer messages.
		case p := <-s.peerQueue:
			s.handlePeerMsg(peers, p)

		case <-s.quit:
			break out
		}
	}

	// Drain any wait channels before we go away so we don't leave something
	// waiting for us.
cleanup:
	for {
		select {
		case <-s.peerQueue:
		default:
			break cleanup
		}
	}
}

// handlePeerMsg deals with adding and removing peer message.
func (s *service) handlePeerMsg(peers map[*peer.Peer]*speer.Peer, msg interface{}) {
	switch msg := msg.(type) {
	case newPeerMsg:
		// Create spv peer warpper for the new peer.
		sp := speer.NewPeer(msg.Peer, &speer.Config{
			OnVersion:  s.onVersion,
			OnInv:      s.onInv,
			OnTx:       s.onTx,
			OnBlock:    s.onBlock,
			OnNotFound: s.onNotFound,
			OnReject:   s.onReject,
		})

		peers[msg.Peer] = sp
		msg.reply <- struct{}{}

	case donePeerMsg:
		delete(peers, msg.Peer)
		msg.reply <- struct{}{}
	}
}

// txHandler handles transaction messages like send transaction, transaction inv
// transaction reject etc.
func (s *service) txHandler() {
	var unconfirmed = make(map[common.Uint256]*sendTxMsg)
	var accepted = make(map[common.Uint256]struct{})
	var rejected = make(map[common.Uint256]struct{})

	retryTicker := time.NewTicker(txRebroadcastDuration)
	defer retryTicker.Stop()

out:
	for {
		select {
		case tmsg := <-s.txQueue:
			switch tmsg := tmsg.(type) {
			case *sendTxMsg:
				txId := tmsg.tx.Hash()
				tmsg.expire = time.Now().Add(txExpireTime)
				unconfirmed[txId] = tmsg
				delete(accepted, txId)
				delete(rejected, txId)

				// Broadcast unconfirmed transaction
				s.IServer.BroadcastMessage(msg.NewTx(tmsg.tx))

			case *txInvMsg:
				// When a transaction was accepted and add to the txMemPool, a
				// txInv message will be received through message relay, but it
				// only works when there are more than 2 peers connected.
				txId := tmsg.iv.Hash

				// The transaction has been marked as accepted.
				if _, ok := accepted[txId]; ok {
					continue
				}

				// The txInv is an unconfirmed transaction.
				if txMsg, ok := unconfirmed[txId]; ok {
					delete(unconfirmed, txId)
					accepted[txId] = struct{}{}

					// Use a new goroutine do the invoke to prevent blocking.
					go func(tx util.Transaction) {
						if s.cfg.StateNotifier != nil {
							s.cfg.StateNotifier.TransactionAccepted(tx)
						}
					}(txMsg.tx)
				}

			case *txRejectMsg:
				// If some of the peers are bad actors, transaction can be both
				// accepted and rejected.  For we can not say who are bad actors
				// and who are not, so just pick the first response and notify
				// the transaction state change.
				txId := tmsg.iv.Hash

				// The transaction has been marked as rejected.
				if _, ok := rejected[txId]; ok {
					continue
				}

				// The txInv is an unconfirmed transaction.
				if txMsg, ok := unconfirmed[txId]; ok {
					rejected[txId] = struct{}{}
					delete(unconfirmed, txId)

					// Use a new goroutine do the invoke to prevent blocking.
					go func(tx util.Transaction) {
						if s.cfg.StateNotifier != nil {
							s.cfg.StateNotifier.TransactionRejected(tx)
						}
					}(txMsg.tx)
				}

			case *blockMsg:
				// Loop through all packed transactions, see if match to any
				// sent transactions.
				confirmedTxs := make(map[common.Uint256]util.Transaction)
				for _, tx := range tmsg.block.Transactions {
					txId := tx.Hash()

					if _, ok := unconfirmed[txId]; ok {
						confirmedTxs[txId] = tx
						continue
					}

					if _, ok := accepted[txId]; ok {
						confirmedTxs[txId] = tx
						continue
					}

					if _, ok := rejected[txId]; ok {
						confirmedTxs[txId] = tx
					}
				}

				for txId, tx := range confirmedTxs {
					delete(unconfirmed, txId)
					delete(accepted, txId)
					delete(rejected, txId)

					// Use a new goroutine do the invoke to prevent blocking.
					go func(tx *util.Tx) {
						if s.cfg.StateNotifier != nil {
							s.cfg.StateNotifier.TransactionConfirmed(tx)
						}
					}(util.NewTx(tx, tmsg.block.Height))
				}
			}
		case <-retryTicker.C:
			// Rebroadcast unconfirmed transactions.
			now := time.Now()
			for id, tx := range unconfirmed {
				// Delete expired transaction.
				if tx.expire.Before(now) {
					delete(unconfirmed, id)
					continue
				}

				// Broadcast unconfirmed transaction
				s.IServer.BroadcastMessage(msg.NewTx(tx.tx))
			}

		case <-s.quit:
			break out
		}
	}

	// Drain any wait channels before we go away so we don't leave something
	// waiting for us.
cleanup:
	for {
		select {
		case <-s.txQueue:
		default:
			break cleanup
		}
	}
}

func (s *service) SendTransaction(tx util.Transaction) error {
	if !s.IsCurrent() {
		return fmt.Errorf("spv service did not sync to current")
	}

	s.txQueue <- &sendTxMsg{tx: tx}
	return nil
}

// handleDisconnect handles peer disconnects and remove the peer from
// SyncManager.
func (s *service) handleDisconnect(sp *speer.Peer) {
	sp.WaitForDisconnect()
	s.syncManager.DonePeer(sp)
}

// OnVersion is invoked when a peer receives a version message and is
// used to negotiate the protocol version details as well as kick start
// the communications.
func (s *service) onVersion(sp *speer.Peer, m *msg.Version) {
	// Signal the sync manager this peer is a new sync candidate.
	s.syncManager.NewPeer(sp)

	// Handle peer disconnect.
	go s.handleDisconnect(sp)
}

func (s *service) onInv(sp *speer.Peer, inv *msg.Inv) {
	// If service already synced to current, it most likely to receive a relayed
	// block or transaction inv, not a huge invList with block hashes.
	if s.IsCurrent() {
		for _, iv := range inv.InvList {
			switch iv.Type {
			case msg.InvTypeTx:
				s.txQueue <- &txInvMsg{iv: iv}
			}
		}
	}
	s.syncManager.QueueInv(inv, sp)
}

func (s *service) onBlock(sp *speer.Peer, block *util.Block) {
	s.syncManager.QueueBlock(block, sp, s.blockProcessed)

	select {
	case <-s.blockProcessed:
		s.txQueue <- &blockMsg{block: block}
		if s.cfg.StateNotifier != nil {
			s.cfg.StateNotifier.BlockCommitted(block)
		}
	}
}

func (s *service) onTx(sp *speer.Peer, msgTx util.Transaction) {
	s.syncManager.QueueTx(msgTx, sp, s.txProcessed)
	<-s.txProcessed
}

func (s *service) onNotFound(sp *speer.Peer, notFound *msg.NotFound) {
	// Some times when we com to get a transaction, it has been cleared from
	// peer's mempool, so we get this notfound message, in that case, we just
	// ignore it.  But if we come to get blocks and get this message, then the
	// peer is misbehaving, we disconnect it.
	for _, iv := range notFound.InvList {
		if iv.Type == msg.InvTypeTx {
			continue
		}

		log.Warnf("Peer %s is sending us notFound -- disconnecting", sp)
		sp.Disconnect()
	}
}

func (s *service) onReject(sp *speer.Peer, reject *msg.Reject) {
	if reject.Cmd == p2p.CmdTx {
		s.txQueue <- &txRejectMsg{iv: &msg.InvVect{Type: msg.InvTypeTx, Hash: reject.Hash}}
	}
	log.Warnf("reject message from peer %v: Code: %s, Hash %s, Reason: %s",
		sp, reject.Code.String(), reject.Hash.String(), reject.Reason)
}

func (s *service) IsCurrent() bool {
	return s.syncManager.IsCurrent()
}

func (s *service) UpdateFilter() {
	// Broadcast filterload message to connected peers.
	s.IServer.BroadcastMessage(s.cfg.GetTxFilter())
}

func (s *service) Start() {
	s.start()
	s.syncManager.Start()
	s.IServer.Start()
	log.Info("SPV service started...")
}

func (s *service) Stop() {
	err := s.IServer.Stop()
	if err != nil {
		log.Error(err)
	}

	err = s.syncManager.Stop()
	if err != nil {
		log.Error(err)
	}

	s.cfg.ChainStore.Close()

	close(s.quit)
	log.Info("SPV service stopped...")
}
