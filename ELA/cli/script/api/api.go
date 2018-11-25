package api

import (
	"encoding/hex"

	"github.com/elastos/Elastos.ELA/account"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/yuin/gopher-lua"
)

func Loader(L *lua.LState) int {
	// register functions to the table
	mod := L.SetFuncs(L.NewTable(), exports)
	// register other stuff
	L.SetField(mod, "version", lua.LString("0.1"))

	// returns the module
	L.Push(mod)
	return 1
}

var exports = map[string]lua.LGFunction{
	"hexStrReverse": hexReverse,
	"sendRawTx":     sendTx,
	"getAssetID":    getAssetID,
	"getUTXO":       getUTXO,
}

func hexReverse(L *lua.LState) int {
	str := L.ToString(1)
	buf, _ := hex.DecodeString(str)
	retHex := hex.EncodeToString(common.BytesReverse(buf))

	L.Push(lua.LString(retHex))
	return 1
}

func sendTx(L *lua.LState) int {

	return 1
}

func getAssetID(L *lua.LState) int {
	L.Push(lua.LString(account.SystemAssetID.String()))
	return 1
}

func getUTXO(L *lua.LState) int {
	return 1
}

func RegisterDataType(L *lua.LState) int {
	//RegisterBalanceTxInputType(L)
	RegisterClientType(L)
	//RegisterTxAttributeType(L)
	//RegisterUTXOTxInputType(L)
	//RegisterTxOutputType(L)
	//RegisterCoinBaseType(L)
	//RegisterTransferAssetType(L)
	//RegisterRegisterAssetType(L)
	//RegisterRecordType(L)
	//RegisterDeployCodeType(L)
	//RegisterTransactionType(L)

	return 0
}
