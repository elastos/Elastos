package api

import (
	"github.com/elastos/Elastos.ELA/core/types"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/yuin/gopher-lua"
)

const luaVoteTypeName = "vote"

func RegisterVoteType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaVoteTypeName)
	L.SetGlobal("vote", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newVote))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), voteMethods))
}

// Constructor
func newVote(L *lua.LState) int {
	proposalHash := L.ToString(1)
	signer := L.ToString(2)
	accept := L.ToBool(3)
	sign := L.ToString(4)

	hash, _ := common.Uint256FromHexString(proposalHash)
	signData, _ := common.HexStringToBytes(sign)

	proposal := &types.DPosProposalVote{
		ProposalHash: *hash,
		Signer:       signer,
		Accept:       accept,
		Sign:         signData,
	}
	ud := L.NewUserData()
	ud.Value = proposal
	L.SetMetatable(ud, L.GetTypeMetatable(luaVoteTypeName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *Attribute and returns this *Attribute.
func checkVote(L *lua.LState, idx int) *types.DPosProposalVote {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*types.DPosProposalVote); ok {
		return v
	}
	L.ArgError(1, "DPosProposalVote expected")
	return nil
}

var voteMethods = map[string]lua.LGFunction{
	"hash": voteHash,
}

func voteHash(L *lua.LState) int {
	v := checkVote(L, 1)
	h := v.Hash()

	L.Push(lua.LString(h.String()))

	return 1
}
