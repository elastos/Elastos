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

	proposal := &payload.Confirm{
		Proposal: payload.DPOSProposal{
			Sponsor:    []byte{},
			BlockHash:  *hash,
			ViewOffset: 0,
			Sign:       []byte{},
		},
		Votes: make([]payload.DPOSProposalVote, 0),
	}
	ud := L.NewUserData()
	ud.Value = proposal
	L.SetMetatable(ud, L.GetTypeMetatable(luaConfirmTypeName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *Attribute and returns this *Attribute.
func checkConfirm(L *lua.LState, idx int) *payload.Confirm {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*payload.Confirm); ok {
		return v
	}
	L.ArgError(1, "Confirm expected")
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
