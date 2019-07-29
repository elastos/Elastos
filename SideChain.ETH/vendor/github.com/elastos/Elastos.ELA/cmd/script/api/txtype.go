package api

import (
	"bytes"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"os"

	cmdcom "github.com/elastos/Elastos.ELA/cmd/common"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
	pg "github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/servers"
	"github.com/elastos/Elastos.ELA/utils/http"

	"github.com/yuin/gopher-lua"
)

const luaTransactionTypeName = "transaction"

func RegisterTransactionType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaTransactionTypeName)
	L.SetGlobal("transaction", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newTransaction))
	L.SetField(mt, "fromfile", L.NewFunction(fromFile))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), transactionMethods))
}

// Constructor
//  Version		   TransactionVersion
//	TxType         TxType
//	PayloadVersion byte
//	Payload        Payload
//	Attributes     []*Attribute
//	Inputs     	   []*Input
//	Outputs        []*Output
//	LockTime       uint32
func newTransaction(L *lua.LState) int {
	version := L.ToInt(1)
	txType := types.TxType(L.ToInt(2))
	payloadVersion := byte(L.ToInt(3))
	ud := L.CheckUserData(4)
	lockTime := uint32(L.ToInt(5))

	var pload types.Payload
	switch ud.Value.(type) {
	case *payload.CoinBase:
		pload, _ = ud.Value.(*payload.CoinBase)
	case *payload.RegisterAsset:
		pload, _ = ud.Value.(*payload.RegisterAsset)
	case *payload.TransferAsset:
		pload, _ = ud.Value.(*payload.TransferAsset)
	case *payload.Record:
		pload, _ = ud.Value.(*payload.Record)
	case *payload.ProducerInfo:
		pload, _ = ud.Value.(*payload.ProducerInfo)
	case *payload.ProcessProducer:
		pload, _ = ud.Value.(*payload.ProcessProducer)
	case *payload.ActivateProducer:
		pload, _ = ud.Value.(*payload.ActivateProducer)
	case *payload.ReturnDepositCoin:
		pload, _ = ud.Value.(*payload.ReturnDepositCoin)
	case *payload.DPOSIllegalProposals:
		pload, _ = ud.Value.(*payload.DPOSIllegalProposals)
	case *payload.DPOSIllegalVotes:
		pload, _ = ud.Value.(*payload.DPOSIllegalVotes)
	case *payload.DPOSIllegalBlocks:
		pload, _ = ud.Value.(*payload.DPOSIllegalBlocks)
	case *payload.SidechainIllegalData:
		pload, _ = ud.Value.(*payload.SidechainIllegalData)
	case *payload.InactiveArbitrators:
		pload, _ = ud.Value.(*payload.InactiveArbitrators)
	case *payload.SideChainPow:
		pload, _ = ud.Value.(*payload.SideChainPow)
	default:
		fmt.Println("error: undefined payload type")
		os.Exit(1)
	}

	txn := &types.Transaction{
		Version:        types.TransactionVersion(version),
		TxType:         txType,
		PayloadVersion: payloadVersion,
		Payload:        pload,
		Attributes:     []*types.Attribute{},
		Inputs:         []*types.Input{},
		Outputs:        []*types.Output{},
		LockTime:       lockTime,
	}
	udn := L.NewUserData()
	udn.Value = txn
	L.SetMetatable(udn, L.GetTypeMetatable(luaTransactionTypeName))
	L.Push(udn)

	return 1
}

func fromFile(L *lua.LState) int {
	filePath := L.CheckString(1)
	content, err := cmdcom.ReadFile(filePath)
	if err != nil {
		fmt.Println(err)
	}
	txData, err := common.HexStringToBytes(content)
	if err != nil {
		fmt.Println("decode transaction content failed")
		os.Exit(1)
	}

	var txn types.Transaction
	err = txn.Deserialize(bytes.NewReader(txData))
	if err != nil {
		fmt.Println("deserialize transaction failed")
		os.Exit(1)
	}

	ud := L.NewUserData()
	ud.Value = &txn
	L.SetMetatable(ud, L.GetTypeMetatable(luaTransactionTypeName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *Transaction and returns this *Transaction.
func checkTransaction(L *lua.LState, idx int) *types.Transaction {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*types.Transaction); ok {
		return v
	}
	L.ArgError(1, "Transaction expected")
	return nil
}

var transactionMethods = map[string]lua.LGFunction{
	"appendtxin":    txAppendInput,
	"appendtxout":   txAppendOutput,
	"appendattr":    txAppendAttribute,
	"get":           txGet,
	"sign":          signTx,
	"hash":          txHash,
	"serialize":     serialize,
	"deserialize":   deserialize,
	"appendenough":  appendEnough,
	"appendprogram": appendProgram,
	"signpayload":   signPayload,
}

func signPayload(L *lua.LState) int {
	txn := checkTransaction(L, 1)
	client, err := checkClient(L, 2)
	if err != nil {
		cmdcom.PrintErrorAndExit(err.Error())
	}

	switch txn.TxType {
	case types.RegisterProducer:
		registerProducer, ok := txn.Payload.(*payload.ProducerInfo)
		if !ok {
			cmdcom.PrintErrorAndExit("invalid register producer payload")
		}
		rpSignBuf := new(bytes.Buffer)
		if err := registerProducer.SerializeUnsigned(rpSignBuf, payload.ProducerInfoVersion); err != nil {
			cmdcom.PrintErrorAndExit(err.Error())
		}

		codeHash, err := contract.PublicKeyToStandardCodeHash(registerProducer.OwnerPublicKey)
		if err != nil {
			cmdcom.PrintErrorAndExit(err.Error())
		}
		acc := client.GetAccountByCodeHash(*codeHash)
		if acc == nil {
			cmdcom.PrintErrorAndExit("no available account in wallet")
		}
		rpSig, err := crypto.Sign(acc.PrivKey(), rpSignBuf.Bytes())
		if err != nil {
			cmdcom.PrintErrorAndExit(err.Error())
		}
		registerProducer.Signature = rpSig
		txn.Payload = registerProducer
	default:
		cmdcom.PrintErrorAndExit("invalid payload")
	}

	udn := L.NewUserData()
	udn.Value = txn
	L.SetMetatable(udn, L.GetTypeMetatable(luaTransactionTypeName))
	L.Push(udn)

	return 1
}

// Getter and setter for the Person#Name
func txGet(L *lua.LState) int {
	p := checkTransaction(L, 1)
	fmt.Println(p)

	return 0
}

func txAppendInput(L *lua.LState) int {
	p := checkTransaction(L, 1)
	input := checkInput(L, 2)
	p.Inputs = append(p.Inputs, input)

	return 0
}

func txAppendAttribute(L *lua.LState) int {
	p := checkTransaction(L, 1)
	attr := checkAttribute(L, 2)
	p.Attributes = append(p.Attributes, attr)

	return 0
}

func txAppendOutput(L *lua.LState) int {
	txn := checkTransaction(L, 1)
	output := checkTxOutput(L, 2)
	txn.Outputs = append(txn.Outputs, output)

	return 0
}

func txHash(L *lua.LState) int {
	tx := checkTransaction(L, 1)
	h := tx.Hash()
	hash := common.BytesReverse(h.Bytes())

	L.Push(lua.LString(hex.EncodeToString(hash)))

	return 1
}

func signTx(L *lua.LState) int {
	txn := checkTransaction(L, 1)
	client, err := checkClient(L, 2)
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}

	acc := client.GetMainAccount()
	program := pg.Program{
		Code:      acc.RedeemScript,
		Parameter: []byte{},
	}
	txn.Programs = []*pg.Program{
		&program,
	}

	txn, err = client.Sign(txn)
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}

	return 0
}

func serialize(L *lua.LState) int {
	txn := checkTransaction(L, 1)

	var buffer bytes.Buffer
	txn.Serialize(&buffer)
	txHex := hex.EncodeToString(buffer.Bytes())

	L.Push(lua.LNumber(len(buffer.Bytes())))
	L.Push(lua.LString(txHex))
	return 2
}

func deserialize(L *lua.LState) int {
	txn := checkTransaction(L, 1)
	txSlice, _ := hex.DecodeString(L.ToString(2))

	txn.Deserialize(bytes.NewReader(txSlice))

	return 0
}

func appendEnough(L *lua.LState) int {
	txn := checkTransaction(L, 1)
	from := L.ToString(2)
	totalAmount := L.ToInt64(3)

	result, err := cmdcom.RPCCall("listunspent", http.Params{
		"addresses": []string{from},
	})
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
	data, err := json.Marshal(result)
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
	var utxos []servers.UTXOInfo
	err = json.Unmarshal(data, &utxos)

	var availabelUtxos []servers.UTXOInfo
	for _, utxo := range utxos {
		if types.TxType(utxo.TxType) == types.CoinBase && utxo.Confirmations < 101 {
			continue
		}
		availabelUtxos = append(availabelUtxos, utxo)
	}

	//totalAmount := common.Fixed64(0)
	var charge int64
	// Create transaction inputs
	var txInputs []*types.Input // The inputs in transaction
	for _, utxo := range availabelUtxos {
		txIDReverse, _ := hex.DecodeString(utxo.TxID)
		txID, _ := common.Uint256FromBytes(common.BytesReverse(txIDReverse))
		input := &types.Input{
			Previous: types.OutPoint{
				TxID:  *txID,
				Index: uint16(utxo.VOut),
			},
			Sequence: 4294967295,
		}
		txInputs = append(txInputs, input)
		amount, _ := common.StringToFixed64(utxo.Amount)
		if int64(*amount) < totalAmount {
			totalAmount -= int64(*amount)
		} else if int64(*amount) == totalAmount {
			totalAmount = 0
			break
		} else if int64(*amount) > totalAmount {
			charge = int64(*amount) - totalAmount
			totalAmount = 0
			break
		}
	}

	if totalAmount > 0 {
		fmt.Println("[Wallet], Available token is not enough")
		os.Exit(1)
	}

	txn.Inputs = txInputs
	L.Push(lua.LNumber(charge))

	return 1
}

func appendProgram(L *lua.LState) int {
	txn := checkTransaction(L, 1)
	program := checkProgram(L, 2)
	txn.Programs = append(txn.Programs, program)

	return 0
}
