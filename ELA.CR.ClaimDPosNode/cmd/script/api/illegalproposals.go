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
	"hash":        illegalProposalsHash,
	"set_content": illegalProposalsSetContent,
}

func illegalProposalsHash(L *lua.LState) int {
	p := checkIllegalProposals(L, 1)
	h := p.Hash()

	L.Push(lua.LString(h.String()))

	return 1
}

func illegalProposalsSetContent(L *lua.LState) int {
	i := checkIllegalProposals(L, 1)
	p := checkProposal(L, 2)
	h := checkHeader(L, 3)
	p2 := checkProposal(L, 4)
	h2 := checkHeader(L, 5)

	buf := new(bytes.Buffer)
	h.Serialize(buf)
	e1 := payload.ProposalEvidence{
		Proposal:    *p,
		BlockHeader: buf.Bytes(),
		BlockHeight: h.Height,
	}

	buf2 := new(bytes.Buffer)
	h2.Serialize(buf2)
	e2 := payload.ProposalEvidence{
		Proposal:    *p2,
		BlockHeader: buf2.Bytes(),
		BlockHeight: h2.Height,
	}

	asc := true
	if p.Hash().Compare(p2.Hash()) > 0 {
		asc = false
	}

	if asc {
		i.Evidence = e1
		i.CompareEvidence = e2
	} else {
		i.Evidence = e2
		i.CompareEvidence = e1
	}

	return 0
}
