package dpos

import (
	"errors"
	"math/rand"

	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/dpos/log"
	"github.com/elastos/Elastos.ELA/dpos/manager"
	"github.com/elastos/Elastos.ELA/dpos/p2p"
	"github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"

	"github.com/elastos/Elastos.ELA.Utility/common"
	utip2p "github.com/elastos/Elastos.ELA.Utility/p2p"
)

type PeerItem struct {
	Address     p2p.PeerAddr
	NeedConnect bool
	Peer        *peer.Peer
	VoteStatus  interface{}
	Sequence    uint32
}

type blockItem struct {
	Block     *core.Block
	Confirmed bool
}

type messageItem struct {
	ID      common.Uint256
	Message utip2p.Message
}

type dposNetwork struct {
	listener      manager.NetworkEventListener
	directPeers   map[string]PeerItem
	currentHeight uint32

	p2pServer    p2p.Server
	messageQueue chan *messageItem
	quit         chan bool

	changeViewChan      chan bool
	blockReceivedChan   chan blockItem
	confirmReceivedChan chan *core.DPosProposalVoteSlot
}

func (n *dposNetwork) Start() {
	n.p2pServer.Start()
	n.connectPeers()

	go func() {
	out:
		for {
			select {
			case msgItem := <-n.messageQueue:
				n.processMessage(msgItem)
			case <-n.changeViewChan:
				n.changeView()
			case blockItem := <-n.blockReceivedChan:
				n.blockReceived(blockItem.Block, blockItem.Confirmed)
			case confirm := <-n.confirmReceivedChan:
				n.confirmReceived(confirm)
			case <-n.quit:
				break out
			}
		}
	}()
}

func (n *dposNetwork) connectPeers() {
	addrList := make([]p2p.PeerAddr, 0)
	for _, seed := range config.Parameters.ArbiterConfiguration.SeedList {
		publicKey, err := common.HexStringToBytes(seed.PublicKey)
		if err != nil || len(publicKey) != 33 {
			log.Errorf("connect to arbiter %s failed: %v:", seed.PublicKey, err)
			continue
		}

		var pid [32]byte
		copy(pid[:], publicKey[1:])
		addrList = append(addrList, p2p.PeerAddr{
			PID:  pid,
			Addr: seed.Addrress,
		})
	}
	n.p2pServer.ConnectPeers(addrList)
}

func (n *dposNetwork) Stop() error {
	n.quit <- true
	return n.p2pServer.Stop()
}

func (n *dposNetwork) Reset(epochInfo interface{}) error {
	//todo update direct peers, including current arbitrators and candidate arbitrators
	return nil
}

func (n *dposNetwork) SendMessageToPeer(id common.Uint256, msg utip2p.Message) error {
	return n.p2pServer.SendMessageToPeer(id, msg)
}

func (n *dposNetwork) BroadcastMessage(msg utip2p.Message) {
	n.p2pServer.BroadcastMessage(msg)
}

func (n *dposNetwork) ChangeHeight(height uint32) error {
	if height < n.currentHeight {
		return errors.New("Changing height lower than current height")
	}

	offset := height - n.currentHeight
	if offset == 0 {
		return nil
	}

	for _, v := range n.directPeers {
		if v.Sequence <= offset {
			v.NeedConnect = false
			v.Sequence = 0
			continue
		}

		v.Sequence -= offset
	}

	n.p2pServer.ConnectPeers(n.getValidPeers())
	return nil
}

func (n *dposNetwork) GetActivePeer() *common.Uint256 {
	peers := n.p2pServer.ConnectedPeers()
	if len(peers) == 0 {
		return nil
	}
	i := rand.Int31n(int32(len(peers)) - 1)
	id := peers[i].PID()
	return &id
}

func (n *dposNetwork) PostChangeViewTask() {
	n.changeViewChan <- true
}

func (n *dposNetwork) PostBlockReceivedTask(b *core.Block, confirmed bool) {
	n.blockReceivedChan <- blockItem{b, confirmed}
}

func (n *dposNetwork) PostConfirmReceivedTask(p *core.DPosProposalVoteSlot) {
	n.confirmReceivedChan <- p
}

func (n *dposNetwork) getValidPeers() (result []p2p.PeerAddr) {
	result = make([]p2p.PeerAddr, 0)
	for _, v := range n.directPeers {
		if v.NeedConnect {
			result = append(result, v.Address)
		}
	}
	return result
}

func (n *dposNetwork) notifyFlag(flag p2p.NotifyFlag) {
	if flag == p2p.NFBadNetwork {
		n.listener.OnBadNetwork()
	}
}

func (n *dposNetwork) handleMessage(pid common.Uint256, msg utip2p.Message) {
	n.messageQueue <- &messageItem{pid, msg}
}

func (n *dposNetwork) processMessage(msgItem *messageItem) {
	processed := false
	m := msgItem.Message
	switch m.CMD() {
	case msg.ReceivedProposal:
		_, processed = m.(*msg.ProposalMessage)
		if processed {
			n.listener.OnProposalReceived(msgItem.ID, m.(*msg.ProposalMessage).Proposal)
		}
	case msg.AcceptVote:
		_, processed = m.(*msg.VoteMessage)
		if processed {
			n.listener.OnVoteReceived(msgItem.ID, m.(*msg.VoteMessage).Vote)
		}
	case msg.RejectVote:
		_, processed = m.(*msg.VoteMessage)
		if processed {
			n.listener.OnVoteRejected(msgItem.ID, m.(*msg.VoteMessage).Vote)
		}
	case msg.CmdPing:
		_, processed = m.(*msg.Ping)
		if processed {
			n.listener.OnPing(msgItem.ID, uint32(m.(*msg.Ping).Nonce))
		}
	case msg.CmdPong:
		_, processed = m.(*msg.Pong)
		if processed {
			n.listener.OnPong(msgItem.ID, uint32(m.(*msg.Pong).Nonce))
		}
	case msg.GetBlocks:
		_, processed = m.(*msg.GetBlocksMessage)
		if processed {
			messageObj := m.(*msg.GetBlocksMessage)
			n.listener.OnGetBlocks(msgItem.ID, messageObj.StartBlockHeight, messageObj.EndBlockHeight)
		}
	case msg.ResponseBlocks:
		_, processed = m.(*msg.ResponseBlocksMessage)
		if processed {
			messageObj := m.(*msg.ResponseBlocksMessage)
			n.listener.OnResponseBlocks(msgItem.ID, messageObj.Blocks, messageObj.BlockConfirms)
		}
	case msg.RequestConsensus:
		_, processed = m.(*msg.RequestConsensusMessage)
		if processed {
			n.listener.OnRequestConsensus(msgItem.ID, m.(*msg.RequestConsensusMessage).Height)
		}
	case msg.ResponseConsensus:
		_, processed = m.(*msg.ResponseConsensusMessage)
		if processed {
			n.listener.OnResponseConsensus(msgItem.ID, &m.(*msg.ResponseConsensusMessage).Consensus)
		}
	}
}

func (n *dposNetwork) changeView() {
	n.listener.OnChangeView()
}

func (n *dposNetwork) blockReceived(b *core.Block, confirmed bool) {
	n.listener.OnBlockReceived(b, confirmed)
}

func (n *dposNetwork) confirmReceived(p *core.DPosProposalVoteSlot) {
	n.listener.OnConfirmReceived(p)
}

func (n *dposNetwork) getCurrentHeight(pid common.Uint256) uint64 {
	//todo get current height from proposal dispatcher
	return 0
}

func NewDposNetwork(pid [32]byte, listener manager.NetworkEventListener) (*dposNetwork, error) {
	network := &dposNetwork{
		listener:       listener,
		directPeers:    make(map[string]PeerItem),
		messageQueue:   make(chan *messageItem, 10000), //todo config handle capacity though config file
		quit:           make(chan bool),
		changeViewChan: make(chan bool),
		currentHeight:  0,
	}

	notifier := p2p.NewNotifier(p2p.NFNetStabled|p2p.NFBadNetwork, network.notifyFlag)

	//fixme replace hard code config with config file
	server, err := p2p.NewServer(&p2p.Config{
		PID:              pid,
		MagicNumber:      config.Parameters.ArbiterConfiguration.Magic,
		ProtocolVersion:  config.Parameters.ArbiterConfiguration.ProtocolVersion,
		Services:         config.Parameters.ArbiterConfiguration.Services,
		DefaultPort:      config.Parameters.ArbiterConfiguration.NodePort,
		MakeEmptyMessage: makeEmptyMessage,
		HandleMessage:    network.handleMessage,
		PingNonce:        network.getCurrentHeight,
		PongNonce:        network.getCurrentHeight,
		StateNotifier:    notifier,
	})
	if err != nil {
		return nil, err
	}

	network.p2pServer = server
	return network, nil
}

func makeEmptyMessage(cmd string) (message utip2p.Message, err error) {
	switch cmd {
	case msg.AcceptVote:
		message = &msg.VoteMessage{Command: msg.AcceptVote}
	case msg.ReceivedProposal:
		message = &msg.ProposalMessage{}
	case msg.RejectVote:
		message = &msg.VoteMessage{Command: msg.RejectVote}
	case msg.GetBlocks:
		message = &msg.GetBlocksMessage{}
	case msg.ResponseBlocks:
		message = &msg.ResponseBlocksMessage{}
	case msg.RequestConsensus:
		message = &msg.RequestConsensusMessage{}
	case msg.ResponseConsensus:
		message = &msg.ResponseConsensusMessage{}
	default:
		return nil, errors.New("Received unsupported message, CMD " + cmd)
	}
	return message, nil
}
