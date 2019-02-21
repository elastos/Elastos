package api

import (
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/yuin/gopher-lua"
)

const (
	luaIllegalProposalsTypeName = "illegal_proposals"
)

func RegisterIllegalProposalsType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaIllegalProposalsTypeName)
	L.SetGlobal("illegal_proposals", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newIllegalProposals))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), illegalProposalsMethods))
}

// Constructor
func newIllegalProposals(L *lua.LState) int {
	illegalProposals := &types.DposIllegalProposals{}

	ud := L.NewUserData()
	ud.Value = illegalProposals
	L.SetMetatable(ud, L.GetTypeMetatable(luaIllegalProposalsTypeName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *Attribute and returns this *Attribute.
func checkIllegalProposals(L *lua.LState, idx int) *types.DposIllegalProposals {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*types.DposIllegalProposals); ok {
		return v
	}
	L.ArgError(1, "DPosProposal expected")
	return nil
}

var illegalProposalsMethods = map[string]lua.LGFunction{
	"hash":         illegalProposalsHash,
	"set_proposal": illegalProposalsSetProposal,
	"set_header":   illegalProposalsSetHeader,
}

func illegalProposalsHash(L *lua.LState) int {
	p := checkIllegalProposals(L, 1)
	h := p.Hash()

	L.Push(lua.LString(h.String()))

	return 1
}

func illegalProposalsSetProposal(L *lua.LState) int {
	i := checkIllegalProposals(L, 1)
	p := checkProposal(L, 2)
	first := L.ToBool(3)

	if first {
		i.Evidence.Proposal = *p
	} else {
		i.CompareEvidence.Proposal = *p
	}

	return 0
}

func illegalProposalsSetHeader(L *lua.LState) int {
	i := checkIllegalProposals(L, 1)
	h := checkHeader(L, 2)
	first := L.ToBool(3)

	if first {
		i.Evidence.BlockHeader = *h
	} else {
		i.CompareEvidence.BlockHeader = *h
	}

	return 0
}
