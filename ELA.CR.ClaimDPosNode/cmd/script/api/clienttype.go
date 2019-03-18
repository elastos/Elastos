package api

import (
	"encoding/hex"
	"fmt"

	"github.com/elastos/Elastos.ELA/account"

	"github.com/yuin/gopher-lua"
)

const luaClientTypeName = "client"

func RegisterClientType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaClientTypeName)
	L.SetGlobal("client", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newClient))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), clientMethods))
}

// Constructor
func newClient(L *lua.LState) int {
	name := L.ToString(1)
	pwd := L.ToString(2)
	create := L.ToBool(3)
	var wallet *account.Client
	if create {
		wallet, _ = account.Create(name, []byte(pwd))
	} else {
		wallet, _ = account.Open(name, []byte(pwd))
	}

	ud := L.NewUserData()
	ud.Value = wallet
	L.SetMetatable(ud, L.GetTypeMetatable(luaClientTypeName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *Wallet and returns this *Wallet.
func checkClient(L *lua.LState, idx int) *account.Client {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*account.Client); ok {
		return v
	}
	L.ArgError(1, "Wallet expected")

	return nil
}

var clientMethods = map[string]lua.LGFunction{
	"get":           clientGet,
	"get_address":   getWalletAddr,
	"get_publickey": getWalletPubkey,
}

// Getter and setter for the Person#Name
func clientGet(L *lua.LState) int {
	p := checkClient(L, 1)
	fmt.Println(p)

	return 0
}

func getWalletAddr(L *lua.LState) int {
	wallet := checkClient(L, 1)
	acc := wallet.GetMainAccount()
	addr, _ := acc.ProgramHash.ToAddress()

	L.Push(lua.LString(addr))

	return 1
}

func getWalletPubkey(L *lua.LState) int {
	wallet := checkClient(L, 1)
	acc := wallet.GetMainAccount()
	pubkey, _ := acc.PublicKey.EncodePoint(true)
	L.Push(lua.LString(hex.EncodeToString(pubkey)))

	return 1
}
