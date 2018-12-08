package api

import (
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/yuin/gopher-lua"
)

const (
	luaIllegalBlocksTypeName = "illegal_blocks"
)

func RegisterIllegalBlocksType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaIllegalBlocksTypeName)
	L.SetGlobal("illegal_votes", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newIllegalBlocks))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), illegalBlocksMethods))
}

// Constructor
func newIllegalBlocks(L *lua.LState) int {
	coinType := types.CoinType(L.ToInt(1))
	height := uint32(L.ToInt(2))

	illegalBlock := &types.DposIllegalBlocks{
		CoinType:    coinType,
		BlockHeight: height,
	}

	ud := L.NewUserData()
	ud.Value = illegalBlock
	L.SetMetatable(ud, L.GetTypeMetatable(luaIllegalBlocksTypeName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *Attribute and returns this *Attribute.
func checkIllegalBlocks(L *lua.LState, idx int) *types.DposIllegalBlocks {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*types.DposIllegalBlocks); ok {
		return v
	}
	L.ArgError(1, "DPosProposal expected")
	return nil
}

var illegalBlocksMethods = map[string]lua.LGFunction{
	"hash": illegalBlocksHash,
}

func illegalBlocksHash(L *lua.LState) int {
	p := checkIllegalBlocks(L, 1)
	h := p.Hash()

	L.Push(lua.LString(h.String()))

	return 1
}
