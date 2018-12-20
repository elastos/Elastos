package api

import (
	"github.com/elastos/Elastos.ELA/core/types"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/yuin/gopher-lua"
)

const luaConfirmTypeName = "confirm"

func RegisterConfirmType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaConfirmTypeName)
	L.SetGlobal("confirm", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newConfirm))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), confirmMethods))
}

// Constructor
func newConfirm(L *lua.LState) int {
	blockHash := L.ToString(1)
	hash, _ := common.Uint256FromHexString(blockHash)

	proposal := &types.DPosProposalVoteSlot{
		Hash:  *hash,
		Votes: make([]types.DPosProposalVote, 0),
	}
	ud := L.NewUserData()
	ud.Value = proposal
	L.SetMetatable(ud, L.GetTypeMetatable(luaConfirmTypeName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *Attribute and returns this *Attribute.
func checkConfirm(L *lua.LState, idx int) *types.DPosProposalVoteSlot {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*types.DPosProposalVoteSlot); ok {
		return v
	}
	L.ArgError(1, "DPosProposalVoteSlot expected")
	return nil
}

var confirmMethods = map[string]lua.LGFunction{
	"set_proposal": confirmSetProposal,
	"append_vote":  confirmAppendVote,
}

func confirmSetProposal(L *lua.LState) int {
	c := checkConfirm(L, 1)
	p := checkProposal(L, 2)
	c.Proposal = *p

	return 0
}

func confirmAppendVote(L *lua.LState) int {
	c := checkConfirm(L, 1)
	v := checkVote(L, 2)
	c.Votes = append(c.Votes, *v)

	return 0
}
