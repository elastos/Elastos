package api

import (
	"bytes"

	"github.com/elastos/Elastos.ELA/core/types/payload"

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
	illegalProposals := &payload.DPOSIllegalProposals{}

	ud := L.NewUserData()
	ud.Value = illegalProposals
	L.SetMetatable(ud, L.GetTypeMetatable(luaIllegalProposalsTypeName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *Attribute and returns this *Attribute.
func checkIllegalProposals(L *lua.LState, idx int) *payload.DPOSIllegalProposals {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*payload.DPOSIllegalProposals); ok {
		return v
	}
	L.ArgError(1, "DPOSProposal expected")
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

	buf := new(bytes.Buffer)
	h.Serialize(buf)

	if first {
		i.Evidence.BlockHeader = buf.Bytes()
		i.Evidence.BlockHeight = h.Height
	} else {
		i.CompareEvidence.BlockHeader = buf.Bytes()
		i.CompareEvidence.BlockHeight = h.Height
	}

	return 0
}
