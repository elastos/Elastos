package transfer

import (
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"math/rand"
	"os"
	"strconv"
	"strings"

	"github.com/elastos/Elastos.ELA/account"
	clicom "github.com/elastos/Elastos.ELA/cli/common"
	"github.com/elastos/Elastos.ELA/cli/password"
	"github.com/elastos/Elastos.ELA/common"
	pg "github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/servers"

	"github.com/elastos/Elastos.ELA.Utility/http/jsonrpc"
	httputil "github.com/elastos/Elastos.ELA.Utility/http/util"
	"github.com/urfave/cli"
)

type Transfer struct {
	Address string
	Amount  *common.Fixed64
}

func createTransaction(c *cli.Context) error {
	feeStr := c.String("fee")
	if feeStr == "" {
		return errors.New("use --fee to specify transfer fee")
	}

	fee, err := common.StringToFixed64(feeStr)
	if err != nil {
		return errors.New("invalid transaction fee")
	}

	from := c.String("from")

	//multiOutput := c.String("file")
	//if multiOutput != "" {
	//	return createMultiOutputTransaction(c, wallet, multiOutput, from, fee)
	//}

	amountStr := c.String("amount")
	if amountStr == "" {
		return errors.New("use --amount to specify transfer amount")
	}

	amount, err := common.StringToFixed64(amountStr)
	if err != nil {
		return errors.New("invalid transaction amount")
	}

	var txn *types.Transaction
	var to string
	standard := c.String("to")
	deposit := c.String("deposit")
	if deposit != "" {
		// TODO fix cross chain tx
		//to = config.Params().DepositAddress
		//txn, err = wallet.CreateCrossChainTransaction(from, to, deposit, amount, fee)
		//if err != nil {
		//	return errors.New("create transaction failed: " + err.Error())
		//}
	} else if standard != "" {
		to = standard
		lockStr := c.String("lock")
		if lockStr == "" {
			txn, err = CreateTransaction(from, to, amount, fee)
			if err != nil {
				return errors.New("create transaction failed: " + err.Error())
			}
		} else {
			lock, err := strconv.ParseUint(lockStr, 10, 32)
			if err != nil {
				return errors.New("invalid lock height")
			}
			txn, err = CreateLockedTransaction(from, to, amount, fee, uint32(lock))
			if err != nil {
				return errors.New("create transaction failed: " + err.Error())
			}
		}
	} else {
		return errors.New("use --to or --deposit to specify receiver address")
	}

	output(0, 0, txn)

	return nil
}

func getTransactionContent(context *cli.Context) (string, error) {
	// If parameter with file path is not empty, read content from file
	if filePath := strings.TrimSpace(context.String("file")); filePath != "" {

		if _, err := os.Stat(filePath); err != nil {
			return "", errors.New("invalid transaction file path")
		}
		file, err := os.OpenFile(filePath, os.O_RDONLY, 0666)
		if err != nil {
			return "", errors.New("open transaction file failed")
		}
		rawData, err := ioutil.ReadAll(file)
		if err != nil {
			return "", errors.New("read transaction file failed")
		}

		content := strings.TrimSpace(string(rawData))
		// File content can not by empty
		if content == "" {
			return "", errors.New("transaction file is empty")
		}
		return content, nil
	}

	content := strings.TrimSpace(context.String("hex"))
	// Hex string content can not be empty
	if content == "" {
		return "", errors.New("transaction hex string is empty")
	}

	return content, nil
}

func CreateTransaction(fromAddress, toAddress string, amount, fee *common.Fixed64) (*types.Transaction, error) {
	return CreateLockedTransaction(fromAddress, toAddress, amount, fee, uint32(0))
}

func CreateLockedTransaction(fromAddress, toAddress string, amount, fee *common.Fixed64, lockedUntil uint32) (*types.Transaction, error) {
	return CreateLockedMultiOutputTransaction(fromAddress, fee, lockedUntil, &Transfer{toAddress, amount})
}

func CreateMultiOutputTransaction(fromAddress string, fee *common.Fixed64, outputs ...*Transfer) (*types.Transaction, error) {
	return CreateLockedMultiOutputTransaction(fromAddress, fee, uint32(0), outputs...)
}

func CreateLockedMultiOutputTransaction(fromAddress string, fee *common.Fixed64, lockedUntil uint32, outputs ...*Transfer) (*types.Transaction, error) {
	return createTransaction_(fromAddress, fee, lockedUntil, outputs...)
}

func createTransaction_(fromAddress string, fee *common.Fixed64, lockedUntil uint32, outputs ...*Transfer) (*types.Transaction, error) {
	// Check if output is valid
	if len(outputs) == 0 {
		return nil, errors.New("[Wallet], Invalid transaction target")
	}

	// Check from address
	password, err := password.GetPassword()
	if err != nil {
		return nil, err
	}
	client, err := account.Open(account.KeystoreFileName, password)
	if err != nil {
		return nil, err
	}
	acc, err := client.GetDefaultAccount()
	if err != nil {
		return nil, err
	}

	if fromAddress != "" && fromAddress != acc.Address {
		programHash, err := common.Uint168FromAddress(fromAddress)
		if err != nil {
			return nil, err
		}
		acc = client.GetAccountByCodeHash(programHash.ToCodeHash())
		if acc == nil {
			return nil, errors.New(fromAddress + " is not local account")
		}
	}
	fromAddress = acc.Address

	// Check if from address is valid
	spender, err := common.Uint168FromAddress(fromAddress)
	if err != nil {
		return nil, errors.New(fmt.Sprint("[Wallet], Invalid spender address: ", fromAddress, ", error: ", err))
	}
	// Create transaction outputs
	var totalOutputAmount = common.Fixed64(0) // The total amount will be spend
	var txOutputs []*types.Output             // The outputs in transaction
	totalOutputAmount += *fee                 // Add transaction fee

	for _, output := range outputs {
		receiver, err := common.Uint168FromAddress(output.Address)
		if err != nil {
			return nil, errors.New(fmt.Sprint("[Wallet], Invalid receiver address: ", output.Address, ", error: ", err))
		}

		txOutput := &types.Output{
			AssetID:       *account.SystemAssetID,
			ProgramHash:   *receiver,
			Value:         *output.Amount,
			OutputLock:    lockedUntil,
			OutputType:    types.DefaultOutput,
			OutputPayload: &outputpayload.DefaultOutput{},
		}
		totalOutputAmount += *output.Amount
		txOutputs = append(txOutputs, txOutput)
	}

	result, err := jsonrpc.CallParams(clicom.LocalServer(), "listunspent", httputil.Params{
		"addresses": []string{fromAddress},
	})
	if err != nil {
		return nil, err
	}
	data, err := json.Marshal(result)
	if err != nil {
		return nil, err
	}
	var utxos []servers.UTXOInfo
	err = json.Unmarshal(data, &utxos)

	var availabelUtxos []servers.UTXOInfo
	for _, utxo := range utxos {
		if types.TxType(utxo.TxType) == types.CoinBase && utxo.Confirmations < 100 {
			continue
		}
		availabelUtxos = append(availabelUtxos, utxo)
	}

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
		if *amount < totalOutputAmount {
			totalOutputAmount -= *amount
		} else if *amount == totalOutputAmount {
			totalOutputAmount = 0
			break
		} else if *amount > totalOutputAmount {
			change := &types.Output{
				AssetID:       *account.SystemAssetID,
				Value:         *amount - totalOutputAmount,
				OutputLock:    uint32(0),
				ProgramHash:   *spender,
				OutputType:    types.DefaultOutput,
				OutputPayload: &outputpayload.DefaultOutput{},
			}
			txOutputs = append(txOutputs, change)
			totalOutputAmount = 0
			break
		}
	}
	if totalOutputAmount > 0 {
		return nil, errors.New("[Wallet], Available token is not enough")
	}

	return newTransaction(acc.Contract.Code, txInputs, txOutputs, types.TransferAsset), nil
}

func newTransaction(redeemScript []byte, inputs []*types.Input, outputs []*types.Output, txType types.TxType) *types.Transaction {
	txPayload := &payload.PayloadTransferAsset{}
	txAttr := types.NewAttribute(types.Nonce, []byte(strconv.FormatInt(rand.Int63(), 10)))
	attributes := make([]*types.Attribute, 0)
	attributes = append(attributes, &txAttr)
	var program = &pg.Program{redeemScript, nil}

	return &types.Transaction{
		Version:    types.TxVersion09,
		TxType:     txType,
		Payload:    txPayload,
		Attributes: attributes,
		Inputs:     inputs,
		Outputs:    outputs,
		Programs:   []*pg.Program{program},
		LockTime:   0,
	}
}
