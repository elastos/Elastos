// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package api

import (
	"fmt"

	"github.com/elastos/Elastos.ELA/common"
	pg "github.com/elastos/Elastos.ELA/core/contract/program"

	lua "github.com/yuin/gopher-lua"
)

const (
	luaProgramTypeName = "program"
)

func RegisterProgramType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaProgramTypeName)
	L.SetGlobal("program", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newProgram))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), programMethods))
}

// Constructor
func newProgram(L *lua.LState) int {
	codeHex := L.ToString(1)
	parameterHex := L.ToString(2)

	code, err := common.HexStringToBytes(codeHex)
	if err != nil {
		fmt.Println("invalid code")
	}
	parameter, err := common.HexStringToBytes(parameterHex)
	if err != nil {
		fmt.Println("invalid parameter")
	}

	program := &pg.Program{
		Code:      code,
		Parameter: parameter,
	}

	ud := L.NewUserData()
	ud.Value = program
	L.SetMetatable(ud, L.GetTypeMetatable(luaProgramTypeName))
	L.Push(ud)

	return 1
}

func checkProgram(L *lua.LState, idx int) *pg.Program {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*pg.Program); ok {
		return v
	}
	L.ArgError(1, "Program expected")
	return nil
}

var programMethods = map[string]lua.LGFunction{
	"get": programGet,
}

func programGet(L *lua.LState) int {
	p := checkProgram(L, 1)
	fmt.Println(p)

	return 0
}
