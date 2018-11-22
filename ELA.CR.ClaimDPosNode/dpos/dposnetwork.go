package dpos

import (
	"errors"
	"math/rand"
	"sync"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/dpos/account"
	"github.com/elastos/Elastos.ELA/dpos/log"
	"github.com/elastos/Elastos.ELA/dpos/manager"
	"github.com/elastos/Elastos.ELA/dpos/p2p"
	"github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"

	"github.com/elastos/Elastos.ELA.Utility/common"
	utip2p "github.com/elastos/Elastos.ELA.Utility/p2p"
	utimsg "github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

type PeerItem struct {
	Address     p2p.PeerAddr
	NeedConnect bool
	Peer        *peer.Peer
	Sequence    uint32
}

type blockItem struct {
	Block     *core.Block
	Confirmed bool
}

type messageItem struct {
	ID      peer.PID
	Message utip2p.Message
}

type dposNetwork struct {
	listener           manager.NetworkEventListener
	currentHeight      uint32
	account            account.DposAccount
	proposalDispatcher manager.ProposalDispatcher
	directPeers        map[string]PeerItem
	peersLock          sync.Mutex

	p2pServer    p2p.Server
	messageQueue chan *messageItem
	quit         chan bool

	changeViewChan      chan bool
	blockReceivedChan   chan blockItem
	confirmReceivedChan chan *core.DPosProposalVoteSlot
}

func (n *dposNetwork) Initialize(proposalDispatcher manager.ProposalDispatcher) {
	n.proposalDispatcher = proposalDispatcher
}

func (n *dposNetwork) Start() {
	n.p2pServer.Start()
	n.connectPeers()

	n.OnProducersChanged()
	n.UpdatePeers(blockchain.DefaultLedger.Arbitrators.GetArbitrators())

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

		var pid peer.PID
		copy(pid[:], publicKey)
		addrList = append(addrList, p2p.PeerAddr{
			PID:  pid,
			Addr: seed.Addrress,
		})
	}
	log.Info("[connectPeers] addr list:", addrList)
	n.p2pServer.ConnectPeers(addrList)
}

//todo fired when producers changed
func (n *dposNetwork) OnProducersChanged() {

	connectionInfoMap := n.getProducersConnectionInfo()

	n.peersLock.Lock()
	defer n.peersLock.Unlock()
	for k, v := range connectionInfoMap {

		if _, ok := n.directPeers[k]; !ok {
			n.directPeers[k] = PeerItem{
				Address:     v,
				NeedConnect: false,
				Peer:        nil,
				Sequence:    0,
			}
		}
	}
	//todo clean canceled producers connection info
}

func (n *dposNetwork) getProducersConnectionInfo() map[string]p2p.PeerAddr {
	//todo complete me
	return nil
}

func (n *dposNetwork) Stop() error {
	n.quit <- true
	return n.p2pServer.Stop()
}

func (n *dposNetwork) UpdatePeers(arbitrators [][]byte) error {

	for _, v := range arbitrators {
		pubKey := common.BytesToHexString(v)

		n.peersLock.Lock()
		ad, ok := n.directPeers[pubKey]
		n.peersLock.Unlock()

		if !ok {
			log.Error("Can not find arbitrator related connection information, arbitrator public key is: ", pubKey)
			continue
		}

		ad.NeedConnect = true
		ad.Sequence += uint32(len(arbitrators))
	}

	return nil
}

func (n *dposNetwork) SendMessageToPeer(id peer.PID, msg utip2p.Message) error {
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

	n.peersLock.Lock()
	for _, v := range n.directPeers {
		if v.Sequence <= offset {
			v.NeedConnect = false
			v.Sequence = 0
			continue
		}

		v.Sequence -= offset
	}

	n.p2pServer.ConnectPeers(n.getValidPeers())
	n.peersLock.Unlock()

	n.currentHeight = height
	return nil
}

func (n *dposNetwork) GetActivePeer() *peer.PID {
	peers := n.p2pServer.ConnectedPeers()
	if len(peers) == 0 {
		return nil
	}
	if len(peers) == 1 {
		id := peers[0].PID()
		return &id
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

func (n *dposNetwork) handleMessage(pid peer.PID, msg utip2p.Message) {
	n.messageQueue <- &messageItem{pid, msg}
}

func (n *dposNetwork) processMessage(msgItem *messageItem) {
	m := msgItem.Message
	switch m.CMD() {
	case msg.ReceivedProposal:
		msgProposal, processed := m.(*msg.ProposalMessage)
		if processed {
			n.listener.OnProposalReceived(msgItem.ID, msgProposal.Proposal)
		}
	case msg.AcceptVote:
		msgVote, processed := m.(*msg.VoteMessage)
		if processed {
			n.listener.OnVoteReceived(msgItem.ID, msgVote.Vote)
		}
	case msg.RejectVote:
		msgVote, processed := m.(*msg.VoteMessage)
		if processed {
			n.listener.OnVoteRejected(msgItem.ID, msgVote.Vote)
		}
	case msg.CmdPing:
		msgPing, processed := m.(*msg.Ping)
		if processed {
			n.listener.OnPing(msgItem.ID, uint32(msgPing.Nonce))
		}
	case msg.CmdPong:
		msgPong, processed := m.(*msg.Pong)
		if processed {
			n.listener.OnPong(msgItem.ID, uint32(msgPong.Nonce))
		}
	case utip2p.CmdBlock:
		msgBlock, processed := m.(*utimsg.Block)
		if processed {
			if block, ok := msgBlock.Serializable.(*core.Block); ok {
				n.listener.OnBlock(msgItem.ID, block)
			}
		}
	case msg.CmdInv:
		msgInv, processed := m.(*msg.Inventory)
		if processed {
			n.listener.OnInv(msgItem.ID, msgInv.BlockHash)
		}
	case msg.CmdGetBlock:
		msgGetBlock, processed := m.(*msg.GetBlock)
		if processed {
			n.listener.OnGetBlock(msgItem.ID, msgGetBlock.BlockHash)
		}
	case msg.GetBlocks:
		msgGetBlocks, processed := m.(*msg.GetBlocksMessage)
		if processed {
			n.listener.OnGetBlocks(msgItem.ID, msgGetBlocks.StartBlockHeight, msgGetBlocks.EndBlockHeight)
		}
	case msg.ResponseBlocks:
		msgResponseBlocks, processed := m.(*msg.ResponseBlocksMessage)
		if processed {
			n.listener.OnResponseBlocks(msgItem.ID, msgResponseBlocks.BlockConfirms)
		}
	case msg.RequestConsensus:
		msgRequestConsensus, processed := m.(*msg.RequestConsensusMessage)
		if processed {
			n.listener.OnRequestConsensus(msgItem.ID, msgRequestConsensus.Height)
		}
	case msg.ResponseConsensus:
		msgResponseConsensus, processed := m.(*msg.ResponseConsensusMessage)
		if processed {
			n.listener.OnResponseConsensus(msgItem.ID, &msgResponseConsensus.Consensus)
		}
	case msg.CmdRequestProposal:
		msgRequestProposal, processed := m.(*msg.RequestProposal)
		if processed {
			n.listener.OnRequestProposal(msgItem.ID, msgRequestProposal.ProposalHash)
		}
	case msg.CmdResponseProposal:
		msgResponseProposal, processed := m.(*msg.ResponseProposal)
		if processed {
			n.listener.OnResponseProposal(msgItem.ID, &msgResponseProposal.Proposal)
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

func (n *dposNetwork) getCurrentHeight(pid peer.PID) uint64 {
	return uint64(n.proposalDispatcher.CurrentHeight())
}

func NewDposNetwork(pid peer.PID, listener manager.NetworkEventListener, dposAccount account.DposAccount) (*dposNetwork, error) {
	network := &dposNetwork{
		listener:            listener,
		directPeers:         make(map[string]PeerItem),
		messageQueue:        make(chan *messageItem, 10000), //todo config handle capacity though config file
		quit:                make(chan bool),
		changeViewChan:      make(chan bool),
		blockReceivedChan:   make(chan blockItem, 10),                  //todo config handle capacity though config file
		confirmReceivedChan: make(chan *core.DPosProposalVoteSlot, 10), //todo config handle capacity though config file
		currentHeight:       0,
		account:             dposAccount,
	}

	notifier := p2p.NewNotifier(p2p.NFNetStabled|p2p.NFBadNetwork, network.notifyFlag)

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
		SignNonce:        dposAccount.SignPeerNonce,
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
	case utip2p.CmdBlock:
		message = utimsg.NewBlock(&core.BlockConfirm{})
	case msg.AcceptVote:
		message = &msg.VoteMessage{Command: msg.AcceptVote}
	case msg.ReceivedProposal:
		message = &msg.ProposalMessage{}
	case msg.RejectVote:
		message = &msg.VoteMessage{Command: msg.RejectVote}
	case msg.CmdInv:
		message = &msg.Inventory{}
	case msg.CmdGetBlock:
		message = &msg.GetBlock{}
	case msg.GetBlocks:
		message = &msg.GetBlocksMessage{}
	case msg.ResponseBlocks:
		message = &msg.ResponseBlocksMessage{}
	case msg.RequestConsensus:
		message = &msg.RequestConsensusMessage{}
	case msg.ResponseConsensus:
		message = &msg.ResponseConsensusMessage{}
	case msg.CmdRequestProposal:
		message = &msg.RequestProposal{}
	case msg.CmdResponseProposal:
		message = &msg.ResponseProposal{}
	default:
		return nil, errors.New("Received unsupported message, CMD " + cmd)
	}
	return message, nil
}
