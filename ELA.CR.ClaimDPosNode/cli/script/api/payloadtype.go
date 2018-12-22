package api

import (
	"encoding/hex"
	"fmt"
	"os"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types/payload"

	"github.com/yuin/gopher-lua"
)

const (
	luaCoinBaseTypeName      = "coinbase"
	luaTransferAssetTypeName = "transferasset"
	luaRegisterProducerName  = "registerproducer"
	luaUpdateProducerName    = "updateproducer"
	luaReturnDepositCoinName = "returndepositcoin"
)

func RegisterCoinBaseType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaCoinBaseTypeName)
	L.SetGlobal("coinbase", mt)
	L.SetField(mt, "new", L.NewFunction(newCoinBase))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), coinbaseMethods))
}

// Constructor
func newCoinBase(L *lua.LState) int {
	data, _ := hex.DecodeString(L.ToString(1))
	cb := &payload.PayloadCoinBase{
		CoinbaseData: data,
	}
	ud := L.NewUserData()
	ud.Value = cb
	L.SetMetatable(ud, L.GetTypeMetatable(luaCoinBaseTypeName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *PayloadCoinBase and
// returns this *PayloadCoinBase.
func checkCoinBase(L *lua.LState, idx int) *payload.PayloadCoinBase {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*payload.PayloadCoinBase); ok {
		return v
	}
	L.ArgError(1, "PayloadCoinBase expected")
	return nil
}

var coinbaseMethods = map[string]lua.LGFunction{
	"get": coinbaseGet,
}

// Getter and setter for the Person#Name
func coinbaseGet(L *lua.LState) int {
	p := checkCoinBase(L, 1)
	fmt.Println(p)

	return 0
}

// Registers my person type to given L.
func RegisterTransferAssetType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaTransferAssetTypeName)
	L.SetGlobal("transferasset", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newTransferAsset))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), transferassetMethods))
}

// Constructor
func newTransferAsset(L *lua.LState) int {
	ta := &payload.PayloadTransferAsset{}
	ud := L.NewUserData()
	ud.Value = ta
	L.SetMetatable(ud, L.GetTypeMetatable(luaTransferAssetTypeName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *PayloadTransferAsset and
// returns this *PayloadTransferAsset.
func checkTransferAsset(L *lua.LState, idx int) *payload.PayloadTransferAsset {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*payload.PayloadTransferAsset); ok {
		return v
	}
	L.ArgError(1, "PayloadTransferAsset expected")
	return nil
}

var transferassetMethods = map[string]lua.LGFunction{
	"get": transferassetGet,
}

// Getter and setter for the Person#Name
func transferassetGet(L *lua.LState) int {
	p := checkTransferAsset(L, 1)
	fmt.Println(p)

	return 0
}

func RegisterUpdateProducerType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaUpdateProducerName)
	L.SetGlobal("updateproducer", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newUpdateProducer))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), updateProducerMethods))
}

// Constructor
func newUpdateProducer(L *lua.LState) int {
	publicKeyStr := L.ToString(1)
	nickName := L.ToString(2)
	url := L.ToString(3)
	location := L.ToInt64(4)
	address := L.ToString(5)

	publicKey, err := common.HexStringToBytes(publicKeyStr)
	if err != nil {
		fmt.Println("wrong producer public key")
		os.Exit(1)
	}
	updateProducer := &payload.PayloadUpdateProducer{
		PublicKey: []byte(publicKey),
		NickName:  nickName,
		Url:       url,
		Location:  uint64(location),
		Address:   address,
	}
	ud := L.NewUserData()
	ud.Value = updateProducer
	L.SetMetatable(ud, L.GetTypeMetatable(luaUpdateProducerName))
	L.Push(ud)

	return 1
}

func checkUpdateProducer(L *lua.LState, idx int) *payload.PayloadUpdateProducer {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*payload.PayloadUpdateProducer); ok {
		return v
	}
	L.ArgError(1, "PayloadUpdateProducer expected")
	return nil
}

var updateProducerMethods = map[string]lua.LGFunction{
	"get": updateProducerGet,
}

// Getter and setter for the Person#Name
func updateProducerGet(L *lua.LState) int {
	p := checkUpdateProducer(L, 1)
	fmt.Println(p)

	return 0
}

// Registers my person type to given L.
func RegisterRegisterProducerType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaRegisterProducerName)
	L.SetGlobal("registerproducer", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newRegisterProducer))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), registerProducerMethods))
}

// Constructor
func newRegisterProducer(L *lua.LState) int {
	publicKeyStr := L.ToString(1)
	nickName := L.ToString(2)
	url := L.ToString(3)
	location := L.ToInt64(4)
	address := L.ToString(5)

	publicKey, err := common.HexStringToBytes(publicKeyStr)
	if err != nil {
		fmt.Println("wrong producer public key")
		os.Exit(1)
	}
	registerProducer := &payload.PayloadRegisterProducer{
		PublicKey: []byte(publicKey),
		NickName:  nickName,
		Url:       url,
		Location:  uint64(location),
		Address:   address,
	}
	ud := L.NewUserData()
	ud.Value = registerProducer
	L.SetMetatable(ud, L.GetTypeMetatable(luaRegisterProducerName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *PayloadRegisterProducer and
// returns this *PayloadRegisterProducer.
func checkRegisterProducer(L *lua.LState, idx int) *payload.PayloadRegisterProducer {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*payload.PayloadRegisterProducer); ok {
		return v
	}
	L.ArgError(1, "PayloadRegisterProducer expected")
	return nil
}

var registerProducerMethods = map[string]lua.LGFunction{
	"get": registerProducerGet,
}

// Getter and setter for the Person#Name
func registerProducerGet(L *lua.LState) int {
	p := checkRegisterProducer(L, 1)
	fmt.Println(p)

	return 0
}

func RegisterReturnDepositCoinType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaReturnDepositCoinName)
	L.SetGlobal("returndepositcoin", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newReturnDepositCoin))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), returnDepositCoinMethods))
}

// Constructor
func newReturnDepositCoin(L *lua.LState) int {
	registerProducer := &payload.PayloadReturnDepositCoin{}
	ud := L.NewUserData()
	ud.Value = registerProducer
	L.SetMetatable(ud, L.GetTypeMetatable(luaReturnDepositCoinName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *PayloadReturnDepositCoin and
// returns this *PayloadReturnDepositCoin.
func checkReturnDepositCoin(L *lua.LState, idx int) *payload.PayloadReturnDepositCoin {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*payload.PayloadReturnDepositCoin); ok {
		return v
	}
	L.ArgError(1, "PayloadReturnDepositCoin expected")
	return nil
}

var returnDepositCoinMethods = map[string]lua.LGFunction{
	"get": returnDepositCoinGet,
}

// Getter and setter for the Person#Name
func returnDepositCoinGet(L *lua.LState) int {
	p := checkReturnDepositCoin(L, 1)
	fmt.Println(p)

	return 0
}
