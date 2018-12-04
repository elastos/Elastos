package api

import (
	"bytes"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"os"

	clicom "github.com/elastos/Elastos.ELA/cli/common"
	pg "github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/servers"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/http/jsonrpc"
	"github.com/elastos/Elastos.ELA.Utility/http/util"
	"github.com/yuin/gopher-lua"
)

const luaTransactionTypeName = "transaction"

func RegisterTransactionType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaTransactionTypeName)
	L.SetGlobal("transaction", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newTransaction))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), transactionMethods))
}

// Constructor
//  Version		   TransactionVersion
//	TxType         TransactionType
//	PayloadVersion byte
//	Payload        Payload
//	Attributes     []*Attribute
//	Inputs     	   []*Input
//	Outputs        []*Output
//	LockTime       uint32
func newTransaction(L *lua.LState) int {
	version := L.ToInt(1)
	txType := types.TransactionType(L.ToInt(2))
	payloadVersion := byte(L.ToInt(3))
	ud := L.CheckUserData(4)
	lockTime := uint32(L.ToInt(5))

	var pload types.Payload
	switch ud.Value.(type) {
	case *payload.PayloadCoinBase:
		pload, _ = ud.Value.(*payload.PayloadCoinBase)
	case *payload.PayloadRegisterAsset:
		pload, _ = ud.Value.(*payload.PayloadRegisterAsset)
	case *payload.PayloadTransferAsset:
		pload, _ = ud.Value.(*payload.PayloadTransferAsset)
	case *payload.PayloadRecord:
		pload, _ = ud.Value.(*payload.PayloadRecord)
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
	"appendtxin":   transactionAppendInput,
	"appendtxout":  transactionAppendOutput,
	"appendattr":   transactionAppendAttribute,
	"get":          transactionGet,
	"sign":         transactionSign,
	"hash":         transactionHash,
	"serialize":    transactionSerialize,
	"deserialize":  transactionDeserialize,
	"appendenough": transactionAppendEnough,
}

// Getter and setter for the Person#Name
func transactionGet(L *lua.LState) int {
	p := checkTransaction(L, 1)
	fmt.Println(p)

	return 0
}

func transactionAppendInput(L *lua.LState) int {
	p := checkTransaction(L, 1)
	input := checkInput(L, 2)
	p.Inputs = append(p.Inputs, input)

	return 0
}

func transactionAppendAttribute(L *lua.LState) int {
	p := checkTransaction(L, 1)
	attr := checkAttribute(L, 2)
	p.Attributes = append(p.Attributes, attr)

	return 0
}

func transactionAppendOutput(L *lua.LState) int {
	txn := checkTransaction(L, 1)
	output := checkTxOutput(L, 2)
	txn.Outputs = append(txn.Outputs, output)

	return 0
}

func transactionHash(L *lua.LState) int {
	tx := checkTransaction(L, 1)
	h := tx.Hash()
	hash := common.BytesReverse(h.Bytes())

	L.Push(lua.LString(hex.EncodeToString(hash)))

	return 1
}

func transactionSign(L *lua.LState) int {
	txn := checkTransaction(L, 1)
	client := checkClient(L, 2)
	//fmt.Println("txn:", txn)
	//fmt.Println("client:", client)

	acc, err := client.GetDefaultAccount()
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}

	program := pg.Program{
		Code:      acc.Contract.RedeemScript,
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

func transactionSerialize(L *lua.LState) int {
	txn := checkTransaction(L, 1)

	var buffer bytes.Buffer
	txn.Serialize(&buffer)
	txHex := hex.EncodeToString(buffer.Bytes())

	L.Push(lua.LNumber(len(buffer.Bytes())))
	L.Push(lua.LString(txHex))
	return 2
}

func transactionDeserialize(L *lua.LState) int {
	txn := checkTransaction(L, 1)
	txSlice, _ := hex.DecodeString(L.ToString(2))

	txn.Deserialize(bytes.NewReader(txSlice))

	return 0
}

func transactionAppendEnough(L *lua.LState) int {
	txn := checkTransaction(L, 1)
	from := L.ToString(2)
	totalAmount := L.ToInt64(3)

	result, err := jsonrpc.CallParams(clicom.LocalServer(), "listunspent", util.Params{
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
		if types.TransactionType(utxo.TxType) == types.CoinBase && utxo.Confirmations < 100 {
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
