package wallet

import (
	"encoding/hex"
	"errors"
	"fmt"
	"github.com/elastos/Elastos.ELA/account"
	cmdcom "github.com/elastos/Elastos.ELA/cmd/common"
	"github.com/elastos/Elastos.ELA/common"
	pg "github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"math/rand"
	"strconv"

	"github.com/urfave/cli"
)

type Transfer struct {
	Address string
	Amount  *common.Fixed64
}

func CreateTransaction(c *cli.Context) error {
	feeStr := c.String("fee")
	if feeStr == "" {
		return errors.New("use --fee to specify transfer fee")
	}

	fee, err := common.StringToFixed64(feeStr)
	if err != nil {
		return errors.New("invalid transaction fee")
	}

	from := c.String("from")
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
			txn, err = createTransaction(from, fee, uint32(0), &Transfer{to, amount})
			if err != nil {
				return errors.New("create transaction failed: " + err.Error())
			}
		} else {
			lock, err := strconv.ParseUint(lockStr, 10, 32)
			if err != nil {
				return errors.New("invalid lock height")
			}
			txn, err = createTransaction(from, fee, uint32(lock), &Transfer{to, amount})
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

func createTransaction(from string, fee *common.Fixed64, lockedUntil uint32, outputs ...*Transfer) (*types.Transaction, error) {
	// Check output
	if len(outputs) == 0 {
		return nil, errors.New("[Wallet], Invalid transaction target")
	}

	// Check from address
	password, err := cmdcom.GetPassword()
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

	if from != "" && from != acc.Address {
		programHash, err := common.Uint168FromAddress(from)
		if err != nil {
			return nil, err
		}
		acc = client.GetAccountByCodeHash(programHash.ToCodeHash())
		if acc == nil {
			return nil, errors.New(from + " is not local account")
		}
	}
	from = acc.Address

	spender, err := common.Uint168FromAddress(from)
	if err != nil {
		return nil, errors.New(fmt.Sprint("[Wallet], Invalid spender address: ", from, ", error: ", err))
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
			AssetID:     *account.SystemAssetID,
			ProgramHash: *receiver,
			Value:       *output.Amount,
			OutputLock:  lockedUntil,
			Type:        types.OTNone,
			Payload:     &outputpayload.DefaultOutput{},
		}
		totalOutputAmount += *output.Amount
		txOutputs = append(txOutputs, txOutput)
	}

	availableUTXOs, _, err := getAddressUTXOs(from)
	if err != nil {
		return nil, err
	}

	// Create transaction inputs
	var txInputs []*types.Input // The inputs in transaction
	for _, utxo := range availableUTXOs {
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
				AssetID:     *account.SystemAssetID,
				Value:       *amount - totalOutputAmount,
				OutputLock:  uint32(0),
				ProgramHash: *spender,
				Type:        types.OTNone,
				Payload:     &outputpayload.DefaultOutput{},
			}
			txOutputs = append(txOutputs, change)
			totalOutputAmount = 0
			break
		}
	}
	if totalOutputAmount > 0 {
		return nil, errors.New("[Wallet], Available token is not enough")
	}

	return newTransaction(acc.RedeemScript, txInputs, txOutputs, types.TransferAsset), nil
}

func newTransaction(redeemScript []byte, inputs []*types.Input, outputs []*types.Output, txType types.TxType) *types.Transaction {
	txPayload := &payload.TransferAsset{}
	txAttr := types.NewAttribute(types.Nonce, []byte(strconv.FormatInt(rand.Int63(), 10)))
	attributes := make([]*types.Attribute, 0)
	attributes = append(attributes, &txAttr)
	var program = &pg.Program{
		Code:      redeemScript,
		Parameter: nil,
	}

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
