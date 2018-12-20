package api

import (
	"encoding/hex"
	"fmt"

	"github.com/elastos/Elastos.ELA/core/types"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/yuin/gopher-lua"
)

const luaInputTypeName = "input"

func RegisterInputType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaInputTypeName)
	L.SetGlobal("input", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newInput))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), inputMethods))
}

// Constructor
func newInput(L *lua.LState) int {
	referIDStr := L.ToString(1)
	referIdx := L.ToInt(2)
	sequence := L.ToInt(3)
	referIDSlice, _ := hex.DecodeString(referIDStr)
	referIDSlice = common.BytesReverse(referIDSlice)
	var referID common.Uint256
	copy(referID[:], referIDSlice[0:32])
	input := &types.Input{
		Previous: types.OutPoint{
			TxID:  referID,
			Index: uint16(referIdx),
		},
		Sequence: uint32(sequence),
	}
	ud := L.NewUserData()
	ud.Value = input
	L.SetMetatable(ud, L.GetTypeMetatable(luaInputTypeName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *Input and returns this *Input.
func checkInput(L *lua.LState, idx int) *types.Input {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*types.Input); ok {
		return v
	}
	L.ArgError(1, "Input expected")
	return nil
}

var inputMethods = map[string]lua.LGFunction{
	"get": inputGet,
}

// Getter and setter for the Person#Name
func inputGet(L *lua.LState) int {
	p := checkInput(L, 1)
	fmt.Println(p)

	return 0
}
