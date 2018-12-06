package mock

import (
	"github.com/elastos/Elastos.ELA/core/types"
	. "github.com/elastos/Elastos.ELA/dpos/manager"
	"github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

type NetworkMock interface {
	DposNetwork
	SetListener(listener NetworkEventListener)

	FirePing(id peer.PID, height uint32)
	FirePong(id peer.PID, height uint32)
	FireBlock(id peer.PID, block *types.Block)
	FireInv(id peer.PID, blockHash common.Uint256)
	FireGetBlock(id peer.PID, blockHash common.Uint256)
	FireGetBlocks(id peer.PID, startBlockHeight, endBlockHeight uint32)
	FireResponseBlocks(id peer.PID, blockConfirms []*types.BlockConfirm)
	FireRequestConsensus(id peer.PID, height uint32)
	FireResponseConsensus(id peer.PID, status *msg.ConsensusStatus)
	FireRequestProposal(id peer.PID, hash common.Uint256)
	FireIllegalProposalReceived(id peer.PID, proposals *types.DposIllegalProposals)
	FireIllegalVotesReceived(id peer.PID, votes *types.DposIllegalVotes)
	FireProposalReceived(id peer.PID, p types.DPosProposal)
	FireVoteReceived(id peer.PID, p types.DPosProposalVote)
	FireVoteRejected(id peer.PID, p types.DPosProposalVote)
	FireChangeView()
	FireBadNetwork()
	FireBlockReceived(b *types.Block, confirmed bool)
	FireConfirmReceived(p *types.DPosProposalVoteSlot)
	FireIllegalBlocksReceived(i *types.DposIllegalBlocks)

	GetLastMessage() p2p.Message
	GetLastPID() *peer.PID
}

func NewNetworkMock() NetworkMock {
	return &network{}
}

//mock object of dposNetwork
type network struct {
	listener    NetworkEventListener
	lastMessage p2p.Message
	lastPID     *peer.PID
}

func (n *network) GetLastMessage() p2p.Message {
	return n.lastMessage
}

func (n *network) GetLastPID() *peer.PID {
	return n.lastPID
}

func (n *network) SetListener(listener NetworkEventListener) {
	n.listener = listener
}

func (n *network) Initialize(proposalDispatcher ProposalDispatcher) {

}

func (n *network) Start() {

}

func (n *network) Stop() error {
	return nil
}

func (n *network) SendMessageToPeer(id peer.PID, msg p2p.Message) error {
	n.lastMessage = msg
	n.lastPID = &id
	return nil
}

func (n *network) BroadcastMessage(msg p2p.Message) {
	n.lastMessage = msg
	n.lastPID = nil
}

func (n *network) UpdatePeers(arbitrators [][]byte) error {
	return nil
}

func (n *network) ChangeHeight(height uint32) error {
	return nil
}

func (n *network) GetActivePeer() *peer.PID {
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

func (n *network) FireResponseBlocks(id peer.PID, blockConfirms []*types.BlockConfirm) {
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

func (n *network) FireIllegalProposalReceived(id peer.PID, proposals *types.DposIllegalProposals) {
	n.listener.OnIllegalProposalReceived(id, proposals)
}

func (n *network) FireIllegalVotesReceived(id peer.PID, votes *types.DposIllegalVotes) {
	n.listener.OnIllegalVotesReceived(id, votes)
}

func (n *network) FireProposalReceived(id peer.PID, p types.DPosProposal) {
	n.listener.OnProposalReceived(id, p)
}

func (n *network) FireVoteReceived(id peer.PID, p types.DPosProposalVote) {
	n.listener.OnVoteReceived(id, p)
}

func (n *network) FireVoteRejected(id peer.PID, p types.DPosProposalVote) {
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

func (n *network) FireConfirmReceived(p *types.DPosProposalVoteSlot) {
	n.listener.OnConfirmReceived(p)
}

func (n *network) FireIllegalBlocksReceived(i *types.DposIllegalBlocks) {
	n.listener.OnIllegalBlocksReceived(i)
}
