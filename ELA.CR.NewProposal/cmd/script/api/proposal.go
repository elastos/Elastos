// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package api

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types/payload"

	lua "github.com/yuin/gopher-lua"
)

const luaProposalTypeName = "proposal"

func RegisterProposalType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaProposalTypeName)
	L.SetGlobal("proposal", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newProposal))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), proposalMethods))
}

// Constructor
func newProposal(L *lua.LState) int {
	sponsor := L.ToString(1)
	blockHash := L.ToString(2)
	offset := uint32(L.ToInt(3))

	hash, _ := common.Uint256FromHexString(blockHash)
	pk, _ := common.HexStringToBytes(sponsor)
	proposal := &payload.DPOSProposal{
		Sponsor:    pk,
		BlockHash:  *hash,
		ViewOffset: offset,
		Sign:       nil,
	}
	ud := L.NewUserData()
	ud.Value = proposal
	L.SetMetatable(ud, L.GetTypeMetatable(luaProposalTypeName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *Attribute and returns this *Attribute.
func checkProposal(L *lua.LState, idx int) *payload.DPOSProposal {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*payload.DPOSProposal); ok {
		return v
	}
	L.ArgError(1, "DPOSProposal expected")
	return nil
}

var proposalMethods = map[string]lua.LGFunction{
	"hash":        proposalHash,
	"get_sponsor": proposalGetSponsor,
}

func proposalHash(L *lua.LState) int {
	p := checkProposal(L, 1)
	h := p.Hash()

	L.Push(lua.LString(h.String()))

	return 1
}

func proposalGetSponsor(L *lua.LState) int {
	p := checkProposal(L, 1)

	L.Push(lua.LString(common.BytesToHexString(p.Sponsor)))

	return 1
}
