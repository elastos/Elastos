package api

import (
	"encoding/hex"
	"fmt"

	"github.com/elastos/Elastos.ELA/core"

	"github.com/yuin/gopher-lua"
)

const luaAttributeTypeName = "attribute"

func RegisterAttributeType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaAttributeTypeName)
	L.SetGlobal("attribute", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newAttribute))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), attributeMethods))
}

// Constructor
func newAttribute(L *lua.LState) int {
	usage := L.ToInt(1)
	dataStr := L.ToString(2)
	data, _ := hex.DecodeString(dataStr)

	txAttr := &core.Attribute{
		Usage: core.AttributeUsage(usage),
		Data:  data,
	}
	ud := L.NewUserData()
	ud.Value = txAttr
	L.SetMetatable(ud, L.GetTypeMetatable(luaAttributeTypeName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *Attribute and returns this *Attribute.
func checkAttribute(L *lua.LState, idx int) *core.Attribute {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*core.Attribute); ok {
		return v
	}
	L.ArgError(1, "Attribute expected")
	return nil
}

var attributeMethods = map[string]lua.LGFunction{
	"get": attributeGet,
}

// Getter and setter for the Person#Name
func attributeGet(L *lua.LState) int {
	p := checkAttribute(L, 1)
	fmt.Println(p)

	return 0
}
