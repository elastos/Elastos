package arbitrator

import (
	"bytes"
	"encoding/binary"
	"errors"

	"github.com/elastos/Elastos.ELA/core"
	common2 "github.com/elastos/Elastos.ELA/dpos/arbitration/common"
	. "github.com/elastos/Elastos.ELA/dpos/arbitration/cs"
	"github.com/elastos/Elastos.ELA/dpos/chain"
	"github.com/elastos/Elastos.ELA/dpos/config"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/peer"
)

type StatusSyncEventListener interface {
	OnPing(peer *peer.Peer, height uint32)
	OnPong(peer *peer.Peer, height uint32)
	OnGetBlocks(peer *peer.Peer, startBlockHeight, endBlockHeight uint32)
	OnResponseBlocks(peer *peer.Peer, blocks []*core.Block, blockConfirms []*chain.ProposalVoteSlot)
	OnRequestConsensus(peer *peer.Peer, height uint32)
	OnResponseConsensus(peer *peer.Peer, status *ConsensusStatus)
}

type EventListener interface {
	StatusSyncEventListener

	OnProposalReceived(peer *peer.Peer, p common2.DPosProposal)
	OnVoteReceived(peer *peer.Peer, p common2.DPosProposalVote)
	OnVoteRejected(peer *peer.Peer, p common2.DPosProposalVote)
}

type PeerConnectionPoolImpl struct {
	Listener    EventListener
	directPeers map[string]*peer.Peer //for direct send response message
}

func (p *PeerConnectionPoolImpl) HandleMessage(peer *peer.Peer, msg p2p.Message) (bool, error) {
	P2PClientSingleton.MessageQueue <- &MessageItem{Message: msg, Peer: peer}
	return false, nil
}

func (p *PeerConnectionPoolImpl) ProcessMessage(msgItem MessageItem) bool {
	processed := false
	msg := msgItem.Message
	switch msg.CMD() {
	case ReceivedProposal:
		_, processed = msg.(*ProposalMessage)
		if processed {
			p.Listener.OnProposalReceived(msgItem.Peer, msg.(*ProposalMessage).Proposal)
		}
	case AcceptVote:
		_, processed = msg.(*VoteMessage)
		if processed {
			p.Listener.OnVoteReceived(msgItem.Peer, msg.(*VoteMessage).Vote)
		}
	case RejectVote:
		_, processed = msg.(*VoteMessage)
		if processed {
			p.Listener.OnVoteRejected(msgItem.Peer, msg.(*VoteMessage).Vote)
		}
	case Ping:
		_, processed = msg.(*PingMessage)
		if processed {
			p.Listener.OnPing(msgItem.Peer, msg.(*PingMessage).Height)
		}
	case Pong:
		_, processed = msg.(*PongMessage)
		if processed {
			p.Listener.OnPing(msgItem.Peer, msg.(*PongMessage).Height)
		}
	case GetBlocks:
		_, processed = msg.(*GetBlocksMessage)
		if processed {
			messageObj := msg.(*GetBlocksMessage)
			p.Listener.OnGetBlocks(msgItem.Peer, messageObj.StartBlockHeight, messageObj.EndBlockHeight)
		}
	case ResponseBlocks:
		_, processed = msg.(*ResponseBlocksMessage)
		if processed {
			messageObj := msg.(*ResponseBlocksMessage)
			p.Listener.OnResponseBlocks(msgItem.Peer, messageObj.Blocks, messageObj.BlockConfirms)
		}
	case RequestConsensus:
		_, processed = msg.(*RequestConsensusMessage)
		if processed {
			p.Listener.OnRequestConsensus(msgItem.Peer, msg.(*RequestConsensusMessage).Height)
		}
	case ResponseConsensus:
		_, processed = msg.(*ResponseConsensusMessage)
		if processed {
			p.Listener.OnResponseConsensus(msgItem.Peer, &msg.(*ResponseConsensusMessage).Consensus)
		}
	}
	return processed
}

func (p *PeerConnectionPoolImpl) GetLastActivePeer() (*peer.Peer, error) {
	p.UpdatePeers()
	for _, peer := range p.directPeers {
		return peer, nil
	}

	return nil, errors.New("[GetLastActivePeer], No active peer.")
}

func (p *PeerConnectionPoolImpl) SendAll(msg p2p.Message) {
	p.UpdatePeers()
	for _, v := range p.directPeers {
		v.SendMessage(msg, nil)
	}
}

func (p *PeerConnectionPoolImpl) Send(msg p2p.Message, peerName string) {
	if peer, ok := p.directPeers[peerName]; ok {
		peer.SendMessage(msg, nil)
	}
}

func (p *PeerConnectionPoolImpl) UpdatePeers() error {
	directPeers := make(map[string]*peer.Peer)
	arbitrators := GetOtherArbitrators()
	for _, v := range arbitrators {
		if peer, ok := GetPeerByName(v); ok {
			directPeers[v] = peer
		}
	}

	//todo call p2p new interface to remove removingPeers and add addingPeersNames

	p.directPeers = directPeers
	return nil
}

func GetOtherArbitrators() []string {
	arbitrators := ArbitratorGroupSingleton.Arbitrators //todo lock get Arbitrators operation
	arbiters := make([]string, 0)
	for _, a := range arbitrators {
		if a != config.Parameters.Name {
			arbiters = append(arbiters, a)
		}
	}
	return arbiters
}

func GetPeerByName(arbitratorName string) (*peer.Peer, bool) {
	peers := P2PClientSingleton.Server.ConnectedPeers()
	for _, p := range peers {
		if peerEqual(p.ToPeer(), arbitratorName) {
			return p.ToPeer(), true
		}
	}
	return nil, false
}

func GetPeerName(peer *peer.Peer) (string, bool) {
	arbitrators := GetOtherArbitrators()
	for _, n := range arbitrators {
		if peerEqual(peer, n) {
			return n, true
		}
	}
	return "", false
}

func peerEqual(peer *peer.Peer, arbitratorName string) bool {
	var clientId uint64
	buf := new(bytes.Buffer)
	buf.WriteString(arbitratorName)
	bytes := buf.Bytes()
	for i := len(buf.Bytes()); i < 8; i++ {
		bytes = append(bytes, byte(0))
	}
	clientId = binary.BigEndian.Uint64(bytes)

	if peer.ID() == clientId {
		return true
	}

	return false
}
