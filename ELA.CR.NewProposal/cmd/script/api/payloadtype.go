// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package api

import (
	"bytes"
	"encoding/hex"
	"fmt"
	"os"
	"strings"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"
	lua "github.com/yuin/gopher-lua"
)

const (
	luaCoinBaseTypeName          = "coinbase"
	luaTransferAssetTypeName     = "transferasset"
	luaRegisterProducerName      = "registerproducer"
	luaUpdateProducerName        = "updateproducer"
	luaCancelProducerName        = "cancelproducer"
	luaActivateProducerName      = "activateproducer"
	luaReturnDepositCoinName     = "returndepositcoin"
	luaSideChainPowName          = "sidechainpow"
	luaRegisterCRName            = "registercr"
	luaUpdateCRName              = "updatecr"
	luaUnregisterCRName          = "unregistercr"
	luaCRCProposalName           = "crcproposal"
	luaCRChangeProposalOwnerName = "crchangeproposalowner"
	luaCRCCloseProposalHashName  = "crccloseproposalhash"
	luaCRCProposalReviewName     = "crcproposalreview"
	luaCRCProposalTrackingName   = "crcproposaltracking"
	luaCRCProposalWithdrawName   = "crcproposalwithdraw"
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
	cb := &payload.CoinBase{
		Content: data,
	}
	ud := L.NewUserData()
	ud.Value = cb
	L.SetMetatable(ud, L.GetTypeMetatable(luaCoinBaseTypeName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *CoinBase and
// returns this *CoinBase.
func checkCoinBase(L *lua.LState, idx int) *payload.CoinBase {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*payload.CoinBase); ok {
		return v
	}
	L.ArgError(1, "CoinBase expected")
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
	ta := &payload.TransferAsset{}
	ud := L.NewUserData()
	ud.Value = ta
	L.SetMetatable(ud, L.GetTypeMetatable(luaTransferAssetTypeName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *TransferAsset and
// returns this *TransferAsset.
func checkTransferAsset(L *lua.LState, idx int) *payload.TransferAsset {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*payload.TransferAsset); ok {
		return v
	}
	L.ArgError(1, "TransferAsset expected")
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
	ownerPublicKeyStr := L.ToString(1)
	nodePublicKeyStr := L.ToString(2)
	nickName := L.ToString(3)
	url := L.ToString(4)
	location := L.ToInt64(5)
	address := L.ToString(6)
	needSign := true
	client, err := checkClient(L, 7)
	if err != nil {
		needSign = false
	}

	ownerPublicKey, err := common.HexStringToBytes(ownerPublicKeyStr)
	if err != nil {
		fmt.Println("wrong producer public key")
		os.Exit(1)
	}
	nodePublicKey, err := common.HexStringToBytes(nodePublicKeyStr)
	if err != nil {
		fmt.Println("wrong producer public key")
		os.Exit(1)
	}
	updateProducer := &payload.ProducerInfo{
		OwnerPublicKey: []byte(ownerPublicKey),
		NodePublicKey:  []byte(nodePublicKey),
		NickName:       nickName,
		Url:            url,
		Location:       uint64(location),
		NetAddress:     address,
	}

	if needSign {
		upSignBuf := new(bytes.Buffer)
		err = updateProducer.SerializeUnsigned(upSignBuf, payload.ProducerInfoVersion)
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}

		codeHash, err := contract.PublicKeyToStandardCodeHash(ownerPublicKey)
		acc := client.GetAccountByCodeHash(*codeHash)
		if acc == nil {
			fmt.Println("no available account in wallet")
			os.Exit(1)
		}
		rpSig, err := crypto.Sign(acc.PrivKey(), upSignBuf.Bytes())
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		updateProducer.Signature = rpSig
	}

	ud := L.NewUserData()
	ud.Value = updateProducer
	L.SetMetatable(ud, L.GetTypeMetatable(luaUpdateProducerName))
	L.Push(ud)

	return 1
}

func checkUpdateProducer(L *lua.LState, idx int) *payload.ProducerInfo {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*payload.ProducerInfo); ok {
		return v
	}
	L.ArgError(1, "ProducerInfo expected")
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
	ownerPublicKeyStr := L.ToString(1)
	nodePublicKeyStr := L.ToString(2)
	nickName := L.ToString(3)
	url := L.ToString(4)
	location := L.ToInt64(5)
	address := L.ToString(6)
	needSign := true
	client, err := checkClient(L, 7)
	if err != nil {
		needSign = false
	}

	ownerPublicKey, err := common.HexStringToBytes(ownerPublicKeyStr)
	if err != nil {
		fmt.Println("wrong producer public key")
		os.Exit(1)
	}
	nodePublicKey, err := common.HexStringToBytes(nodePublicKeyStr)
	if err != nil {
		fmt.Println("wrong producer public key")
		os.Exit(1)
	}

	registerProducer := &payload.ProducerInfo{
		OwnerPublicKey: []byte(ownerPublicKey),
		NodePublicKey:  []byte(nodePublicKey),
		NickName:       nickName,
		Url:            url,
		Location:       uint64(location),
		NetAddress:     address,
	}

	if needSign {
		rpSignBuf := new(bytes.Buffer)
		err = registerProducer.SerializeUnsigned(rpSignBuf, payload.ProducerInfoVersion)
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		codeHash, err := contract.PublicKeyToStandardCodeHash(ownerPublicKey)
		acc := client.GetAccountByCodeHash(*codeHash)
		if acc == nil {
			fmt.Println("no available account in wallet")
			os.Exit(1)
		}
		rpSig, err := crypto.Sign(acc.PrivKey(), rpSignBuf.Bytes())
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		registerProducer.Signature = rpSig
	}

	ud := L.NewUserData()
	ud.Value = registerProducer
	L.SetMetatable(ud, L.GetTypeMetatable(luaRegisterProducerName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *ProducerInfo and
// returns this *ProducerInfo.
func checkRegisterProducer(L *lua.LState, idx int) *payload.ProducerInfo {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*payload.ProducerInfo); ok {
		return v
	}
	L.ArgError(1, "ProducerInfo expected")
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

func RegisterCancelProducerType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaCancelProducerName)
	L.SetGlobal("cancelproducer", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newProcessProducer))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), cancelProducerMethods))
}

// Constructor
func newProcessProducer(L *lua.LState) int {
	publicKeyStr := L.ToString(1)
	client, err := checkClient(L, 2)
	if err != nil {
		fmt.Println(err)
	}

	publicKey, err := common.HexStringToBytes(publicKeyStr)
	if err != nil {
		fmt.Println("wrong producer public key")
		os.Exit(1)
	}
	processProducer := &payload.ProcessProducer{
		OwnerPublicKey: []byte(publicKey),
	}

	cpSignBuf := new(bytes.Buffer)
	err = processProducer.SerializeUnsigned(cpSignBuf, payload.ProcessProducerVersion)
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
	codeHash, err := contract.PublicKeyToStandardCodeHash(publicKey)
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
	acc := client.GetAccountByCodeHash(*codeHash)
	if acc == nil {
		fmt.Println("no available account in wallet")
		os.Exit(1)
	}
	rpSig, err := crypto.Sign(acc.PrivKey(), cpSignBuf.Bytes())
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
	processProducer.Signature = rpSig

	ud := L.NewUserData()
	ud.Value = processProducer
	L.SetMetatable(ud, L.GetTypeMetatable(luaCancelProducerName))
	L.Push(ud)

	return 1
}

func checkCancelProducer(L *lua.LState, idx int) *payload.ProcessProducer {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*payload.ProcessProducer); ok {
		return v
	}
	L.ArgError(1, "CancelProducer expected")
	return nil
}

var cancelProducerMethods = map[string]lua.LGFunction{
	"get": cancelProducerGet,
}

// Getter and setter for the Person#Name
func cancelProducerGet(L *lua.LState) int {
	p := checkCancelProducer(L, 1)
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
	returnDeposit := &payload.ReturnDepositCoin{}
	ud := L.NewUserData()
	ud.Value = returnDeposit
	L.SetMetatable(ud, L.GetTypeMetatable(luaReturnDepositCoinName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *ReturnDepositCoin and
// returns this *ReturnDepositCoin.
func checkReturnDepositCoin(L *lua.LState, idx int) *payload.ReturnDepositCoin {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*payload.ReturnDepositCoin); ok {
		return v
	}
	L.ArgError(1, "ReturnDepositCoin expected")
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

func RegisterActivateProducerType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaActivateProducerName)
	L.SetGlobal("activateproducer", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newActivateProducer))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), activateProducerMethods))
}

func newActivateProducer(L *lua.LState) int {
	publicKeyStr := L.ToString(1)
	client, err := checkClient(L, 2)
	if err != nil {
		fmt.Println(err)
	}

	publicKey, err := common.HexStringToBytes(publicKeyStr)
	if err != nil {
		fmt.Println("wrong producer node public key")
		os.Exit(1)
	}
	activateProducer := &payload.ActivateProducer{
		NodePublicKey: []byte(publicKey),
	}

	apSignBuf := new(bytes.Buffer)
	err = activateProducer.SerializeUnsigned(apSignBuf, payload.ActivateProducerVersion)
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
	codeHash, err := contract.PublicKeyToStandardCodeHash(publicKey)
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
	acc := client.GetAccountByCodeHash(*codeHash)
	if err != nil {
		fmt.Println(err)
	}
	if acc == nil {
		fmt.Println("no available account in wallet")
		os.Exit(1)
	}
	rpSig, err := crypto.Sign(acc.PrivKey(), apSignBuf.Bytes())
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
	activateProducer.Signature = rpSig

	ud := L.NewUserData()
	ud.Value = activateProducer
	L.SetMetatable(ud, L.GetTypeMetatable(luaActivateProducerName))
	L.Push(ud)

	return 1
}

func checkActivateProducer(L *lua.LState, idx int) *payload.ActivateProducer {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*payload.ActivateProducer); ok {
		return v
	}
	L.ArgError(1, "ActivateProducer expected")
	return nil
}

var activateProducerMethods = map[string]lua.LGFunction{
	"get": activateProducerGet,
}

// Getter and setter for the Person#Name
func activateProducerGet(L *lua.LState) int {
	p := checkActivateProducer(L, 1)
	fmt.Println(p)

	return 0
}

func RegisterSidechainPowType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaSideChainPowName)
	L.SetGlobal("sidechainpow", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newSideChainPow))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), returnSideChainPowMethods))
}

// Constructor
func newSideChainPow(L *lua.LState) int {
	sideBlockHashStr := L.ToString(1)
	sideGenesisHashStr := L.ToString(2)
	blockHeight := L.ToInt(3)
	client, err := checkClient(L, 4)
	if err != nil {
		fmt.Println(err)
	}

	sideBlockHash, err := common.Uint256FromHexString(sideBlockHashStr)
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
	sideGenesisHash, err := common.Uint256FromHexString(sideGenesisHashStr)
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}

	sideChainPow := &payload.SideChainPow{
		SideBlockHash:   *sideBlockHash,
		SideGenesisHash: *sideGenesisHash,
		BlockHeight:     uint32(blockHeight),
	}

	spSignBuf := new(bytes.Buffer)
	err = sideChainPow.SerializeUnsigned(spSignBuf, payload.SideChainPowVersion)
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
	acc := client.GetMainAccount()
	spSig, err := crypto.Sign(acc.PrivKey(), spSignBuf.Bytes())
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
	sideChainPow.Signature = spSig

	ud := L.NewUserData()
	ud.Value = sideChainPow
	L.SetMetatable(ud, L.GetTypeMetatable(luaSideChainPowName))
	L.Push(ud)

	return 1
}

func checkSideChainPow(L *lua.LState, idx int) *payload.SideChainPow {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*payload.SideChainPow); ok {
		return v
	}
	L.ArgError(1, "SideChainPow expected")
	return nil
}

var returnSideChainPowMethods = map[string]lua.LGFunction{
	"get": returnSideChainPowGet,
}

// Getter and setter for the Person#Name
func returnSideChainPowGet(L *lua.LState) int {
	p := checkSideChainPow(L, 1)
	fmt.Println(p)

	return 0
}

// Registers my person type to given L.
func RegisterRegisterCRType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaRegisterCRName)
	L.SetGlobal("registercr", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newRegisterCR))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), registerCRMethods))
}

// Constructor
func newRegisterCR(L *lua.LState) int {
	publicKeyStr := L.ToString(1)
	nickName := L.ToString(2)
	url := L.ToString(3)
	location := L.ToInt64(4)
	payloadVersion := byte(L.ToInt(5))
	needSign := true
	client, err := checkClient(L, 6)
	if err != nil {
		needSign = false
	}
	publicKey, err := common.HexStringToBytes(publicKeyStr)
	if err != nil {
		fmt.Println("wrong cr public key")
		os.Exit(1)
	}

	pk, err := crypto.DecodePoint(publicKey)
	if err != nil {
		fmt.Println("wrong cr public key")
		os.Exit(1)
	}

	code, err := contract.CreateStandardRedeemScript(pk)
	if err != nil {
		fmt.Println("wrong cr public key")
		os.Exit(1)
	}

	ct, err := contract.CreateCRIDContractByCode(code)
	if err != nil {
		fmt.Println("wrong cr public key")
		os.Exit(1)
	}

	didCode := make([]byte, len(code))
	copy(didCode, code)
	didCode = append(didCode[:len(code)-1], common.DID)
	didCT, err := contract.CreateCRIDContractByCode(didCode)
	if err != nil {
		fmt.Println("wrong cr public key")
		os.Exit(1)
	}

	registerCR := &payload.CRInfo{
		Code:     code,
		CID:      *ct.ToProgramHash(),
		DID:      *didCT.ToProgramHash(),
		NickName: nickName,
		Url:      url,
		Location: uint64(location),
	}

	if needSign {
		rpSignBuf := new(bytes.Buffer)
		err = registerCR.SerializeUnsigned(rpSignBuf, payloadVersion)
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		codeHash, err := contract.PublicKeyToStandardCodeHash(publicKey)
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		acc := client.GetAccountByCodeHash(*codeHash)
		if acc == nil {
			fmt.Println("no available account in wallet")
			os.Exit(1)
		}
		rpSig, err := crypto.Sign(acc.PrivKey(), rpSignBuf.Bytes())
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		registerCR.Signature = rpSig
	}

	ud := L.NewUserData()
	ud.Value = registerCR
	L.SetMetatable(ud, L.GetTypeMetatable(luaRegisterCRName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *CRInfo and
// returns this *CRInfo.
func checkRegisterCR(L *lua.LState, idx int) *payload.CRInfo {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*payload.CRInfo); ok {
		return v
	}
	L.ArgError(1, "ProducerInfo expected")
	return nil
}

var registerCRMethods = map[string]lua.LGFunction{
	"get": registerCRGet,
}

// Getter and setter for the Person#Name
func registerCRGet(L *lua.LState) int {
	p := checkRegisterCR(L, 1)
	fmt.Println(p)

	return 0
}

//
// Registers my person type to given L.
func RegisterUpdateCRType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaUpdateCRName)
	L.SetGlobal("updatecr", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newUpdateCR))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), updateCRMethods))
}

// Constructor
func newUpdateCR(L *lua.LState) int {
	publicKeyStr := L.ToString(1)
	nickName := L.ToString(2)
	url := L.ToString(3)
	location := L.ToInt64(4)
	payloadVersion := byte(L.ToInt(5))
	needSign := true
	client, err := checkClient(L, 6)
	if err != nil {
		needSign = false
	}
	publicKey, err := common.HexStringToBytes(publicKeyStr)
	if err != nil {
		fmt.Println("wrong cr public key")
		os.Exit(1)
	}

	pk, err := crypto.DecodePoint(publicKey)
	if err != nil {
		fmt.Println("wrong cr public key")
		os.Exit(1)
	}

	code, err := contract.CreateStandardRedeemScript(pk)
	if err != nil {
		fmt.Println("wrong cr public key")
		os.Exit(1)
	}

	ct, err := contract.CreateCRIDContractByCode(code)
	if err != nil {
		fmt.Println("wrong cr public key")
		os.Exit(1)
	}

	didCode := make([]byte, len(code))
	copy(didCode, code)
	didCode = append(didCode[:len(code)-1], common.DID)
	didCT, err := contract.CreateCRIDContractByCode(didCode)
	if err != nil {
		fmt.Println("wrong cr public key")
		os.Exit(1)
	}

	updateCR := &payload.CRInfo{
		Code:     ct.Code,
		CID:      *ct.ToProgramHash(),
		DID:      *didCT.ToProgramHash(),
		NickName: nickName,
		Url:      url,
		Location: uint64(location),
	}

	if needSign {
		rpSignBuf := new(bytes.Buffer)
		err = updateCR.SerializeUnsigned(rpSignBuf, payloadVersion)
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		codeHash, err := contract.PublicKeyToStandardCodeHash(publicKey)
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		acc := client.GetAccountByCodeHash(*codeHash)
		if acc == nil {
			fmt.Println("no available account in wallet")
			os.Exit(1)
		}
		rpSig, err := crypto.Sign(acc.PrivKey(), rpSignBuf.Bytes())
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		updateCR.Signature = rpSig
	}

	ud := L.NewUserData()
	ud.Value = updateCR
	L.SetMetatable(ud, L.GetTypeMetatable(luaUpdateCRName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *CRInfo and
// returns this *CRInfo.
func checkUpdateCR(L *lua.LState, idx int) *payload.CRInfo {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*payload.CRInfo); ok {
		return v
	}
	L.ArgError(1, "CRInfo expected")
	return nil
}

var updateCRMethods = map[string]lua.LGFunction{
	"get": updateCRGet,
}

// Getter and setter for the Person#Name
func updateCRGet(L *lua.LState) int {
	p := checkUpdateCR(L, 1)
	fmt.Println(p)

	return 0
}

func RegisterUnregisterCRType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaUnregisterCRName)
	L.SetGlobal("unregistercr", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newUnregisterCR))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), unregisterCRMethods))
}
func getIDProgramHash(code []byte) *common.Uint168 {
	ct, _ := contract.CreateCRIDContractByCode(code)
	return ct.ToProgramHash()
}

// Constructor
func newUnregisterCR(L *lua.LState) int {
	publicKeyStr := L.ToString(1)
	needSign := true
	client, err := checkClient(L, 2)
	if err != nil {
		needSign = false
	}
	publicKey, err := common.HexStringToBytes(publicKeyStr)
	if err != nil {
		fmt.Println("wrong cr public key")
		os.Exit(1)
	}

	pk, err := crypto.DecodePoint(publicKey)
	if err != nil {
		fmt.Println("wrong cr public key")
		os.Exit(1)
	}

	ct, err := contract.CreateStandardContract(pk)
	if err != nil {
		fmt.Println("wrong cr public key")
		os.Exit(1)
	}
	cid := getIDProgramHash(ct.Code)
	unregisterCR := &payload.UnregisterCR{
		CID: *cid,
	}

	if needSign {
		rpSignBuf := new(bytes.Buffer)
		err = unregisterCR.SerializeUnsigned(rpSignBuf, payload.UnregisterCRVersion)
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		codeHash, err := contract.PublicKeyToStandardCodeHash(publicKey)
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		acc := client.GetAccountByCodeHash(*codeHash)
		if acc == nil {
			fmt.Println("no available account in wallet")
			os.Exit(1)
		}
		rpSig, err := crypto.Sign(acc.PrivKey(), rpSignBuf.Bytes())
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		unregisterCR.Signature = rpSig
	}

	ud := L.NewUserData()
	ud.Value = unregisterCR
	L.SetMetatable(ud, L.GetTypeMetatable(luaUnregisterCRName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *CRInfo and
// returns this *CRInfo.
func checkUnregisterCR(L *lua.LState, idx int) *payload.UnregisterCR {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*payload.UnregisterCR); ok {
		return v
	}
	L.ArgError(1, "UnregisterCR expected")
	return nil
}

var unregisterCRMethods = map[string]lua.LGFunction{
	"get": unregisterCRGet,
}

// Getter and setter for the Person#Name
func unregisterCRGet(L *lua.LState) int {
	p := checkUnregisterCR(L, 1)
	fmt.Println(p)

	return 0
}

func RegisterCRCProposalType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaCRCProposalName)
	L.SetGlobal("crcproposal", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newCRCProposal))
	L.SetField(mt, "newsg", L.NewFunction(newSecretaryGeneralProposal))

	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), crcProposalMethods))
}

func RegisterCRChangeProposalOwnerType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaCRChangeProposalOwnerName)
	L.SetGlobal("crchangeproposalowner", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newCRChangeProposalOwner))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), crcProposalMethods))
}

func RegisterCRCCloseProposalHashType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaCRCCloseProposalHashName)
	L.SetGlobal("crccloseproposalhash", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newCRCCloseProposalHash))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), crcProposalMethods))
}

// Constructor
func newSecretaryGeneralProposal(L *lua.LState) int {
	fmt.Println("newSecretaryGeneralProposal begin")

	ownerPublicKeyStr := L.ToString(1)
	ownerPrivateKeyStr := L.ToString(2)

	proposalType := L.ToInt64(3)
	draftHashStr := L.ToString(4)

	secretaryGeneralPublicKeyStr := L.ToString(5)
	secretaryGeneralPrivateKeyStr := L.ToString(6)
	client, err := checkClient(L, 7)

	fmt.Println("ownerPublicKeyStr", ownerPublicKeyStr)
	fmt.Println("ownerPrivateKeyStr", ownerPrivateKeyStr)
	fmt.Println("proposalType", proposalType)
	fmt.Println("draftHashStr", draftHashStr)
	fmt.Println("secretaryGeneralPublicKeyStr", secretaryGeneralPublicKeyStr)
	fmt.Println("secretaryGeneralPrivateKeyStr", secretaryGeneralPrivateKeyStr)

	secretaryGeneralPublicKey, _ := common.HexStringToBytes(secretaryGeneralPublicKeyStr)
	ownPublicKey, _ := common.HexStringToBytes(ownerPublicKeyStr)
	ownerPrivateKey, _ := common.HexStringToBytes(ownerPrivateKeyStr)
	secretaryGeneralPrivateKey, _ := common.HexStringToBytes(secretaryGeneralPrivateKeyStr)

	SecretaryGeneralDID, _ := getDiDFromPublicKey(secretaryGeneralPublicKey)
	needSign := true
	if err != nil {
		needSign = false
	}
	draftHash, err := common.Uint256FromHexString(draftHashStr)
	if err != nil {
		fmt.Println("wrong draft proposal hash")
		os.Exit(1)
	}

	account := client.GetMainAccount()
	CRCouncilMembercode := account.RedeemScript
	CRCouncilMemberDID, _ := getDIDFromCode(CRCouncilMembercode)

	fmt.Printf("account %+v\n", account)

	crcProposal := &payload.CRCProposal{
		ProposalType:              payload.CRCProposalType(proposalType),
		OwnerPublicKey:            ownPublicKey,
		DraftHash:                 *draftHash,
		SecretaryGeneralPublicKey: secretaryGeneralPublicKey,
		SecretaryGeneralDID:       *SecretaryGeneralDID,
		CRCouncilMemberDID:        *CRCouncilMemberDID,
	}

	if needSign {
		signBuf := new(bytes.Buffer)
		err = crcProposal.SerializeUnsigned(signBuf, payload.CRCProposalVersion)
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		sig, err := crypto.Sign(ownerPrivateKey, signBuf.Bytes())
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		crcProposal.Signature = sig

		secretaryGeneralSig, err := crypto.Sign(secretaryGeneralPrivateKey, signBuf.Bytes())
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		crcProposal.SecretaryGeneraSignature = secretaryGeneralSig

		if err = common.WriteVarBytes(signBuf, sig); err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		if err = common.WriteVarBytes(signBuf, secretaryGeneralSig); err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		if err = crcProposal.CRCouncilMemberDID.Serialize(signBuf); err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		crSig, err := crypto.Sign(account.PrivKey(), signBuf.Bytes())

		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		crcProposal.CRCouncilMemberSignature = crSig
	}
	ud := L.NewUserData()
	ud.Value = crcProposal
	L.SetMetatable(ud, L.GetTypeMetatable(luaCRCProposalName))
	L.Push(ud)

	fmt.Println("newSecretaryGeneralProposal end")
	return 1
}

// Constructor
func newCRCProposal(L *lua.LState) int {
	publicKeyStr := L.ToString(1)
	proposalType := L.ToInt64(2)
	draftHashStr := L.ToString(3)

	budgetsTable := L.ToTable(4)
	recipientStr := L.ToString(5)
	needSign := true
	client, err := checkClient(L, 6)
	if err != nil {
		needSign = false
	}
	draftHash, err := common.Uint256FromHexString(draftHashStr)
	if err != nil {
		fmt.Println("wrong draft proposal hash")
		os.Exit(1)
	}

	budgets := make([]payload.Budget, 0)
	budgetsTable.ForEach(func(i, value lua.LValue) {
		index := lua.LVAsNumber(i) - 1
		budgetStr := lua.LVAsString(value)
		budgetStr = strings.Replace(budgetStr, "{", "", 1)
		budgetStr = strings.Replace(budgetStr, "}", "", 1)
		amount, _ := common.StringToFixed64(budgetStr)
		var budgetType = payload.NormalPayment
		if int(index) == 0 {
			budgetType = payload.Imprest
		}
		if int(index) == budgetsTable.Len()-1 {
			budgetType = payload.FinalPayment
		}
		budget := &payload.Budget{
			Stage:  byte(int(index)),
			Type:   budgetType,
			Amount: *amount,
		}
		budgets = append(budgets, *budget)
	})

	publicKey, err := common.HexStringToBytes(publicKeyStr)
	if err != nil {
		fmt.Println("wrong cr public key")
		os.Exit(1)
	}

	pk, err := crypto.DecodePoint(publicKey)
	if err != nil {
		fmt.Println("wrong cr public key")
		os.Exit(1)
	}

	ct, err := contract.CreateStandardContract(pk)
	if err != nil {
		fmt.Println("wrong cr public key")
		os.Exit(1)
	}
	recipient, err := common.Uint168FromAddress(recipientStr)
	if err != nil {
		fmt.Println("wrong cr proposal ELA recipient")
		os.Exit(1)
	}
	did, _ := getDIDFromCode(ct.Code)
	crcProposal := &payload.CRCProposal{
		ProposalType:       payload.CRCProposalType(proposalType),
		OwnerPublicKey:     publicKey,
		DraftHash:          *draftHash,
		Budgets:            budgets,
		Recipient:          *recipient,
		CRCouncilMemberDID: *did,
	}

	if needSign {
		signBuf := new(bytes.Buffer)
		err = crcProposal.SerializeUnsigned(signBuf, payload.CRCProposalVersion)
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		codeHash, err := contract.PublicKeyToStandardCodeHash(publicKey)

		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}

		acc := client.GetAccountByCodeHash(*codeHash)
		if acc == nil {
			fmt.Println("no available account in wallet")
			os.Exit(1)
		}

		sig, err := crypto.Sign(acc.PrivKey(), signBuf.Bytes())
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		crcProposal.Signature = sig
		if err = common.WriteVarBytes(signBuf, sig); err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		if err = crcProposal.CRCouncilMemberDID.Serialize(signBuf); err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		crSig, err := crypto.Sign(acc.PrivKey(), signBuf.Bytes())

		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		crcProposal.CRCouncilMemberSignature = crSig
	}
	ud := L.NewUserData()
	ud.Value = crcProposal
	L.SetMetatable(ud, L.GetTypeMetatable(luaCRCProposalName))
	L.Push(ud)

	return 1
}

func newCRChangeProposalOwner(L *lua.LState) int {
	proposalType := L.ToInt64(1)
	recipientStr := L.ToString(2)
	targetHashStr := L.ToString(3)
	ownerPublicKeyStr := L.ToString(4)
	ownerPrivateKeyStr := L.ToString(5)
	newOwnerPublicKeyStr := L.ToString(6)
	newOwnerPrivateKeyStr := L.ToString(7)

	needSign := true
	client, err := checkClient(L, 8)
	if err != nil {
		needSign = false
	}

	targetHash, err := common.Uint256FromHexString(targetHashStr)
	if err != nil {
		fmt.Println("wrong target ProposalHash")
		os.Exit(1)
	}

	recipient := &common.Uint168{}
	if recipientStr != "" {
		recipient, err = common.Uint168FromAddress(recipientStr)
		if err != nil {
			fmt.Println("wrong cr proposal ELA recipient")
			os.Exit(1)
		}
	}

	ownerPublicKey, err := common.HexStringToBytes(ownerPublicKeyStr)
	if err != nil {
		fmt.Println("wrong cr proposal owner public key")
		os.Exit(1)
	}
	ownerPrivateKey, err := common.HexStringToBytes(ownerPrivateKeyStr)
	if err != nil {
		fmt.Println("wrong cr proposal owner private key")
		os.Exit(1)
	}

	newOwnerPublicKey, err := common.HexStringToBytes(newOwnerPublicKeyStr)
	if err != nil {
		fmt.Println("wrong new cr proposal owner public key")
		os.Exit(1)
	}

	newOwnerPrivateKey, err := common.HexStringToBytes(newOwnerPrivateKeyStr)
	if err != nil {
		fmt.Println("wrong new cr proposal owner private key")
		os.Exit(1)
	}

	account := client.GetMainAccount()
	CRCouncilMembercode := account.RedeemScript
	CRCouncilMemberDID, _ := getDIDFromCode(CRCouncilMembercode)

	fmt.Println("-----newCRChangeProposalOwner------")
	fmt.Println("proposalType", proposalType)
	fmt.Println("recipient", recipientStr)
	fmt.Println("targetHashStr", targetHashStr)
	fmt.Println("ownerPublicKeyStr", ownerPublicKeyStr)
	fmt.Println("ownerPrivateStr", ownerPrivateKeyStr)
	fmt.Println("newOwnerPublicKeyStr", newOwnerPublicKeyStr)
	fmt.Println("newOwnerPrivateKeyStr", newOwnerPrivateKeyStr)
	fmt.Printf("account %+v\n", account)
	fmt.Println("-----newCRChangeProposalOwner------")

	crcProposal := &payload.CRCProposal{
		ProposalType:       payload.CRCProposalType(proposalType),
		OwnerPublicKey:     ownerPublicKey,
		Recipient:          *recipient,
		TargetProposalHash: *targetHash,
		NewOwnerPublicKey:  newOwnerPublicKey,
		CRCouncilMemberDID: *CRCouncilMemberDID,
		NewOwnerSignature:  []byte{},
	}

	if needSign {
		signBuf := new(bytes.Buffer)
		err = crcProposal.SerializeUnsigned(signBuf, payload.CRCProposalVersion)
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}

		sig, err := crypto.Sign(ownerPrivateKey, signBuf.Bytes())
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		crcProposal.Signature = sig

		newOwnerSig, err := crypto.Sign(newOwnerPrivateKey, signBuf.Bytes())
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		crcProposal.NewOwnerSignature = newOwnerSig

		if err = common.WriteVarBytes(signBuf, sig); err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		if err = common.WriteVarBytes(signBuf, newOwnerSig); err != nil {
			fmt.Println(err)
			os.Exit(1)
		}

		if err = crcProposal.CRCouncilMemberDID.Serialize(signBuf); err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		crSig, err := crypto.Sign(account.PrivKey(), signBuf.Bytes())
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		crcProposal.CRCouncilMemberSignature = crSig
	}
	ud := L.NewUserData()
	ud.Value = crcProposal
	L.SetMetatable(ud, L.GetTypeMetatable(luaCRChangeProposalOwnerName))
	L.Push(ud)

	return 1

}

// Constructor
func newCRCCloseProposalHash(L *lua.LState) int {
	publicKeyStr := L.ToString(1)
	proposalType := L.ToInt64(2)
	draftHashStr := L.ToString(3)
	closeProposalHashStr := L.ToString(4)

	needSign := true
	client, err := checkClient(L, 5)
	if err != nil {
		needSign = false
	}
	draftHash, err := common.Uint256FromHexString(draftHashStr)
	if err != nil {
		fmt.Println("wrong draft proposal hash")
		os.Exit(1)
	}
	closeProposalHash, err := common.Uint256FromHexString(closeProposalHashStr)
	if err != nil {
		fmt.Println("wrong closeProposalHash")
		os.Exit(1)
	}

	publicKey, err := common.HexStringToBytes(publicKeyStr)
	if err != nil {
		fmt.Println("wrong cr public key")
		os.Exit(1)
	}

	pk, err := crypto.DecodePoint(publicKey)
	if err != nil {
		fmt.Println("wrong cr public key")
		os.Exit(1)
	}

	ct, err := contract.CreateStandardContract(pk)
	if err != nil {
		fmt.Println("wrong cr public key")
		os.Exit(1)
	}
	did, _ := getDIDFromCode(ct.Code)
	crcProposal := &payload.CRCProposal{
		ProposalType:       payload.CRCProposalType(proposalType),
		OwnerPublicKey:     publicKey,
		DraftHash:          *draftHash,
		TargetProposalHash: *closeProposalHash,
		CRCouncilMemberDID: *did,
	}

	if needSign {
		signBuf := new(bytes.Buffer)
		err = crcProposal.SerializeUnsigned(signBuf, payload.CRCProposalVersion)
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		codeHash, err := contract.PublicKeyToStandardCodeHash(publicKey)

		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}

		acc := client.GetAccountByCodeHash(*codeHash)
		if acc == nil {
			fmt.Println("no available account in wallet")
			os.Exit(1)
		}

		sig, err := crypto.Sign(acc.PrivKey(), signBuf.Bytes())
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		crcProposal.Signature = sig
		if err = common.WriteVarBytes(signBuf, sig); err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		if err = crcProposal.CRCouncilMemberDID.Serialize(signBuf); err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		crSig, err := crypto.Sign(acc.PrivKey(), signBuf.Bytes())

		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		crcProposal.CRCouncilMemberSignature = crSig
	}
	ud := L.NewUserData()
	ud.Value = crcProposal
	L.SetMetatable(ud, L.GetTypeMetatable(luaCRCCloseProposalHashName))
	L.Push(ud)

	return 1
}

func getCodeHexStr(publicKey string) string {
	pkBytes, _ := common.HexStringToBytes(publicKey)
	pk, _ := crypto.DecodePoint(pkBytes)
	redeemScript, _ := contract.CreateStandardRedeemScript(pk)
	codeHexStr := common.BytesToHexString(redeemScript)
	return codeHexStr
}

// Checks whether the first lua argument is a *LUserData with *CRInfo and
// returns this *CRInfo.
func checkCRCProposal(L *lua.LState, idx int) *payload.CRCProposal {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*payload.CRCProposal); ok {
		return v
	}
	L.ArgError(1, "CRCProposal expected")
	return nil
}

var crcProposalMethods = map[string]lua.LGFunction{
	"get": crcProposalGet,
}

// Getter and setter for the Person#Name
func crcProposalGet(L *lua.LState) int {
	p := checkCRCProposal(L, 1)
	fmt.Println(p)
	return 0
}

func checkCRCProposalReview(L *lua.LState, idx int) *payload.CRCProposalReview {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*payload.CRCProposalReview); ok {
		return v
	}
	L.ArgError(1, "CRCProposalReview expected")
	return nil
}

func RegisterCRCProposalReviewType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaCRCProposalReviewName)
	L.SetGlobal("crcproposalreview", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newCRCProposalReview))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), crcProposalReviewMethods))
}

func getDiDFromPublicKey(publicKey []byte) (*common.Uint168, error) {
	if code, err := getCode(publicKey); err != nil {
		return nil, err
	} else {
		return getDIDFromCode(code)
	}
}

func getDIDFromCode(code []byte) (*common.Uint168, error) {
	newCode := make([]byte, len(code))
	copy(newCode, code)
	didCode := append(newCode[:len(newCode)-1], common.DID)

	if ct1, err := contract.CreateCRIDContractByCode(didCode); err != nil {
		return nil, err
	} else {
		return ct1.ToProgramHash(), nil
	}
}

func getCode(publicKey []byte) ([]byte, error) {
	if pk, err := crypto.DecodePoint(publicKey); err != nil {
		return nil, err
	} else {
		if redeemScript, err := contract.CreateStandardRedeemScript(pk); err != nil {
			return nil, err
		} else {
			return redeemScript, nil
		}
	}
}

// Constructor
func newCRCProposalReview(L *lua.LState) int {
	fmt.Println("newCRCProposalReview begin")

	proposalHashString := L.ToString(1)
	voteResult := L.ToInt(2)
	code := L.ToString(3)
	opinionHashStr := L.ToString(4)

	needSign := true
	client, err := checkClient(L, 5)
	if err != nil {
		needSign = false
	}
	proposalHash, _ := common.Uint256FromHexString(proposalHashString)
	codeByte, _ := common.HexStringToBytes(code)
	opinionHash := &common.Uint256{}
	if opinionHashStr != "" {
		var err error
		opinionHash, err = common.Uint256FromHexString(opinionHashStr)
		if err != nil {
			return 1
		}
	}
	did, _ := getDIDFromCode(codeByte)
	crcProposalReview := &payload.CRCProposalReview{
		ProposalHash: *proposalHash,
		VoteResult:   payload.VoteResult(voteResult),
		OpinionHash:  *opinionHash,
		DID:          *did,
	}
	if needSign {
		rpSignBuf := new(bytes.Buffer)
		err = crcProposalReview.SerializeUnsigned(rpSignBuf, payload.CRCProposalReviewVersion)
		codeHash := common.ToCodeHash(codeByte)
		fmt.Println("newCRCProposalReview codeHash", common.BytesToHexString(codeHash.Bytes()))
		acc := client.GetAccountByCodeHash(*codeHash)
		if acc == nil {
			fmt.Println("no available account in wallet")
			os.Exit(1)
		}
		rpSig, err := crypto.Sign(acc.PrivKey(), rpSignBuf.Bytes())
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		crcProposalReview.Signature = rpSig
	}

	ud := L.NewUserData()
	ud.Value = crcProposalReview
	L.SetMetatable(ud, L.GetTypeMetatable(luaCRCProposalReviewName))
	L.Push(ud)
	fmt.Println("newCRCProposalReview end")
	return 1
}

var crcProposalReviewMethods = map[string]lua.LGFunction{
	"get": crcProposalReviewGet,
}

// Getter and setter for the Person#Name
func crcProposalReviewGet(L *lua.LState) int {
	p := checkCRCProposalReview(L, 1)
	fmt.Println(p)

	return 0
}

func RegisterCRCProposalTrackingType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaCRCProposalTrackingName)
	L.SetGlobal("crcproposaltracking", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newCRCProposalTracking))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), crcProposalTrackingMethods))
}

// Constructor
func newCRCProposalTracking(L *lua.LState) int {
	fmt.Println("newCRCProposalTracking begin====")
	proposalTrackingType := L.ToInt64(1)
	proposalHashStr := L.ToString(2)
	MessageHashStr := L.ToString(3)

	stage := L.ToInt64(4)
	ownerpublickeyStr := L.ToString(5)
	ownerprivatekeyStr := L.ToString(6)
	newownerpublickeyStr := L.ToString(7)
	newownerprivatekeyStr := L.ToString(8)
	sgPrivateKeyStr := L.ToString(9)
	SecretaryGeneralOpinionHashStr := L.ToString(10)
	proposalHash, _ := common.Uint256FromHexString(proposalHashStr)
	MessageHash, _ := common.Uint256FromHexString(MessageHashStr)
	opinionHash := &common.Uint256{}

	if SecretaryGeneralOpinionHashStr != "" {
		var err error
		opinionHash, err = common.Uint256FromHexString(SecretaryGeneralOpinionHashStr)
		if err != nil {
			return 1
		}
	}
	ownerpublickey, _ := common.HexStringToBytes(ownerpublickeyStr)
	ownerprivatekey, _ := common.HexStringToBytes(ownerprivatekeyStr)
	newownerpublickey, _ := common.HexStringToBytes(newownerpublickeyStr)
	newownerprivatekey, _ := common.HexStringToBytes(newownerprivatekeyStr)
	sgPrivateKey, _ := common.HexStringToBytes(sgPrivateKeyStr)

	cPayload := &payload.CRCProposalTracking{
		ProposalTrackingType:        payload.CRCProposalTrackingType(proposalTrackingType),
		ProposalHash:                *proposalHash,
		MessageHash:                 *MessageHash,
		SecretaryGeneralOpinionHash: *opinionHash,
		Stage:                       uint8(stage),
		OwnerPublicKey:              ownerpublickey,
		NewOwnerPublicKey:           newownerpublickey,
		OwnerSignature:              []byte{},
		NewOwnerSignature:           []byte{},
		SecretaryGeneralSignature:   []byte{},
	}

	signBuf := new(bytes.Buffer)
	cPayload.SerializeUnsigned(signBuf, payload.CRCProposalTrackingVersion)
	sig, _ := crypto.Sign(ownerprivatekey, signBuf.Bytes())
	cPayload.OwnerSignature = sig

	if len(newownerpublickey) != 0 && len(newownerprivatekey) != 0 {
		common.WriteVarBytes(signBuf, sig)
		crSig, _ := crypto.Sign(newownerprivatekey, signBuf.Bytes())
		cPayload.NewOwnerSignature = crSig
		sig = crSig
	}

	common.WriteVarBytes(signBuf, sig)

	//w.Write([]byte{byte(p.ProposalTrackingType)})
	if proposalTrackingType != int64(payload.ChangeOwner) {
		err := common.WriteVarBytes(signBuf, cPayload.NewOwnerSignature)
		if err != nil {
			fmt.Println("WriteVarBytes NewOwnerSignature error", err)
		}
	}
	signBuf.Write([]byte{byte(cPayload.ProposalTrackingType)})
	cPayload.SecretaryGeneralOpinionHash.Serialize(signBuf)

	crSig, _ := crypto.Sign(sgPrivateKey, signBuf.Bytes())
	cPayload.SecretaryGeneralSignature = crSig
	ud := L.NewUserData()
	ud.Value = cPayload
	L.SetMetatable(ud, L.GetTypeMetatable(luaCRCProposalTrackingName))
	L.Push(ud)
	fmt.Println("newCRCProposalTracking end====")

	return 1
}

var crcProposalTrackingMethods = map[string]lua.LGFunction{
	"get": crcProposalTrackingGet,
}

// Getter and setter for the Person#Name
func crcProposalTrackingGet(L *lua.LState) int {
	p := checkCRCProposalTracking(L, 1)
	fmt.Println(p)

	return 0
}

func checkCRCProposalTracking(L *lua.LState, idx int) *payload.CRCProposalTracking {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*payload.CRCProposalTracking); ok {
		return v
	}
	L.ArgError(1, "CRCProposalTracking expected")
	return nil
}

func RegisterCRCProposalWithdrawType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaCRCProposalWithdrawName)
	L.SetGlobal("crcproposalwithdraw", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newCRCProposalWithdraw))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(),
		crcProposalWithdrawMethods))
}

func getPublicKeyFromCode(code []byte) []byte {
	return code[1 : len(code)-1]
}

// Constructor
func newCRCProposalWithdraw(L *lua.LState) int {
	proposalHashString := L.ToString(1)
	client, err := checkClient(L, 2)
	payloadversion := L.ToInt(3)
	receipt := L.ToString(4)
	amount := L.ToInt64(5)
	fee := L.ToInt64(6)

	if err != nil {
		fmt.Println("err != nil wallet expected")
		os.Exit(1)
	}
	proposalHash, _ := common.Uint256FromHexString(proposalHashString)
	crcProposalWithdraw := &payload.CRCProposalWithdraw{
		ProposalHash: *proposalHash,
	}
	rpSignBuf := new(bytes.Buffer)
	acc := client.GetMainAccount()
	if acc == nil {
		fmt.Println("no available account in wallet")
		os.Exit(1)
	}
	pubkey := getPublicKeyFromCode(acc.RedeemScript)
	crcProposalWithdraw.OwnerPublicKey = pubkey
	if payloadversion == 1 {
		r, err := common.Uint168FromAddress(receipt)
		if err != nil {
			fmt.Println("invalid receipt")
			os.Exit(1)
		}
		crcProposalWithdraw.Recipient = *r
		crcProposalWithdraw.Amount = common.Fixed64(amount - fee)
	}
	err = crcProposalWithdraw.SerializeUnsigned(rpSignBuf, byte(payloadversion))
	rpSig, err := crypto.Sign(acc.PrivKey(), rpSignBuf.Bytes())
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
	crcProposalWithdraw.Signature = rpSig

	ud := L.NewUserData()
	ud.Value = crcProposalWithdraw
	L.SetMetatable(ud, L.GetTypeMetatable(luaCRCProposalWithdrawName))
	L.Push(ud)
	return 1
}

var crcProposalWithdrawMethods = map[string]lua.LGFunction{
	"get": crcProposalWithdrawGet,
}

// Getter and setter for the Person#Name
func crcProposalWithdrawGet(L *lua.LState) int {
	p := checkCRCProposalWithdraw(L, 1)
	fmt.Println(p)

	return 0
}

func checkCRCProposalWithdraw(L *lua.LState, idx int) *payload.CRCProposalWithdraw {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*payload.CRCProposalWithdraw); ok {
		return v
	}
	L.ArgError(1, "CRCProposalWithdraw expected")
	return nil
}
