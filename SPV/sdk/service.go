package sdk

import (
	"fmt"
	"os"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/blockchain"
	"github.com/elastos/Elastos.ELA.SPV/bloom"
	speer "github.com/elastos/Elastos.ELA.SPV/peer"
	"github.com/elastos/Elastos.ELA.SPV/sync"
	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA.Utility/p2p/peer"
	"github.com/elastos/Elastos.ELA.Utility/p2p/server"
)

const (
	defaultDataDir        = "./"
	TxExpireTime          = time.Hour * 24
	TxRebroadcastDuration = time.Minute * 15
)

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

	newPeers  chan *peer.Peer
	donePeers chan *peer.Peer
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
		newPeers:       make(chan *peer.Peer, cfg.MaxPeers),
		donePeers:      make(chan *peer.Peer, cfg.MaxPeers),
		txQueue:        make(chan interface{}, 3),
		quit:           make(chan struct{}),
		txProcessed:    make(chan struct{}, 1),
		blockProcessed: make(chan struct{}, 1),
	}

	var maxPeers int
	if cfg.MaxPeers > 0 {
		maxPeers = cfg.MaxPeers
	}

	// Create sync manager instance.
	syncCfg := sync.NewDefaultConfig(chain, service.updateFilter)
	syncCfg.MaxPeers = maxPeers
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

	serverCfg := server.NewDefaultConfig(
		cfg.Magic,
		p2p.EIP001Version,
		OpenService,
		cfg.DefaultPort,
		cfg.SeedList,
		nil,
		service.newPeer,
		service.donePeer,
		service.makeEmptyMessage,
		func() uint64 { return uint64(chain.BestHeight()) },
	)
	serverCfg.DataDir = dataDir
	serverCfg.MaxPeers = maxPeers
	serverCfg.DisableListen = true
	serverCfg.DisableRelayTx = true

	// Create P2P server.
	server, err := server.NewServer(serverCfg)
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

func (s *service) updateFilter() *bloom.Filter {
	addresses, outpoints := s.cfg.GetFilterData()
	elements := uint32(len(addresses) + len(outpoints))

	filter := bloom.NewFilter(elements, 0, 0)
	for _, address := range addresses {
		filter.Add(address.Bytes())
	}

	for _, op := range outpoints {
		filter.Add(op.Bytes())
	}

	return filter
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
	log.Debugf("server new peer %v", peer)
	s.newPeers <- peer.ToPeer()
}

func (s *service) donePeer(peer server.IPeer) {
	log.Debugf("server done peer %v", peer)
	s.donePeers <- peer.ToPeer()
}

// peerHandler handles new peers and done peers from P2P server.
// When comes new peer, create a spv peer warpper for it
func (s *service) peerHandler() {
	peers := make(map[*peer.Peer]*speer.Peer)

out:
	for {
		select {
		case p := <-s.newPeers:
			// Create spv peer warpper for the new peer.
			sp := speer.NewPeer(p,
				&speer.Config{
					OnInv:      s.onInv,
					OnTx:       s.onTx,
					OnBlock:    s.onBlock,
					OnNotFound: s.onNotFound,
					OnReject:   s.onReject,
				})

			peers[p] = sp
			s.syncManager.NewPeer(sp)

		case p := <-s.donePeers:
			sp, ok := peers[p]
			if !ok {
				log.Errorf("unknown done peer %v", p)
				continue
			}

			s.syncManager.DonePeer(sp)

		case <-s.quit:
			break out
		}
	}

	// Drain any wait channels before we go away so we don't leave something
	// waiting for us.
cleanup:
	for {
		select {
		case <-s.newPeers:
		case <-s.donePeers:
		default:
			break cleanup
		}
	}
}

// txHandler handles transaction messages like send transaction, transaction inv
// transaction reject etc.
func (s *service) txHandler() {
	var unconfirmed = make(map[common.Uint256]*sendTxMsg)
	var accepted = make(map[common.Uint256]struct{})
	var rejected = make(map[common.Uint256]struct{})

	retryTicker := time.NewTicker(TxRebroadcastDuration)
	defer retryTicker.Stop()

out:
	for {
		select {
		case tmsg := <-s.txQueue:
			switch tmsg := tmsg.(type) {
			case *sendTxMsg:
				txId := tmsg.tx.Hash()
				tmsg.expire = time.Now().Add(TxExpireTime)
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
	// Every thing we requested was came from this connected peer, so
	// no reason it said I have some data you don't have and when you
	// come to get it, it say oh I didn't have it.
	log.Warnf("Peer %s is sending us notFound -- disconnecting", sp)
	sp.Disconnect()
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
	// Update bloom filter
	filter := s.updateFilter()

	// Broadcast filterload message to connected peers.
	s.IServer.BroadcastMessage(filter.GetFilterLoadMsg())
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
