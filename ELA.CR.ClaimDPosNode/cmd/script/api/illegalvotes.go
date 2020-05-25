// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package api

import (
	"bytes"

	"github.com/elastos/Elastos.ELA/core/types"
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
	"hash":        illegalVotesHash,
	"set_content": illegalVotesSetContent,
}

func illegalVotesHash(L *lua.LState) int {
	p := checkIllegalVotes(L, 1)
	h := p.Hash()

	L.Push(lua.LString(h.String()))

	return 1
}

func illegalVotesSetContent(L *lua.LState) int {
	i := checkIllegalVotes(L, 1)
	p := checkProposal(L, 2)
	v := checkVote(L, 3)
	h := checkHeader(L, 4)
	p2 := checkProposal(L, 5)
	v2 := checkVote(L, 6)
	h2 := checkHeader(L, 7)

	asc := true
	if v.Hash().Compare(v2.Hash()) > 0 {
		asc = false
	}

	if asc {
		i.Evidence = *generateVoteEvidence(p, v, h)
		i.CompareEvidence = *generateVoteEvidence(p2, v2, h2)
	} else {
		i.Evidence = *generateVoteEvidence(p2, v2, h2)
		i.CompareEvidence = *generateVoteEvidence(p, v, h)
	}

	return 0
}

func generateVoteEvidence(p *payload.DPOSProposal,
	v *payload.DPOSProposalVote, h *types.Header) *payload.VoteEvidence {

	buf := new(bytes.Buffer)
	h.Serialize(buf)

	return &payload.VoteEvidence{
		Vote: *v,
		ProposalEvidence: payload.ProposalEvidence{
			Proposal:    *p,
			BlockHeight: h.Height,
			BlockHeader: buf.Bytes(),
		},
	}
}
