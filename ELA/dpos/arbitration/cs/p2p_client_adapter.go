package cs

import (
	"bytes"
	"errors"
	"fmt"
	"sync"

	"github.com/elastos/Elastos.ELA/dpos/chain"
	"github.com/elastos/Elastos.ELA/dpos/config"
	"github.com/elastos/Elastos.ELA/dpos/log"

	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA.Utility/p2p/peer"
	"github.com/elastos/Elastos.ELA.Utility/p2p/server"
	"github.com/elastos/Elastos.ELA/core"
)

var (
	P2PClientSingleton *p2pClientAdapter
)

const (
	OpenService        = 1 << 2
	messageStoreHeight = 5
	defaultMaxPeers    = 12
)

const (
	//len of message need to less than 12
	ReceivedBlock     = "block"
	ReceivedProposal  = "proposal"
	AcceptVote        = "acc_vote"
	RejectVote        = "rej_vote"
	ReceivedConfirm   = "confirm"
	Ping              = "ping"
	Pong              = "pong"
	GetBlocks         = "get_blc"
	ResponseBlocks    = "res_blc"
	RequestConsensus  = "req_con"
	ResponseConsensus = "res_con"
)

type MessageItem struct {
	Message p2p.Message
	Peer    *peer.Peer
}

type P2PListener interface {
	OnBlockReceived(peer *peer.Peer, b *chain.Block)
	OnConfirmReceived(peer *peer.Peer, p *chain.ProposalVoteSlot)
}

type PeerConnectionPool interface {
	UpdatePeers() error

	SendAll(msg p2p.Message)
	Send(msg p2p.Message, peerName string)
}

type PeerHandler interface {
	PeerConnectionPool
	HandleMessage(peer *peer.Peer, msg p2p.Message) (bool, error)
	ProcessMessage(msgItem MessageItem) bool

	GetLastActivePeer() (*peer.Peer, error)
}

type p2pClientAdapter struct {
	Server server.IServer

	messageHashes     map[common.Uint256]uint32
	cacheLock         sync.Mutex
	handleMessageLock sync.Mutex
	MessageQueue      chan *MessageItem
	PeerHandler       PeerHandler

	Listener P2PListener

	newPeers  chan *peer.Peer
	donePeers chan *peer.Peer
	quit      chan struct{}
}

func (c *p2pClientAdapter) newPeer(peer server.IPeer) {
	c.newPeers <- peer.ToPeer()
}

func (c *p2pClientAdapter) donePeer(peer server.IPeer) {
	c.donePeers <- peer.ToPeer()
}

// peerHandler handles new peers and done peers from P2P Server.
// When comes new peer, create a spv peer warpper for it
func (c *p2pClientAdapter) peerHandler() {
	peers := make(map[*peer.Peer]struct{})

out:
	for {
		select {
		case p := <-c.newPeers:
			log.Debugf("p2pclient new peer %v", p)
			p.AddMessageFunc(c.handleMessage)

		case p := <-c.donePeers:
			_, ok := peers[p]
			if !ok {
				log.Errorf("unknown done peer %v", p)
				continue
			}

			log.Debugf("p2pclient done peer %v", p)

		case <-c.quit:
			break out
		}
	}

	// Drain any wait channels before we go away so we don't leave something
	// waiting for us.
cleanup:
	for {
		select {
		case <-c.newPeers:
		case <-c.donePeers:
		default:
			break cleanup
		}
	}
	log.Debug("Service peers handler done")
}

func (adapter *p2pClientAdapter) OnP2PReceived(peer *peer.Peer, msg p2p.Message) error {
	if adapter.Listener == nil {
		return errors.New("listener is null")
	}
	adapter.MessageQueue <- &MessageItem{msg, peer}
	return nil
}

func (adapter *p2pClientAdapter) ProcessMessage(msgItem MessageItem) {
	if adapter.PeerHandler != nil {
		if ok := adapter.PeerHandler.ProcessMessage(msgItem); ok {
			return
		}
	}

	switch msgItem.Message.CMD() {
	case ReceivedBlock:
		blockMsg, ok := msgItem.Message.(*BlockMessage)
		if ok {
			adapter.Listener.OnBlockReceived(msgItem.Peer, &blockMsg.Block)
		}
		adapter.Broadcast(msgItem.Message)
	case ReceivedConfirm:
		confirmMsg, ok := msgItem.Message.(*ConfirmMessage)
		if ok {
			adapter.Listener.OnConfirmReceived(msgItem.Peer, &confirmMsg.Proposal)
		}
		adapter.Broadcast(msgItem.Message)
	default:
		log.Error("[ProcessMessage] unknown message:", msgItem.Message.CMD())
	}
}

func InitP2PClient() error {
	maxPeers := config.Parameters.MaxConnections
	if maxPeers == 0 {
		maxPeers = defaultMaxPeers
	}
	a := p2pClientAdapter{
		messageHashes: make(map[common.Uint256]uint32, 0),
		newPeers:      make(chan *peer.Peer, maxPeers),
		donePeers:     make(chan *peer.Peer, maxPeers),
	}

	//var clientId uint64
	//buf := new(bytes.Buffer)
	//buf.WriteString(config.Parameters.Name)
	//bytes := buf.Bytes()
	//for i := len(buf.Bytes()); i < 8; i++ {
	//	bytes = append(bytes, byte(0))
	//}
	//clientId = binary.BigEndian.Uint64(bytes)

	//todo set peer default id

	// Initiate P2P Server configuration
	serverCfg := server.NewDefaultConfig(
		config.Parameters.Magic,
		p2p.EIP001Version,
		OpenService,
		config.Parameters.NodePort,
		config.Parameters.SeedList,
		[]string{fmt.Sprint("127.0.0.1:", config.Parameters.NodePort)},
		a.newPeer, a.donePeer,
		makeEmptyMessage,
		func() uint64 { return uint64(0) },
	)
	serverCfg.MaxPeers = maxPeers
	log.Info("Server config:", serverCfg)

	var err error
	a.Server, err = server.NewServer(serverCfg)
	if err != nil {
		return err
	}

	P2PClientSingleton = &a

	return nil
}

func (adapter *p2pClientAdapter) Start() {
	go func() {
		P2PClientSingleton.Server.Start()
	}()
}

func (adapter *p2pClientAdapter) Work() {
	adapter.MessageQueue = make(chan *MessageItem, 10000)
	go func() {
		for {
			select {
			case msg := <-adapter.MessageQueue:
				adapter.ProcessMessage(*msg)
			}
		}
	}()
}

func (adapter *p2pClientAdapter) GetMessageHash(msg p2p.Message) common.Uint256 {
	buf := new(bytes.Buffer)
	msg.Serialize(buf)
	msgHash := common.Sha256D(buf.Bytes())
	return msgHash
}

func (adapter *p2pClientAdapter) ExistMessageHash(msgHash common.Uint256) bool {
	adapter.cacheLock.Lock()
	defer adapter.cacheLock.Unlock()
	if _, ok := adapter.messageHashes[msgHash]; ok {
		return true
	}
	return false
}

func (adapter *p2pClientAdapter) AddMessageHash(msgHash common.Uint256, currentMainChainHeight uint32) bool {
	adapter.cacheLock.Lock()
	defer adapter.cacheLock.Unlock()
	adapter.messageHashes[msgHash] = currentMainChainHeight

	//delete message height 5 less than current main chain height
	//var needToDeleteMessages []common.Uint256
	//for k, v := range adapter.messageHashes {
	//	if currentMainChainHeight >= 5 && v < currentMainChainHeight-MessageStoreHeight {
	//		needToDeleteMessages = append(needToDeleteMessages, k)
	//	}
	//}
	//for _, msg := range needToDeleteMessages {
	//	delete(adapter.messageHashes, msg)
	//}

	return false
}

func (adapter *p2pClientAdapter) Broadcast(msg p2p.Message) {
	log.Debug("Broadcast peers", adapter.Server.ConnectedPeers())
	adapter.Server.BroadcastMessage(msg)
}

func (adapter *p2pClientAdapter) handleMessage(peer *peer.Peer, msg p2p.Message) {
	msgHash := adapter.GetMessageHash(msg)

	adapter.handleMessageLock.Lock()
	if adapter.ExistMessageHash(msgHash) {
		log.Debug("received exist message")
		adapter.handleMessageLock.Unlock()
		return
	} else {
		var currentHeight uint32
		/*if config.Parameters.IsProducer {
			currentHeight = producer.ProducerSingleton.CurrentHeight
		} else {
			currentHeight = dpos.ArbitratorSingleton.Leger.LastBlock.Height
		}*/
		adapter.AddMessageHash(msgHash, currentHeight)
		log.Info("Received MSG:", msg.CMD(), "hash:", msgHash)
	}
	adapter.handleMessageLock.Unlock()

	if err := adapter.OnP2PReceived(peer, msg); err != nil {
		log.Warn(err)
	}

	return
}

func makeEmptyMessage(cmd string) (message p2p.Message, err error) {
	switch cmd {
	case p2p.CmdInv:
		message = new(msg.Inv)
	case p2p.CmdGetData:
		message = new(msg.GetData)
	case p2p.CmdNotFound:
		message = new(msg.NotFound)
	case p2p.CmdTx:
		message = msg.NewTx(new(core.Transaction))
	case p2p.CmdMerkleBlock:
		message = msg.NewMerkleBlock(new(core.Header))
	case p2p.CmdReject:
		message = new(msg.Reject)
	case ReceivedBlock:
		message = &BlockMessage{Command: ReceivedBlock}
	case ReceivedConfirm:
		message = &ConfirmMessage{Command: ReceivedConfirm}
	case AcceptVote:
		message = &VoteMessage{Command: AcceptVote}
	case ReceivedProposal:
		message = &ProposalMessage{Command: ReceivedProposal}
	case RejectVote:
		message = &VoteMessage{Command: RejectVote}
	case Ping:
		message = &PingMessage{Command: Ping}
	case Pong:
		message = &PongMessage{Command: Pong}
	case GetBlocks:
		message = &GetBlocksMessage{Command: GetBlocks}
	case ResponseBlocks:
		message = &ResponseBlocksMessage{Command: ResponseBlocks}
	case RequestConsensus:
		message = &RequestConsensusMessage{Command: RequestConsensus}
	case ResponseConsensus:
		message = &ResponseConsensusMessage{Command: ResponseConsensus}
	default:
		return nil, errors.New("Received unsupported message, CMD " + cmd)
	}
	return message, nil
}

func (adapter *p2pClientAdapter) OnHandshake(v *msg.Version) error {
	if v.Version < sdk.ProtocolVersion {
		return errors.New(fmt.Sprint("To support SPV protocol, peer version must greater than ", sdk.ProtocolVersion))
	}

	return nil
}

func (adapter *p2pClientAdapter) OnPeerEstablish(peer *peer.Peer) {
}
