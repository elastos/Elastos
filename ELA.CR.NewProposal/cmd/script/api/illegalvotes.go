package api

import (
	"bytes"

	"github.com/elastos/Elastos.ELA/core/types/payload"

	"github.com/yuin/gopher-lua"
)

const (
	luaIllegalVotesTypeName = "illegal_votes"
)

func RegisterIllegalVotesType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaIllegalVotesTypeName)
	L.SetGlobal("illegal_votes", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newIllegalVotes))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), illegalVotesMethods))
}

// Constructor
func newIllegalVotes(L *lua.LState) int {

	i := &payload.DPOSIllegalVotes{}

	ud := L.NewUserData()
	ud.Value = i
	L.SetMetatable(ud, L.GetTypeMetatable(luaIllegalVotesTypeName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *Attribute and returns this *Attribute.
func checkIllegalVotes(L *lua.LState, idx int) *payload.DPOSIllegalVotes {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*payload.DPOSIllegalVotes); ok {
		return v
	}
	L.ArgError(1, "DPOSProposal expected")
	return nil
}

var illegalVotesMethods = map[string]lua.LGFunction{
	"hash":         illegalVotesHash,
	"set_proposal": illegalVotesSetProposal,
	"set_header":   illegalVotesSetHeader,
	"set_vote":     illegalVotesSetVote,
}

func illegalVotesHash(L *lua.LState) int {
	p := checkIllegalVotes(L, 1)
	h := p.Hash()

	L.Push(lua.LString(h.String()))

	return 1
}

func illegalVotesSetProposal(L *lua.LState) int {
	i := checkIllegalVotes(L, 1)
	p := checkProposal(L, 2)
	first := L.ToBool(3)

	if first {
		i.Evidence.Proposal = *p
	} else {
		i.CompareEvidence.Proposal = *p
	}

	return 0
}

func illegalVotesSetHeader(L *lua.LState) int {
	i := checkIllegalVotes(L, 1)
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

func illegalVotesSetVote(L *lua.LState) int {
	i := checkIllegalVotes(L, 1)
	v := checkVote(L, 2)
	first := L.ToBool(3)

	if first {
		i.Evidence.Vote = *v
	} else {
		i.CompareEvidence.Vote = *v
	}

	return 0
}
