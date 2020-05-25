// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package api

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types/payload"

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

	hash, _ := common.Uint256FromHexString(proposalHash)
	pk, _ := common.HexStringToBytes(signer)
	proposal := &payload.DPOSProposalVote{
		ProposalHash: *hash,
		Signer:       pk,
		Accept:       accept,
		Sign:         nil,
	}
	ud := L.NewUserData()
	ud.Value = proposal
	L.SetMetatable(ud, L.GetTypeMetatable(luaVoteTypeName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *Attribute and returns this *Attribute.
func checkVote(L *lua.LState, idx int) *payload.DPOSProposalVote {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*payload.DPOSProposalVote); ok {
		return v
	}
	L.ArgError(1, "DPOSProposalVote expected")
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
