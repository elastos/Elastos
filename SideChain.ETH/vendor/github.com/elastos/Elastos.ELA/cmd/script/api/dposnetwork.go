package api

import (
	"bytes"

	. "github.com/elastos/Elastos.ELA/cmd/script/api/mock"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"

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

	n := NewNetworkMock()

	ud := L.NewUserData()
	ud.Value = n
	L.SetMetatable(ud, L.GetTypeMetatable(luaNetworkTypeName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *Attribute and returns this *Attribute.
func checkDposNetwork(L *lua.LState, idx int) NetworkMock {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(NetworkMock); ok {
		return v
	}
	L.ArgError(1, "dpos network expected")
	return nil
}

var networkMethods = map[string]lua.LGFunction{
	"push_block":             networkPushBlock,
	"push_confirm":           networkPushConfirm,
	"push_vote":              networkPushVote,
	"push_proposal":          networkPushProposal,
	"push_illegal_proposals": networkPushIllegalProposals,
	"push_illegal_votes":     networkPushIllegalVotes,
	"push_illegal_blocks":    networkPushIllegalBlocks,

	"check_last_msg": networkCheckLastMsg,
	"check_last_pid": networkCheckLastPid,
	"dump_msg":       networkDumpMessage,
}

func networkDumpMessage(L *lua.LState) int {
	n := checkDposNetwork(L, 1)
	m := n.DumpMessages(0)
	L.Push(lua.LString(m))

	return 1
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
	n := checkDposNetwork(L, 1)
	puk := L.ToString(2)
	vote := checkVote(L, 3)

	if vote.Accept {
		n.FireVoteReceived(pidFromString(puk), vote)
	} else {
		n.FireVoteRejected(pidFromString(puk), vote)
	}

	return 0
}

func networkPushProposal(L *lua.LState) int {
	n := checkDposNetwork(L, 1)
	puk := L.ToString(2)
	proposal := checkProposal(L, 3)

	n.FireProposalReceived(pidFromString(puk), proposal)

	return 0
}

func networkPushIllegalProposals(L *lua.LState) int {
	n := checkDposNetwork(L, 1)
	puk := L.ToString(2)
	i := checkIllegalProposals(L, 3)

	n.FireIllegalProposalReceived(pidFromString(puk), i)

	return 0
}

func networkPushIllegalVotes(L *lua.LState) int {
	n := checkDposNetwork(L, 1)
	puk := L.ToString(2)
	i := checkIllegalVotes(L, 3)

	n.FireIllegalVotesReceived(pidFromString(puk), i)

	return 0
}

func networkPushIllegalBlocks(L *lua.LState) int {
	n := checkDposNetwork(L, 1)
	i := checkIllegalBlocks(L, 2)

	n.FireIllegalBlocksReceived(i)

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
		case msg.CmdAcceptVote, msg.CmdRejectVote:
			if voteMsg, ok := n.GetLastMessage().(*msg.Vote); ok {
				vote := checkVote(L, 3)
				result = voteMsg.Vote.Hash().IsEqual(vote.Hash())
			}
		case msg.CmdIllegalProposals:
			if i, ok := n.GetLastMessage().(*msg.IllegalProposals); ok {
				illegalProposals := checkIllegalProposals(L, 3)
				result = i.Proposals.Hash().IsEqual(illegalProposals.Hash())
			}
		case msg.CmdIllegalVotes:
			if i, ok := n.GetLastMessage().(*msg.IllegalVotes); ok {
				illegalVotes := checkIllegalVotes(L, 3)
				result = i.Votes.Hash().IsEqual(illegalVotes.Hash())
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
