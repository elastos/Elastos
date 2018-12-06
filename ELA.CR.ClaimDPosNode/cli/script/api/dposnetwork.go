package api

import (
	"bytes"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA/core/types"
	. "github.com/elastos/Elastos.ELA/dpos/manager"
	"github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/yuin/gopher-lua"
)

const (
	luaNetworkTypeName = "dpos_network"
)

func RegisterDposNetworkType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaNetworkTypeName)
	L.SetGlobal("dpos_network", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newDposNetwork))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), networkMethods))
}

// Constructor
func newDposNetwork(L *lua.LState) int {

	n := &network{}

	ud := L.NewUserData()
	ud.Value = n
	L.SetMetatable(ud, L.GetTypeMetatable(luaNetworkTypeName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *Attribute and returns this *Attribute.
func checkDposNetwork(L *lua.LState, idx int) networkMock {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(networkMock); ok {
		return v
	}
	L.ArgError(1, "dpos network expected")
	return nil
}

var networkMethods = map[string]lua.LGFunction{
	"push_block":    networkPushBlock,
	"push_confirm":  networkPushConfirm,
	"push_vote":     networkPushVote,
	"push_proposal": networkPushProposal,

	"check_last_msg": networkCheckLastMsg,
	"check_last_pid": networkCheckLastPid,
}

func networkPushBlock(L *lua.LState) int {
	n := checkDposNetwork(L, 1)
	block := checkBlock(L, 2)
	confirm := L.ToBool(3)
	n.FireBlockReceived(block, confirm)

	return 0
}

func networkPushConfirm(L *lua.LState) int {
	n := checkDposNetwork(L, 1)
	confirm := checkConfirm(L, 2)
	n.FireConfirmReceived(confirm)

	return 0
}

func networkPushVote(L *lua.LState) int {
	n := checkDposNetwork(L, 1, )
	puk := L.ToString(2)
	vote := checkVote(L, 3)

	if vote.Accept {
		n.FireVoteReceived(pidFromString(puk), *vote)
	} else {
		n.FireVoteRejected(pidFromString(puk), *vote)
	}

	return 0
}

func networkPushProposal(L *lua.LState) int {
	n := checkDposNetwork(L, 1)
	puk := L.ToString(2)
	proposal := checkProposal(L, 3)

	n.FireProposalReceived(pidFromString(puk), *proposal)

	return 0
}

func networkCheckLastMsg(L *lua.LState) int {
	n := checkDposNetwork(L, 1)
	if n.GetLastMessage() != nil {

		cmd := L.ToString(2)

		result := false
		switch cmd {
		case msg.CmdReceivedProposal:
			if proposalMsg, ok := n.GetLastMessage().(*msg.Proposal); ok {
				proposal := checkProposal(L, 3)
				result = proposalMsg.Proposal.Hash().IsEqual(proposal.Hash())
			}
		case msg.CmdAcceptVote:
		case msg.CmdRejectVote:
			if voteMsg, ok := n.GetLastMessage().(*msg.Vote); ok {
				vote := checkVote(L, 3)
				result = voteMsg.Vote.Hash().IsEqual(vote.Hash())
			}
		}
		L.Push(lua.LBool(result))
	}

	return 1
}

func networkCheckLastPid(L *lua.LState) int {
	n := checkDposNetwork(L, 1)
	pidStr := L.ToString(2)

	result := false
	if n.GetLastPID() == nil {
		result = len(pidStr) == 0
	} else {
		if pk, err := common.HexStringToBytes(pidStr); err == nil {
			result = bytes.Equal(pk[:], n.GetLastPID()[:])
		}
	}
	L.Push(lua.LBool(result))

	return 1
}

func pidFromString(pub string) peer.PID {
	pk, _ := common.HexStringToBytes(pub)
	var id peer.PID
	copy(id[:], pk)

	return id
}

type networkMock interface {
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
