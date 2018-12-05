package api

import (
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
func checkDposNetwork(L *lua.LState, idx int) *network {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*network); ok {
		return v
	}
	L.ArgError(1, "dpos network expected")
	return nil
}

var networkMethods = map[string]lua.LGFunction{
}

//mock object of dposNetwork
type network struct {
	listener NetworkEventListener
}

func (n *network) Initialize(proposalDispatcher ProposalDispatcher) {

}

func (n *network) Start() {

}

func (n *network) Stop() error {
	return nil
}

func (n *network) SendMessageToPeer(id peer.PID, msg p2p.Message) error {
	return nil
}

func (n *network) BroadcastMessage(msg p2p.Message) {

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
