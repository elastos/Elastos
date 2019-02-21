package api

import (
	mock2 "github.com/elastos/Elastos.ELA/blockchain/mock"
	"github.com/elastos/Elastos.ELA/common"

	"github.com/yuin/gopher-lua"
)

const (
	luaArbitratorsTypeName = "arbitrators"
	MajorityCount          = 3
)

var (
	arbitratorsPublicKeys = []string{
		"023a133480176214f88848c6eaa684a54b316849df2b8570b57f3a917f19bbc77a",
		"030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9",
		"0288e79636e41edce04d4fa95d8f62fed73a76164f8631ccc42f5425f960e4a0c7",
		"03e281f89d85b3a7de177c240c4961cb5b1f2106f09daa42d15874a38bbeae85dd",
		"0393e823c2087ed30871cbea9fa5121fa932550821e9f3b17acef0e581971efab0",
	}
	arbitratorsPrivateKeys = []string{
		"e372ca1032257bb4be1ac99c4861ec542fd55c25c37f5f58ba8b177850b3fdeb",
		"e6deed7e23406e2dce7b01e85bcb33872a47b6200ca983fcf0540dff284923b0",
		"4441968d02a5df4dbc08ca11da2acc86c980e5fe9ff250450a80fd7421d2b0f1",
		"0b14a04e203301809feccc61dbf4e745203a3263d29a4b4091aaa138ba5fb26d",
		"0c11ebca60af2a09ac13dd84fd29c03b99cd086a08a69a9e5b87255fd9cf2eee",
	}
)

func RegisterArbitratorsType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaArbitratorsTypeName)
	L.SetGlobal("arbitrators", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newArbitrators))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), arbitratorsMethods))
}

// Constructor
func newArbitrators(L *lua.LState) int {
	arbitersByte := make([][]byte, 0)
	for _, arbiter := range arbitratorsPublicKeys {
		arbiterByte, _ := common.HexStringToBytes(arbiter)
		arbitersByte = append(arbitersByte, arbiterByte)
	}

	a := mock2.NewArbitratorsMock(arbitersByte, 0, MajorityCount)

	ud := L.NewUserData()
	ud.Value = a
	L.SetMetatable(ud, L.GetTypeMetatable(luaArbitratorsTypeName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *Attribute and returns this *Attribute.
func checkArbitrators(L *lua.LState, idx int) *mock2.ArbitratorsMock {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*mock2.ArbitratorsMock); ok {
		return v
	}
	L.ArgError(1, "arbitrators expected")
	return nil
}

var arbitratorsMethods = map[string]lua.LGFunction{
	"get_duty_index": arbitratorsGetDutyIndex,
	"set_duty_index": arbitratorsSetDutyIndex,
}

func arbitratorsGetDutyIndex(L *lua.LState) int {
	a := checkArbitrators(L, 1)
	L.Push(lua.LNumber(a.GetDutyChangeCount()))

	return 1
}

func arbitratorsSetDutyIndex(L *lua.LState) int {
	a := checkArbitrators(L, 1)
	index := L.ToInt(2)
	a.SetDutyChangeCount(uint32(index))

	return 0
}
