package elanet

import (
	"bytes"
	"errors"
	"fmt"
	"sync/atomic"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/elanet/bloom"
	"github.com/elastos/Elastos.ELA/elanet/filter"
	"github.com/elastos/Elastos.ELA/elanet/filter/sidefilter"
	"github.com/elastos/Elastos.ELA/elanet/netsync"
	"github.com/elastos/Elastos.ELA/elanet/pact"
	"github.com/elastos/Elastos.ELA/elanet/peer"
	"github.com/elastos/Elastos.ELA/elanet/routes"
	"github.com/elastos/Elastos.ELA/mempool"
	"github.com/elastos/Elastos.ELA/p2p"
	"github.com/elastos/Elastos.ELA/p2p/msg"
	svr "github.com/elastos/Elastos.ELA/p2p/server"
)

const (
	// defaultServices describes the default services that are supported by
	// the server.
	defaultServices = pact.SFNodeNetwork | pact.SFTxFiltering | pact.SFNodeBloom

	// maxNonNodePeers defines the maximum count of accepting non-node peers.
	maxNonNodePeers = 100
)

// naFilter defines a network address filter for the main chain server, for now
// it is used to filter SPV wallet addresses from relaying to other peers.
type naFilter struct{}

func (f *naFilter) Filter(na *p2p.NetAddress) bool {
	service := pact.ServiceFlag(na.Services)
	return service&pact.SFNodeNetwork == pact.SFNodeNetwork
}

// newPeerMsg represent a new connected peer.
type newPeerMsg struct {
	svr.IPeer
	reply chan struct{}
}

// donePeerMsg represent a disconnected peer.
type donePeerMsg struct {
	svr.IPeer
	reply chan struct{}
}

// relayMsg packages an inventory vector along with the newly discovered
// inventory so the relay has access to that information.
type relayMsg struct {
	invVect *msg.InvVect
	data    interface{}
}

// server provides a server for handling communications to and from peers.
type server struct {
	svr.IServer
	syncManager  *netsync.SyncManager
	chain        *blockchain.BlockChain
	chainParams  *config.Params
	txMemPool    *mempool.TxPool
	blockMemPool *mempool.BlockPool
	routes       *routes.Routes

	nonNodePeers int32 // This variable must be use atomically.
	peerQueue    chan interface{}
	relayInv     chan relayMsg
	quit         chan struct{}
	services     pact.ServiceFlag
}

// serverPeer extends the peer to maintain state shared by the server and
// the blockmanager.
type serverPeer struct {
	*peer.Peer

	server        *server
	continueHash  *common.Uint256
	isWhitelisted bool
	filter        *filter.Filter
	quit          chan struct{}
	// The following chans are used to sync blockmanager and server.
	txProcessed    chan struct{}
	blockProcessed chan struct{}
}

// newServerPeer returns a new serverPeer instance. The peer needs to be set by
// the caller.
func newServerPeer(s *server) *serverPeer {
	filter := filter.New(func(typ uint8) filter.TxFilter {
		switch typ {
		case filter.FTBloom:
			return bloom.NewTxFilter()
		case filter.FTDPOS:
			return sidefilter.New(s.chain.GetState())
		}
		return nil
	})

	return &serverPeer{
		server:         s,
		filter:         filter,
		quit:           make(chan struct{}),
		txProcessed:    make(chan struct{}, 1),
		blockProcessed: make(chan struct{}, 1),
	}
}

// handleDisconnect handles peer disconnects and remove the peer from
// SyncManager and Routes.
func (sp *serverPeer) handleDisconnect() {
	sp.WaitForDisconnect()
	sp.server.routes.DonePeer(sp.Peer)
	sp.server.syncManager.DonePeer(sp.Peer)

	// Decrease non node count.
	if !nodeFlag(uint64(sp.Services())) {
		atomic.AddInt32(&sp.server.nonNodePeers, -1)
	}
}

// OnVersion is invoked when a peer receives a version message and is
// used to negotiate the protocol version details as well as kick start
// the communications.
func (sp *serverPeer) OnVersion(_ *peer.Peer, m *msg.Version) {
	// Disconnect full node peers that do not support DPOS protocol.
	if nodeFlag(m.Services) && sp.ProtocolVersion() < pact.DPOSStartVersion {
		sp.Disconnect()
		return
	}

	if !nodeFlag(m.Services) {
		// Disconnect non node peers if limitation arrived.
		if atomic.LoadInt32(&sp.server.nonNodePeers) >= maxNonNodePeers {
			sp.Disconnect()
			return
		}

		// Increase non node peers count.
		atomic.AddInt32(&sp.server.nonNodePeers, 1)
	}

	// Add the remote peer time as a sample for creating an offset against
	// the local clock to keep the network time in sync.
	sp.server.chain.TimeSource.AddTimeSample(sp.Addr(), m.Timestamp)

	// Signal the routes this peer is a new sync candidate.
	sp.server.routes.NewPeer(sp.Peer)

	// Signal the sync manager this peer is a new sync candidate.
	sp.server.syncManager.NewPeer(sp.Peer)

	// Choose whether or not to relay transactions before a filter command
	// is received.
	sp.SetDisableRelayTx(!m.Relay)

	// Handle peer disconnect.
	go sp.handleDisconnect()
}

// OnMemPool is invoked when a peer receives a mempool message.
// It creates and sends an inventory message with the contents of the memory
// pool up to the maximum inventory allowed per message.  When the peer has a
// bloom filter loaded, the contents are filtered accordingly.
func (sp *serverPeer) OnMemPool(_ *peer.Peer, _ *msg.MemPool) {
	// Only allow mempool requests if the server has bloom filtering
	// enabled.
	if sp.server.services&pact.SFTxFiltering != pact.SFTxFiltering {
		log.Debugf("peer %v sent mempool request with bloom "+
			"filtering disabled -- disconnecting", sp)
		sp.Disconnect()
		return
	}

	// A decaying ban score increase is applied to prevent flooding.
	// The ban score accumulates and passes the ban threshold if a burst of
	// mempool messages comes from a peer. The score decays each minute to
	// half of its value.
	sp.AddBanScore(0, 33, "mempool")

	// Generate inventory message with the available transactions in the
	// transaction memory pool.  Limit it to the max allowed inventory
	// per message.  The NewMsgInvSizeHint function automatically limits
	// the passed hint to the maximum allowed, so it's safe to pass it
	// without double checking it here.
	txs := sp.server.txMemPool.GetTxsInPool()
	invMsg := msg.NewInvSize(uint(len(txs)))

	for _, tx := range txs {
		// Either add all transactions when there is no bloom filter,
		// or only the transactions that match the filter when there is
		// one.
		txId := tx.Hash()
		if !sp.filter.IsLoaded() || sp.filter.MatchUnconfirmed(tx) {
			iv := msg.NewInvVect(msg.InvTypeTx, &txId)
			invMsg.AddInvVect(iv)
			if len(invMsg.InvList)+1 > msg.MaxInvPerMsg {
				break
			}
		}
	}

	// Send the inventory message if there is anything to send.
	if len(invMsg.InvList) > 0 {
		sp.QueueMessage(invMsg, nil)
	}
}

// OnTx is invoked when a peer receives a tx message.  It blocks
// until the transaction has been fully processed.  Unlock the block
// handler this does not serialize all transactions through a single thread
// transactions don't rely on the previous one in a linear fashion like blocks.
func (sp *serverPeer) OnTx(_ *peer.Peer, msgTx *msg.Tx) {
	// Add the transaction to the known inventory for the peer.
	// Convert the raw MsgTx to a btcutil.Tx which provides some convenience
	// methods and things such as hash caching.
	tx := msgTx.Serializable.(*types.Transaction)
	txId := tx.Hash()
	iv := msg.NewInvVect(msg.InvTypeTx, &txId)
	sp.AddKnownInventory(iv)

	// Queue the transaction up to be handled by the sync manager and
	// intentionally block further receives until the transaction is fully
	// processed and known good or bad.  This helps prevent a malicious peer
	// from queuing up a bunch of bad transactions before disconnecting (or
	// being disconnected) and wasting memory.
	sp.server.syncManager.QueueTx(tx, sp.Peer, sp.txProcessed)
	<-sp.txProcessed
}

// OnBlock is invoked when a peer receives a block message.  It
// blocks until the block has been fully processed.
func (sp *serverPeer) OnBlock(_ *peer.Peer, msgBlock *msg.Block) {
	block := msgBlock.Serializable.(*types.DposBlock)
	blockHash := block.Block.Hash()
	iv := msg.NewInvVect(msg.InvTypeBlock, &blockHash)
	if block.HaveConfirm {
		iv.Type = msg.InvTypeConfirmedBlock
	}

	// Add the block to the known inventory for the peer.
	sp.AddKnownInventory(iv)

	// Queue the block up to be handled by the block
	// manager and intentionally block further receives
	// until the block is fully processed and known
	// good or bad.  This helps prevent a malicious peer
	// from queuing up a bunch of bad blocks before
	// disconnecting (or being disconnected) and wasting
	// memory.  Additionally, this behavior is depended on
	// by at least the block acceptance test tool as the
	// reference implementation processes blocks in the same
	// thread and therefore blocks further messages until
	// the block has been fully processed.
	sp.server.syncManager.QueueBlock(block, sp.Peer, sp.blockProcessed)
	<-sp.blockProcessed
}

// OnInv is invoked when a peer receives an inv message and is
// used to examine the inventory being advertised by the remote peer and react
// accordingly.  We pass the message down to blockmanager which will call
// QueueMessage with any appropriate responses.
func (sp *serverPeer) OnInv(_ *peer.Peer, inv *msg.Inv) {
	if len(inv.InvList) > 0 {
		sp.server.syncManager.QueueInv(inv, sp.Peer)
		sp.server.routes.QueueInv(sp.Peer, inv)
	}
}

// OnNotFound is invoked when a peer receives an notfounc message.
// A peer should not response a notfound message so we just disconnect it.
func (sp *serverPeer) OnNotFound(_ *peer.Peer, notFound *msg.NotFound) {
	for _, i := range notFound.InvList {
		if i.Type == msg.InvTypeTx {
			continue
		}

		log.Debugf("%s sent us notfound message --  disconnecting", sp)
		sp.AddBanScore(100, 0, notFound.CMD())
		sp.Disconnect()
		return
	}
}

// handleGetData is invoked when a peer receives a getdata message and
// is used to deliver block and transaction information.
func (sp *serverPeer) OnGetData(_ *peer.Peer, getData *msg.GetData) {
	// Notify the GetData message to DPOS routes.
	sp.server.routes.OnGetData(sp.Peer, getData)

	numAdded := 0
	notFound := msg.NewNotFound()

	length := len(getData.InvList)
	// A decaying ban score increase is applied to prevent exhausting resources
	// with unusually large inventory queries.
	// Requesting more than the maximum inventory vector length within a short
	// period of time yields a score above the default ban threshold. Sustained
	// bursts of small requests are not penalized as that would potentially ban
	// peers performing IBD.
	// This incremental score decays each minute to half of its value.
	sp.AddBanScore(0, uint32(length)*99/msg.MaxInvPerMsg, "getdata")

	// We wait on this wait channel periodically to prevent queuing
	// far more data than we can send in a reasonable time, wasting memory.
	// The waiting occurs after the database fetch for the next one to
	// provide a little pipelining.
	var waitChan chan struct{}
	doneChan := make(chan struct{}, 1)

	for i, iv := range getData.InvList {
		var c chan struct{}
		// If this will be the last message we send.
		if i == length-1 && len(notFound.InvList) == 0 {
			c = doneChan
		} else if (i+1)%5 == 0 {
			// Buffered so as to not make the send goroutine block.
			c = make(chan struct{}, 1)
		}
		var err error
		switch iv.Type {
		case msg.InvTypeTx:
			err = sp.server.pushTxMsg(sp, &iv.Hash, c, waitChan)
		case msg.InvTypeBlock:
			err = sp.server.pushBlockMsg(sp, &iv.Hash, c, waitChan)
		case msg.InvTypeConfirmedBlock:
			err = sp.server.pushConfirmedBlockMsg(sp, &iv.Hash, c, waitChan)
		case msg.InvTypeFilteredBlock:
			err = sp.server.pushMerkleBlockMsg(sp, &iv.Hash, c, waitChan)
		case msg.InvTypeAddress:
			continue
		default:
			log.Warnf("Unknown type in inventory request %d", iv.Type)
			continue
		}
		if err != nil {
			notFound.AddInvVect(iv)

			// When there is a failure fetching the final entry
			// and the done channel was sent in due to there
			// being no outstanding not found inventory, consume
			// it here because there is now not found inventory
			// that will use the channel momentarily.
			if i == length-1 && c != nil {
				<-c
			}
		}
		numAdded++
		waitChan = c
	}
	if len(notFound.InvList) != 0 {
		sp.QueueMessage(notFound, doneChan)
	}

	// Wait for messages to be sent. We can send quite a lot of data at this
	// point and this will keep the peer busy for a decent amount of time.
	// We don't process anything else by them in this time so that we
	// have an idea of when we should hear back from them - else the idle
	// timeout could fire when we were only half done sending the blocks.
	if numAdded > 0 {
		<-doneChan
	}
}

// OnGetBlocks is invoked when a peer receives a getblocks
// message.
func (sp *serverPeer) OnGetBlocks(_ *peer.Peer, m *msg.GetBlocks) {
	// Find the most recent known block in the best chain based on the block
	// locator and fetch all of the block hashes after it until either
	// wire.MaxBlocksPerMsg have been fetched or the provided stop hash is
	// encountered.
	//
	// Use the block after the genesis block if no other blocks in the
	// provided locator are known.  This does mean the client will start
	// over with the genesis block if unknown block locators are provided.
	//
	// This mirrors the behavior in the reference implementation.
	chain := sp.server.chain
	hashList := chain.LocateBlocks(m.Locator, &m.HashStop, pact.MaxBlocksPerMsg)

	// Generate inventory message.
	invMsg := msg.NewInv()
	for i := range hashList {
		invType := msg.InvTypeConfirmedBlock
		if sp.filter.IsLoaded() { // Compatible for SPV client.
			invType = msg.InvTypeBlock
		}
		iv := msg.NewInvVect(invType, hashList[i])
		invMsg.AddInvVect(iv)
	}

	// Send the inventory message if there is anything to send.
	if len(invMsg.InvList) > 0 {
		invListLen := len(invMsg.InvList)
		if invListLen == pact.MaxBlocksPerMsg {
			// Intentionally use a copy of the final hash so there
			// is not a reference into the inventory slice which
			// would prevent the entire slice from being eligible
			// for GC as soon as it's sent.
			continueHash := invMsg.InvList[invListLen-1].Hash
			sp.continueHash = &continueHash
		}
		sp.QueueMessage(invMsg, nil)
	}
}

// enforceTxFilterFlag disconnects the peer if the server is not configured to
// allow tx filters.  Additionally, if the peer has negotiated to a protocol
// version  that is high enough to observe the bloom filter service support bit,
// it will be banned since it is intentionally violating the protocol.
func (sp *serverPeer) enforceTxFilterFlag(cmd string) bool {
	if sp.server.services&pact.SFTxFiltering != pact.SFTxFiltering {
		// Disconnect the peer regardless of protocol version or banning
		// state.
		log.Debugf("%s sent an unsupported %s request -- "+
			"disconnecting", sp, cmd)
		sp.AddBanScore(100, 0, cmd)
		sp.Disconnect()
		return false
	}

	return true
}

// OnFilterAdd is invoked when a peer receives a filteradd
// message and is used by remote peers to add data to an already loaded bloom
// filter.  The peer will be disconnected if a filter is not loaded when this
// message is received or the server is not configured to allow bloom filters.
func (sp *serverPeer) OnFilterAdd(_ *peer.Peer, filterAdd *msg.FilterAdd) {
	// Disconnect and/or ban depending on the node bloom services flag and
	// negotiated protocol version.
	if !sp.enforceTxFilterFlag(filterAdd.CMD()) {
		return
	}

	if !sp.filter.IsLoaded() {
		log.Debugf("%s sent a filteradd request with no filter "+
			"loaded -- disconnecting", sp)
		sp.Disconnect()
		return
	}

	err := sp.filter.Add(filterAdd.Data)
	if err != nil {
		log.Debugf("%s sent invalid filteradd request with error %s"+
			" -- disconnecting", sp, err)
		sp.Disconnect()
	}
}

// OnFilterClear is invoked when a peer receives a filterclear
// message and is used by remote peers to clear an already loaded bloom filter.
// The peer will be disconnected if a filter is not loaded when this message is
// received  or the server is not configured to allow bloom filters.
func (sp *serverPeer) OnFilterClear(_ *peer.Peer, filterClear *msg.FilterClear) {
	// Disconnect and/or ban depending on the node bloom services flag and
	// negotiated protocol version.
	if !sp.enforceTxFilterFlag(filterClear.CMD()) {
		return
	}

	if !sp.filter.IsLoaded() {
		log.Debugf("%s sent a filterclear request with no "+
			"filter loaded -- disconnecting", sp)
		sp.Disconnect()
		return
	}

	sp.filter.Clear()

	sp.SetDisableRelayTx(true)
}

// OnFilterLoad is invoked when a peer receives a filterload
// message and it used to load a bloom filter that should be used for
// delivering merkle blocks and associated transactions that match the filter.
// The peer will be disconnected if the server is not configured to allow bloom
// filters.
func (sp *serverPeer) OnFilterLoad(_ *peer.Peer, filterLoad *msg.FilterLoad) {
	// Disconnect and/or ban depending on the node bloom services flag and
	// negotiated protocol version.
	if !sp.enforceTxFilterFlag(filterLoad.CMD()) {
		return
	}

	sp.SetDisableRelayTx(false)

	buf := new(bytes.Buffer)
	filterLoad.Serialize(buf)
	err := sp.filter.Load(&msg.TxFilterLoad{
		Type: filter.FTBloom,
		Data: buf.Bytes(),
	})
	if err != nil {
		log.Debugf("%s sent invalid filterload request with error %s"+
			" -- disconnecting", sp, err)
		sp.Disconnect()
	}
}

// OnTxFilterLoad is invoked when a peer receives a txfilter message and it used to
// load a transaction filter that should be used for delivering merkle blocks and
// associated transactions that match the filter. The peer will be disconnected
// if the server is not configured to allow transaction filtering.
func (sp *serverPeer) OnTxFilterLoad(_ *peer.Peer, tf *msg.TxFilterLoad) {
	// Disconnect and/or ban depending on the tx filter services flag and
	// negotiated protocol version.
	if !sp.enforceTxFilterFlag(tf.CMD()) {
		return
	}

	sp.SetDisableRelayTx(false)

	err := sp.filter.Load(tf)
	if err != nil {
		log.Debugf("%s sent invalid txfilter request with error %s"+
			" -- disconnecting", sp, err)
		sp.Disconnect()
		return
	}
}

// OnReject is invoked when a peer receives a reject message.
func (sp *serverPeer) OnReject(_ *peer.Peer, msg *msg.Reject) {
	log.Infof("%s sent a reject message Code: %s, Hash %s, Reason: %s",
		sp, msg.Code.String(), msg.Hash.String(), msg.Reason)
}

// pushTxMsg sends a tx message for the provided transaction hash to the
// connected peer.  An error is returned if the transaction hash is not known.
func (s *server) pushTxMsg(sp *serverPeer, hash *common.Uint256, doneChan chan<- struct{},
	waitChan <-chan struct{}) error {

	// Attempt to fetch the requested transaction from the pool.  A
	// call could be made to check for existence first, but simply trying
	// to fetch a missing transaction results in the same behavior.
	tx := s.txMemPool.GetTransaction(*hash)
	if tx == nil {
		if doneChan != nil {
			doneChan <- struct{}{}
		}
		return fmt.Errorf("unable to fetch tx %v from transaction pool", hash)
	}

	// Once we have fetched data wait for any previous operation to finish.
	if waitChan != nil {
		<-waitChan
	}

	sp.QueueMessage(msg.NewTx(tx), doneChan)

	return nil
}

// pushBlockMsg sends a block message for the provided block hash to the
// connected peer.  An error is returned if the block hash is not known.
func (s *server) pushBlockMsg(sp *serverPeer, hash *common.Uint256, doneChan chan<- struct{},
	waitChan <-chan struct{}) error {

	// Fetch the block from the database.
	block, _ := s.blockMemPool.GetDposBlockByHash(*hash)
	if block == nil {
		block, _ = s.chain.GetDposBlockByHash(*hash)
		if block == nil {
			if doneChan != nil {
				doneChan <- struct{}{}
			}
			return errors.New("not found block")
		}
	}
	block.HaveConfirm = false
	block.Confirm = nil

	// Once we have fetched data wait for any previous operation to finish.
	if waitChan != nil {
		<-waitChan
	}

	// We only send the channel for this message if we aren't sending
	// an inv straight after.
	var dc chan<- struct{}
	continueHash := sp.continueHash
	sendInv := continueHash != nil && continueHash.IsEqual(*hash)
	if !sendInv {
		dc = doneChan
	}
	sp.QueueMessage(msg.NewBlock(block), dc)

	// When the peer requests the final block that was advertised in
	// response to a getblocks message which requested more blocks than
	// would fit into a single message, send it a new inventory message
	// to trigger it to issue another getblocks message for the next
	// batch of inventory.
	if sendInv {
		best := sp.server.chain.BestChain
		invMsg := msg.NewInvSize(1)
		iv := msg.NewInvVect(msg.InvTypeBlock, best.Hash)
		invMsg.AddInvVect(iv)
		sp.QueueMessage(invMsg, doneChan)
		sp.continueHash = nil
	}
	return nil
}

// pushBlockMsg sends a block message for the provided block hash to the
// connected peer.  An error is returned if the block hash is not known.
func (s *server) pushConfirmedBlockMsg(sp *serverPeer, hash *common.Uint256, doneChan chan<- struct{},
	waitChan <-chan struct{}) error {

	// Fetch the block from the database.
	block, err := s.chain.GetDposBlockByHash(*hash)
	if block == nil {
		if doneChan != nil {
			doneChan <- struct{}{}
		}
		return err
	}

	// Once we have fetched data wait for any previous operation to finish.
	if waitChan != nil {
		<-waitChan
	}

	// We only send the channel for this message if we aren't sending
	// an inv straight after.
	var dc chan<- struct{}
	continueHash := sp.continueHash
	sendInv := continueHash != nil && continueHash.IsEqual(*hash)
	if !sendInv {
		dc = doneChan
	}
	sp.QueueMessage(msg.NewBlock(block), dc)

	// When the peer requests the final block that was advertised in
	// response to a getblocks message which requested more blocks than
	// would fit into a single message, send it a new inventory message
	// to trigger it to issue another getblocks message for the next
	// batch of inventory.
	if sendInv {
		best := sp.server.chain.BestChain
		invMsg := msg.NewInvSize(1)
		iv := msg.NewInvVect(msg.InvTypeConfirmedBlock, best.Hash)
		invMsg.AddInvVect(iv)
		sp.QueueMessage(invMsg, doneChan)
		sp.continueHash = nil
	}
	return nil
}

// pushMerkleBlockMsg sends a merkleblock message for the provided block hash to
// the connected peer.  Since a merkle block requires the peer to have a filter
// loaded, this call will simply be ignored if there is no filter loaded.  An
// error is returned if the block hash is not known.
func (s *server) pushMerkleBlockMsg(sp *serverPeer, hash *common.Uint256,
	doneChan chan<- struct{}, waitChan <-chan struct{}) error {

	// Do not send a response if the peer doesn't have a filter loaded.
	if !sp.filter.IsLoaded() {
		if doneChan != nil {
			doneChan <- struct{}{}
		}
		return nil
	}

	// Fetch the block from the database.
	blk, err := s.chain.GetDposBlockByHash(*hash)
	if err != nil {
		if doneChan != nil {
			doneChan <- struct{}{}
		}
		return err
	}

	// Generate a merkle block by filtering the requested block according
	// to the filter for the peer.
	merkle, matchedTxIndices := filter.NewMerkleBlock(blk.Transactions,
		sp.filter)

	// Create block header.
	switch sp.filter.Filter().(type) {
	// Compatible with old version SPV clients.
	case *bloom.TxFilter:
		merkle.Header = &blk.Header

	// Side chain needs DPOS header format to receive confirm.
	case *sidefilter.Filter:
		var confirm payload.Confirm
		if blk.HaveConfirm {
			confirm = *blk.Confirm
		}
		merkle.Header = &types.DPOSHeader{
			Header:      blk.Header,
			HaveConfirm: blk.HaveConfirm,
			Confirm:     confirm,
		}
	}

	// Once we have fetched data wait for any previous operation to finish.
	if waitChan != nil {
		<-waitChan
	}

	// Send the merkleblock.  Only send the done channel with this message
	// if no transactions will be sent afterwards.
	var dc chan<- struct{}
	if len(matchedTxIndices) == 0 {
		dc = doneChan
	}
	sp.QueueMessage(merkle, dc)

	// Finally, send any matched transactions.
	blkTransactions := blk.Transactions
	for i, txIndex := range matchedTxIndices {
		// Only send the done channel on the final transaction.
		var dc chan<- struct{}
		if i == len(matchedTxIndices)-1 {
			dc = doneChan
		}
		if txIndex < uint32(len(blkTransactions)) {
			sp.QueueMessage(msg.NewTx(blkTransactions[txIndex]), dc)
		}
	}

	return nil
}

// handleRelayInvMsg deals with relaying inventory to peers that are not already
// known to have it.  It is invoked from the peerHandler goroutine.
func (s *server) handleRelayInvMsg(peers map[svr.IPeer]*serverPeer, rmsg relayMsg) {
	// TODO remove after main net growth higher than H1 for efficiency.
	current := s.chain.GetHeight()

	for _, sp := range peers {
		if !sp.Connected() {
			continue
		}

		switch rmsg.invVect.Type {
		case msg.InvTypeTx:
			// Don't relay the transaction to the peer when it has
			// transaction relaying disabled.
			if sp.RelayTxDisabled() {
				continue
			}

			tx, ok := rmsg.data.(*types.Transaction)
			if !ok {
				log.Warnf("Underlying data for tx inv "+
					"relay is not a *core.Transaction: %T",
					rmsg.data)
				return
			}

			// Don't relay the transaction if there is a bloom
			// filter loaded and the transaction doesn't match it.
			if sp.filter.IsLoaded() &&
				!sp.filter.MatchUnconfirmed(tx) {
				continue
			}

		case msg.InvTypeBlock:
			fallthrough
		case msg.InvTypeConfirmedBlock:
			// Compatible for old version SPV client.
			if sp.filter.IsLoaded() {
				// Do not send unconfirmed block to SPV client after H1.
				if current >= s.chainParams.CRCOnlyDPOSHeight-1 &&
					rmsg.invVect.Type == msg.InvTypeBlock {
					continue
				}

				// Change inv type to InvTypeBlock for compatible.
				invVect := *rmsg.invVect
				invVect.Type = msg.InvTypeBlock

				sp.QueueInventory(&invVect)
				continue
			}
		}

		// Queue the inventory to be relayed with the next batch.
		// It will be ignored if the peer is already known to
		// have the inventory.
		sp.QueueInventory(rmsg.invVect)
	}
}

// peerHandler is used to handle peer operations such as adding and removing
// peers to and from the server, banning peers, and broadcasting messages to
// peers.  It must be run in a goroutine.
func (s *server) peerHandler() {
	// Start the address manager and sync manager, both of which are needed
	// by peers.  This is done here since their lifecycle is closely tied
	// to this handler and rather than adding more channels to sychronize
	// things, it's easier and slightly faster to simply start and stop them
	// in this handler.
	s.syncManager.Start()

	peers := make(map[svr.IPeer]*serverPeer)

out:
	for {
		select {
		// Deal with peer messages.
		case peer := <-s.peerQueue:
			s.handlePeerMsg(peers, peer)

			// New inventory to potentially be relayed to other peers.
		case invMsg := <-s.relayInv:
			s.handleRelayInvMsg(peers, invMsg)

		case <-s.quit:
			break out
		}
	}

	s.syncManager.Stop()

	// Drain channels before exiting so nothing is left waiting around
	// to send.
cleanup:
	for {
		select {
		case <-s.peerQueue:
		case <-s.relayInv:
		default:
			break cleanup
		}
	}
}

// handlePeerMsg deals with adding and removing peers.
func (s *server) handlePeerMsg(peers map[svr.IPeer]*serverPeer, p interface{}) {
	switch p := p.(type) {
	case newPeerMsg:
		sp := newServerPeer(s)
		sp.Peer = peer.New(p, &peer.Listeners{
			OnVersion:      sp.OnVersion,
			OnMemPool:      sp.OnMemPool,
			OnTx:           sp.OnTx,
			OnBlock:        sp.OnBlock,
			OnInv:          sp.OnInv,
			OnNotFound:     sp.OnNotFound,
			OnGetData:      sp.OnGetData,
			OnGetBlocks:    sp.OnGetBlocks,
			OnFilterAdd:    sp.OnFilterAdd,
			OnFilterClear:  sp.OnFilterClear,
			OnFilterLoad:   sp.OnFilterLoad,
			OnTxFilterLoad: sp.OnTxFilterLoad,
			OnReject:       sp.OnReject,
			OnDAddr:        s.routes.QueueDAddr,
		})

		peers[p.IPeer] = sp
		p.reply <- struct{}{}

	case donePeerMsg:
		delete(peers, p.IPeer)
		p.reply <- struct{}{}
	}
}

// Services returns the service flags the server supports.
func (s *server) Services() pact.ServiceFlag {
	return s.services
}

// NewPeer adds a new peer that has already been connected to the server.
func (s *server) NewPeer(p svr.IPeer) {
	reply := make(chan struct{})
	s.peerQueue <- newPeerMsg{p, reply}
	<-reply
}

// DonePeer removes a peer that has already been connected to the server by ip.
func (s *server) DonePeer(p svr.IPeer) {
	reply := make(chan struct{})
	s.peerQueue <- donePeerMsg{p, reply}
	<-reply
}

// RelayInventory relays the passed inventory vector to all connected peers
// that are not already known to have it.
func (s *server) RelayInventory(invVect *msg.InvVect, data interface{}) {
	s.relayInv <- relayMsg{invVect: invVect, data: data}
}

// IsCurrent returns whether or not the sync manager believes it is synced with
// the connected peers.
func (s *server) IsCurrent() bool {
	return s.syncManager.IsCurrent()
}

// Start begins accepting connections from peers.
func (s *server) Start() {
	s.routes.Start()
	s.IServer.Start()

	go s.peerHandler()
}

// Stop gracefully shuts down the server by stopping and disconnecting all
// peers and the main listener.
func (s *server) Stop() error {
	s.routes.Stop()
	err := s.IServer.Stop()

	// Signal the remaining goroutines to quit.
	close(s.quit)
	return err
}

// NewServer returns a new elanet server configured to listen on addr for the
// network type specified by chainParams.  Use start to begin accepting
// connections from peers.
func NewServer(dataDir string, cfg *Config) (*server, error) {
	services := defaultServices
	params := cfg.ChainParams
	if params.DisableTxFilters {
		services &^= pact.SFNodeBloom
		services &^= pact.SFTxFiltering
	}

	// If no listeners added, create default listener.
	if len(params.ListenAddrs) == 0 {
		params.ListenAddrs = []string{fmt.Sprint(":", params.DefaultPort)}
	}

	svrCfg := svr.NewDefaultConfig(
		params.Magic, pact.DPOSStartVersion, uint64(services),
		params.DefaultPort, params.DNSSeeds, params.ListenAddrs,
		nil, nil, makeEmptyMessage,
		func() uint64 { return uint64(cfg.Chain.GetHeight()) },
	)
	svrCfg.DataDir = dataDir
	svrCfg.NAFilter = &naFilter{}
	svrCfg.PermanentPeers = cfg.PermanentPeers

	s := server{
		chain:        cfg.Chain,
		chainParams:  cfg.ChainParams,
		txMemPool:    cfg.TxMemPool,
		blockMemPool: cfg.BlockMemPool,
		routes:       cfg.Routes,
		peerQueue:    make(chan interface{}, svrCfg.MaxPeers),
		relayInv:     make(chan relayMsg, svrCfg.MaxPeers),
		quit:         make(chan struct{}),
		services:     services,
	}
	svrCfg.OnNewPeer = s.NewPeer
	svrCfg.OnDonePeer = s.DonePeer

	p2pServer, err := svr.NewServer(svrCfg)
	if err != nil {
		return nil, err
	}
	s.IServer = p2pServer

	s.syncManager = netsync.New(&netsync.Config{
		PeerNotifier: &s,
		Chain:        cfg.Chain,
		ChainParams:  cfg.ChainParams,
		TxMemPool:    cfg.TxMemPool,
		BlockMemPool: cfg.BlockMemPool,
		MaxPeers:     svrCfg.MaxPeers,
	})

	return &s, nil
}

func makeEmptyMessage(cmd string) (p2p.Message, error) {
	var message p2p.Message
	switch cmd {
	case p2p.CmdMemPool:
		message = &msg.MemPool{}

	case p2p.CmdTx:
		message = msg.NewTx(&types.Transaction{})

	case p2p.CmdBlock:
		message = msg.NewBlock(&types.DposBlock{})

	case p2p.CmdInv:
		message = &msg.Inv{}

	case p2p.CmdNotFound:
		message = &msg.NotFound{}

	case p2p.CmdGetData:
		message = &msg.GetData{}

	case p2p.CmdGetBlocks:
		message = &msg.GetBlocks{}

	case p2p.CmdFilterAdd:
		message = &msg.FilterAdd{}

	case p2p.CmdFilterClear:
		message = &msg.FilterClear{}

	case p2p.CmdFilterLoad:
		message = &msg.FilterLoad{}

	case p2p.CmdTxFilter:
		message = &msg.TxFilterLoad{}

	case p2p.CmdReject:
		message = &msg.Reject{}

	case p2p.CmdDAddr:
		message = &msg.DAddr{}

	default:
		return nil, fmt.Errorf("unhandled command [%s]", cmd)
	}
	return message, nil
}

// nodeFlag returns if a peer contains the full node flag.
func nodeFlag(flag uint64) bool {
	return pact.ServiceFlag(flag)&pact.SFNodeNetwork == pact.SFNodeNetwork
}
