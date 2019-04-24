package wallet

import (
	"bytes"
	"encoding/hex"
	"errors"
	"fmt"
	"math/rand"
	"strconv"

	"github.com/elastos/Elastos.ELA/account"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
	pg "github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/utils"

	"github.com/urfave/cli"
)

type Transfer struct {
	Address string
	Amount  *common.Fixed64
}

func CreateTransaction(c *cli.Context) error {
	walletPath := c.String("wallet")

	feeStr := c.String("fee")
	if feeStr == "" {
		return errors.New("use --fee to specify transfer fee")
	}

	fee, err := common.StringToFixed64(feeStr)
	if err != nil {
		return errors.New("invalid transaction fee")
	}

	from := c.String("from")

	outputs := make([]*Transfer, 0)
	to := c.String("to")
	amountStr := c.String("amount")
	toMany := c.String("tomany")
	if toMany != "" {
		if to != "" {
			return errors.New("'--to' cannot be specified when specify '--tomany' option")
		}
		if amountStr != "" {
			return errors.New("'--amount' cannot be specified when specify '--tomany' option")
		}
		outputs, err = parseMultiOutput(toMany)
		if err != nil {
			return err
		}
	} else {
		if amountStr == "" {
			return errors.New("use --amount to specify transfer amount")
		}

		amount, err := common.StringToFixed64(amountStr)
		if err != nil {
			return errors.New("invalid transaction amount")
		}
		outputs = []*Transfer{{to, amount}}
	}

	lockStr := c.String("lock")
	lock := uint64(0)
	if lockStr != "" {
		lock, err = strconv.ParseUint(lockStr, 10, 32)
		if err != nil {
			return errors.New("invalid lock height")
		}
	}

	var txn *types.Transaction
	txn, err = createTransaction(walletPath, from, fee, uint32(lock), outputs...)
	if err != nil {
		return errors.New("create transaction failed: " + err.Error())
	}

	output(0, 1, txn)

	return nil
}

func CreateActivateProducerTransaction(c *cli.Context) error {
	walletPath := c.String("wallet")
	pwdHex := c.String("password")

	pwd := []byte(pwdHex)
	if pwdHex == "" {
		var err error
		pwd, err = utils.GetPassword()
		if err != nil {
			return err
		}
	}

	client, err := account.Open(walletPath, pwd)
	if err != nil {
		return err
	}

	nodePublicKeyStr := c.String("nodepublickey")
	nodePublicKey, err := common.HexStringToBytes(nodePublicKeyStr)
	if err != nil {
		return err
	}

	codeHash, err := contract.PublicKeyToStandardCodeHash(nodePublicKey)
	if err != nil {
		return err
	}

	apPayload := &payload.ActivateProducer{
		NodePublicKey: nodePublicKey,
	}
	buf := new(bytes.Buffer)
	apPayload.SerializeUnsigned(buf, payload.ActivateProducerVersion)
	acc := client.GetAccountByCodeHash(*codeHash)
	if acc == nil {
		return errors.New("no available account in wallet")
	}
	signature, err := acc.Sign(buf.Bytes())
	if err != nil {
		return err
	}
	apPayload.Signature = signature

	txn := &types.Transaction{
		Version:    types.TxVersion09,
		TxType:     types.ActivateProducer,
		Payload:    apPayload,
		Attributes: nil,
		Inputs:     nil,
		Outputs:    nil,
		Programs:   []*pg.Program{},
		LockTime:   0,
	}

	output(0, 0, txn)

	return nil
}

func createTransaction(walletPath string, from string, fee *common.Fixed64, lockedUntil uint32, outputs ...*Transfer) (*types.Transaction, error) {
	// Check output
	if len(outputs) == 0 {
		return nil, errors.New("[Wallet], Invalid transaction target")
	}

	// Check from address
	var usingAccount *account.AccountData
	mainAccount, err := account.GetWalletMainAccountData(walletPath)
	if err != nil {
		return nil, err
	}

	if from == "" {
		from = mainAccount.Address
		usingAccount = mainAccount
	} else if from != mainAccount.Address {
		storeAccounts, err := account.GetWalletAccountData(walletPath)
		if err != nil {
			return nil, err
		}
		for _, acc := range storeAccounts {
			if from == acc.Address {
				usingAccount = &acc
				break
			}
		}
		if usingAccount == nil {
			return nil, errors.New(from + " is not local account")
		}
	}

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

	redeemScript, err := common.HexStringToBytes(usingAccount.RedeemScript)
	if err != nil {
		return nil, err
	}
	txAttr := types.NewAttribute(types.Nonce, []byte(strconv.FormatInt(rand.Int63(), 10)))
	txAttributes := make([]*types.Attribute, 0)
	txAttributes = append(txAttributes, &txAttr)
	var txProgram = &pg.Program{
		Code:      redeemScript,
		Parameter: nil,
	}

	return &types.Transaction{
		Version:    types.TxVersion09,
		TxType:     types.TransferAsset,
		Payload:    &payload.TransferAsset{},
		Attributes: txAttributes,
		Inputs:     txInputs,
		Outputs:    txOutputs,
		Programs:   []*pg.Program{txProgram},
		LockTime:   0,
	}, nil
}
