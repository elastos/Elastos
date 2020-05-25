// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package api

import (
	"github.com/yuin/gopher-lua"
)

const (
	luaStringsTypeName = "strings"
)

func RegisterStringsType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaStringsTypeName)
	L.SetGlobal("strings", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newStrings))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), stringsMethods))
}

// Constructor
func newStrings(L *lua.LState) int {
	table := L.NewTable()
	L.SetMetatable(table, L.GetTypeMetatable(luaStringsTypeName))
	L.Push(table)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *Attribute and returns this *Attribute.
func checkStrings(L *lua.LState, idx int) *lua.LTable {
	table := L.CheckTable(idx)
	if table != nil {
		return table
	}
	L.ArgError(1, "string array expected")
	return nil
}

var stringsMethods = map[string]lua.LGFunction{
}
