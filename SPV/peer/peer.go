package sdk

import (
	"time"

	"github.com/elastos/Elastos.ELA.SPV/log"
	"github.com/elastos/Elastos.ELA.SPV/net"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA.Utility/p2p/rw"
	"github.com/elastos/Elastos.ELA/core"
)

const (
	// stallTickInterval is the interval of time between each check for
	// stalled peers.
	stallTickInterval = 5 * time.Second

	// stallResponseTimeout is the base maximum amount of time messages that
	// expect a response will wait before disconnecting the peer for
	// stalling.  The deadlines are adjusted for callback running times and
	// only checked on each stall tick interval.
	stallResponseTimeout = 15 * time.Second
)

func (p *SPVPeer) EnqueueBlock(header *msg.MerkleBlock, txIds []*common.Uint256) *block {
	hash := header.Header.(*core.Header).Hash()

	block := newBlock(header)

	// No transactions to download, just finish it
	if len(txIds) == 0 {
		return block
	}

	// Download transactions of this block
	getData := msg.NewGetData()
	for _, txId := range txIds {
		block.txQueue[*txId] = struct{}{}
		getData.AddInvVect(msg.NewInvVect(msg.InvTypeTx, txId))
	}
	// Stall message
	p.stallControl <- getData

	p.blocksQueue[hash] = block

	return nil
}

func (p *SPVPeer) EnqueueTx(txId common.Uint256) {
	p.txsQueue[txId] = struct{}{}
}

func (p *SPVPeer) DequeueTx(tx *core.Transaction) (download interface{}, finished bool) {
	// Get tx id
	txId := tx.Hash()

	// Check downloading block first
	if blockId, ok := p.blockTxs[txId]; ok {
		// Dequeue block tx
		block := p.blocksQueue[blockId]
		block.txs = append(block.txs, tx)
		delete(block.txQueue, txId)

		return block, len(block.txQueue) == 0
	}

	// Dequeue from downloading tx
	_, ok := p.txsQueue[txId]
	if !ok {
		return tx, false
	}
	delete(p.txsQueue, txId)

	return tx, true
}

type block struct {
	*msg.MerkleBlock
	txQueue map[common.Uint256]struct{}
	txs     []*core.Transaction
}

func newBlock(header *msg.MerkleBlock) *block {
	return &block{
		MerkleBlock: header,
		txQueue:     make(map[common.Uint256]struct{}),
	}
}

type SPVPeerConfig struct {
	// LocalHeight is invoked when peer queue a ping or pong message
	LocalHeight func() uint32

	// After send a blocks request message, this inventory message
	// will return with a bunch of block hashes, then you can use them
	// to request all the blocks by send data requests.
	OnInventory func(*SPVPeer, *msg.Inventory) error

	// After sent a data request with invType BLOCK, a merkleblock message will return through this method.
	// To make this work, you must register a filterload message to the connected peer first,
	// then this client will be known as a SPV client. To create a bloom filter and get the
	// filterload message, you will use the method in SDK bloom sdk.NewBloomFilter()
	// merkleblock includes a block header, transaction hashes in merkle proof format.
	// Which transaction hashes will be in the merkleblock is depends on the addresses and outpoints
	// you've added into the bloom filter before you send a filterload message with this bloom filter.
	// You will use these transaction hashes to request transactions by sending data request message
	// with invType TRANSACTION
	OnMerkleBlock func(*SPVPeer, *msg.MerkleBlock) error

	// After sent a data request with invType TRANSACTION, a txn message will return through this method.
	// these transactions are matched to the bloom filter you have sent with the filterload message.
	OnTx func(*SPVPeer, *msg.Tx) error

	// If the BLOCK or TRANSACTION requested by the data request message can not be found,
	// notfound message with requested data hash will return through this method.
	OnNotFound func(*SPVPeer, *msg.NotFound) error

	// If the submitted transaction was rejected, this message will return.
	OnReject func(*SPVPeer, *msg.Reject) error
}

type SPVPeer struct {
	*net.Peer

	blockQueue  chan common.Uint256
	blocksQueue map[common.Uint256]*block
	blockTxs    map[common.Uint256]common.Uint256
	txsQueue    map[common.Uint256]struct{}
	receivedTxs int
	fPositives  int

	stallControl chan p2p.Message
}

func NewSPVPeer(peer *net.Peer, config SPVPeerConfig) *SPVPeer {
	spvPeer := &SPVPeer{
		Peer:         peer,
		blockQueue:   make(chan common.Uint256, p2p.MaxBlocksPerMsg),
		blocksQueue:  make(map[common.Uint256]*block),
		blockTxs:     make(map[common.Uint256]common.Uint256),
		txsQueue:     make(map[common.Uint256]struct{}),
		stallControl: make(chan p2p.Message, 1),
	}

	msgConfig := rw.Config{
		ProtocolVersion: p2p.EIP001Version,
		MakeTx:          func() *msg.Tx { return msg.NewTx(new(core.Transaction)) },
		MakeBlock:       func() *msg.Block { return msg.NewBlock(new(core.Block)) },
		MakeMerkleBlock: func() *msg.MerkleBlock { return msg.NewMerkleBlock(new(core.Header)) },
	}

	spvPeer.SetMessageConfig(msgConfig)

	peerConfig := net.PeerConfig{
		PingNonce: config.LocalHeight,

		PongNonce: config.LocalHeight,

		OnPing: func(peer *net.Peer, ping *msg.Ping) {
			peer.SetHeight(ping.Nonce)
		},

		OnPong: func(peer *net.Peer, pong *msg.Pong) {
			peer.SetHeight(pong.Nonce)
		},

		HandleMessage: func(peer *net.Peer, message p2p.Message) {
			// Notify stall control
			spvPeer.stallControl <- message

			switch m := message.(type) {
			case *msg.Inventory:
				config.OnInventory(spvPeer, m)

			case *msg.MerkleBlock:
				config.OnMerkleBlock(spvPeer, m)

			case *msg.Tx:
				config.OnTx(spvPeer, m)

			case *msg.NotFound:
				config.OnNotFound(spvPeer, m)

			case *msg.Reject:
				config.OnReject(spvPeer, m)
			}
		},
	}

	spvPeer.SetPeerConfig(peerConfig)

	go spvPeer.stallHandler()

	return spvPeer
}

func (p *SPVPeer) stallHandler() {
	// stallTicker is used to periodically check pending responses that have
	// exceeded the expected deadline and disconnect the peer due to stalling.
	stallTicker := time.NewTicker(stallTickInterval)
	defer stallTicker.Stop()

	// pendingResponses tracks the expected responses.
	pendingResponses := make(map[string]struct{})

	// lastActive tracks the last active sync message.
	var lastActive time.Time

	for p.Connected() {
		select {
		case ctrMsg := <-p.stallControl:
			// update last active time
			lastActive = time.Now()

			switch message := ctrMsg.(type) {
			case *msg.GetBlocks:
				// Add expected response
				pendingResponses[p2p.CmdInv] = struct{}{}

			case *msg.Inventory:
				// Remove inventory from expected response map.
				delete(pendingResponses, p2p.CmdInv)

			case *msg.GetData:
				// Add expected responses
				for _, iv := range message.InvList {
					pendingResponses[iv.Hash.String()] = struct{}{}
				}

			case *msg.MerkleBlock:
				// Remove received merkleblock from expected response map.
				delete(pendingResponses, message.Header.(*core.Header).Hash().String())

			case *msg.Tx:
				// Remove received transaction from expected response map.
				delete(pendingResponses, message.Transaction.(*core.Transaction).Hash().String())

			case *msg.NotFound:
				// NotFound should not received from sync peer
				p.Disconnect()
			}

		case <-stallTicker.C:
			// There are no pending responses
			if len(pendingResponses) == 0 {
				continue
			}

			// Disconnect the peer if any of the pending responses
			// don't arrive by their adjusted deadline.
			if time.Now().Before(lastActive.Add(stallResponseTimeout)) {
				continue
			}

			log.Debugf("peer %v appears to be stalled or misbehaving, response timeout -- disconnecting", p)
			p.Disconnect()
		}
	}

	// Drain any wait channels before going away so there is nothing left
	// waiting on this goroutine.
cleanup:
	for {
		select {
		case <-p.stallControl:
		default:
			break cleanup
		}
	}
	log.Tracef("Peer stall handler done for %v", p)
}

// Add message to output queue and wait until message sent
func (p *SPVPeer) SendMessage(message p2p.Message) {
	doneChan := make(chan struct{})
	p.queueMessage(message, doneChan)
	<-doneChan
}

// Add a message into output queue
func (p *SPVPeer) QueueMessage(message p2p.Message) {
	p.queueMessage(message, nil)
}

func (p *SPVPeer) queueMessage(message p2p.Message, doneChan chan struct{}) {
	switch message.(type) {
	case *msg.GetBlocks, *msg.GetData:
		p.stallControl <- message
	}
	p.Peer.QueueMessage(message, doneChan)
}

func (p *SPVPeer) GetFalsePositiveRate() float32 {
	return float32(p.fPositives) / float32(p.receivedTxs)
}

func (p *SPVPeer) ResetFalsePositives() {
	p.fPositives, p.receivedTxs = 0, 0
}
