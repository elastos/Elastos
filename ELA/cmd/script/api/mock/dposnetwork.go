package mock

import (
	"fmt"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	. "github.com/elastos/Elastos.ELA/dpos/manager"
	dp2p "github.com/elastos/Elastos.ELA/dpos/p2p"
	"github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"
	"github.com/elastos/Elastos.ELA/p2p"
)

type NetworkMock interface {
	DPOSNetwork
	SetListener(listener NetworkEventListener)

	FirePing(id peer.PID, height uint32)
	FirePong(id peer.PID, height uint32)
	FireBlock(id peer.PID, block *types.Block)
	FireInv(id peer.PID, blockHash common.Uint256)
	FireGetBlock(id peer.PID, blockHash common.Uint256)
	FireGetBlocks(id peer.PID, startBlockHeight, endBlockHeight uint32)
	FireResponseBlocks(id peer.PID, blockConfirms []*types.DposBlock)
	FireRequestConsensus(id peer.PID, height uint32)
	FireResponseConsensus(id peer.PID, status *msg.ConsensusStatus)
	FireRequestProposal(id peer.PID, hash common.Uint256)
	FireIllegalProposalReceived(id peer.PID, proposals *payload.DPOSIllegalProposals)
	FireIllegalVotesReceived(id peer.PID, votes *payload.DPOSIllegalVotes)
	FireProposalReceived(id peer.PID, p *payload.DPOSProposal)
	FireVoteReceived(id peer.PID, p *payload.DPOSProposalVote)
	FireVoteRejected(id peer.PID, p *payload.DPOSProposalVote)
	FireChangeView()
	FireBadNetwork()
	FireBlockReceived(b *types.Block, confirmed bool)
	FireConfirmReceived(p *payload.Confirm)
	FireIllegalBlocksReceived(i *payload.DPOSIllegalBlocks)

	GetLastMessage() p2p.Message
	GetLastPID() *peer.PID
	DumpMessages(level uint32) string
}

func NewNetworkMock() NetworkMock {
	return &network{messageList: make([]messageItem, 0)}
}

type messageItem struct {
	ID      *peer.PID
	Message p2p.Message
}

//mock object of dposNetwork
type network struct {
	listener    NetworkEventListener
	messageList []messageItem
}

func (n *network) RecoverTimeout() {
	panic("implement me")
}

func (n *network) DumpMessages(level uint32) string {
	result := ""

	switch level {
	case 0:
		for _, v := range n.messageList {
			result += fmt.Sprintln("[message] type=" + v.Message.CMD())
		}
	}

	return result
}

func (n *network) GetLastMessage() p2p.Message {
	if len(n.messageList) == 0 {
		return nil
	}
	return n.messageList[len(n.messageList)-1].Message
}

func (n *network) GetLastPID() *peer.PID {
	if len(n.messageList) == 0 {
		return nil
	}
	return n.messageList[len(n.messageList)-1].ID
}

func (n *network) SetListener(listener NetworkEventListener) {
	n.listener = listener
}

func (n *network) Initialize(dnConfig DPOSNetworkConfig) {

}

func (n *network) Start() {

}

func (n *network) Stop() error {
	return nil
}

func (n *network) SendMessageToPeer(id peer.PID, msg p2p.Message) error {
	n.messageList = append(n.messageList, messageItem{ID: &id, Message: msg})
	return nil
}

func (n *network) BroadcastMessage(msg p2p.Message) {
	n.messageList = append(n.messageList, messageItem{ID: nil, Message: msg})
}

func (n *network) UpdatePeers(peers []peer.PID) {}

func (n *network) GetActivePeers() []dp2p.Peer {
	return nil
}

func (n *network) FirePing(id peer.PID, height uint32) {
	n.listener.OnPing(id, height)
}

func (n *network) FirePong(id peer.PID, height uint32) {
	n.listener.OnPong(id, height)
}

func (n *network) FireBlock(id peer.PID, block *types.Block) {
	n.listener.OnBlock(id, block)
}

func (n *network) FireInv(id peer.PID, blockHash common.Uint256) {
	n.listener.OnInv(id, blockHash)
}

func (n *network) FireGetBlock(id peer.PID, blockHash common.Uint256) {
	n.listener.OnGetBlock(id, blockHash)
}

func (n *network) FireGetBlocks(id peer.PID, startBlockHeight, endBlockHeight uint32) {
	n.listener.OnGetBlocks(id, startBlockHeight, endBlockHeight)
}

func (n *network) FireResponseBlocks(id peer.PID, blockConfirms []*types.DposBlock) {
	n.listener.OnResponseBlocks(id, blockConfirms)
}

func (n *network) FireRequestConsensus(id peer.PID, height uint32) {
	n.listener.OnRequestConsensus(id, height)
}

func (n *network) FireResponseConsensus(id peer.PID, status *msg.ConsensusStatus) {
	n.listener.OnResponseConsensus(id, status)
}

func (n *network) FireRequestProposal(id peer.PID, hash common.Uint256) {
	n.listener.OnRequestProposal(id, hash)
}

func (n *network) FireIllegalProposalReceived(id peer.PID, proposals *payload.DPOSIllegalProposals) {
	n.listener.OnIllegalProposalReceived(id, proposals)
}

func (n *network) FireIllegalVotesReceived(id peer.PID, votes *payload.DPOSIllegalVotes) {
	n.listener.OnIllegalVotesReceived(id, votes)
}

func (n *network) FireProposalReceived(id peer.PID, p *payload.DPOSProposal) {
	n.listener.OnProposalReceived(id, p)
}

func (n *network) FireVoteReceived(id peer.PID, p *payload.DPOSProposalVote) {
	n.listener.OnVoteAccepted(id, p)
}

func (n *network) FireVoteRejected(id peer.PID, p *payload.DPOSProposalVote) {
	n.listener.OnVoteRejected(id, p)
}

func (n *network) FireChangeView() {
	n.listener.OnChangeView()
}

func (n *network) FireBadNetwork() {
	n.listener.OnBadNetwork()
}

func (n *network) FireBlockReceived(b *types.Block, confirmed bool) {
	n.listener.OnBlockReceived(b, confirmed)
}

func (n *network) FireConfirmReceived(p *payload.Confirm) {
	n.listener.OnConfirmReceived(p)
}

func (n *network) FireIllegalBlocksReceived(i *payload.DPOSIllegalBlocks) {
	n.listener.OnIllegalBlocksTxReceived(i)
}
