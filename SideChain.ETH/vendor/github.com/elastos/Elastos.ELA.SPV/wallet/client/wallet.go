package client

import (
	"bytes"
	"encoding/hex"
	"errors"
	"math"
	"math/rand"
	"strconv"

	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA.SPV/wallet/client/database"
	"github.com/elastos/Elastos.ELA.SPV/wallet/sutil"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/utils/http/jsonrpc"
)

var (
	dataPath   string
	jsonRpcUrl string
	sysAssetId common.Uint256
)

type Transfer struct {
	Address string
	Value   *common.Fixed64
}

type Wallet struct {
	database.Database
	*Keystore
}

func Setup(dataDir, rpcUrl string, assetId common.Uint256) {
	dataPath = dataDir
	jsonRpcUrl = rpcUrl
	sysAssetId = assetId
}

func Create(password []byte) error {
	keyStore, err := CreateKeystore(password)
	if err != nil {
		return err
	}

	db, err := database.New(dataPath)
	if err != nil {
		return err
	}

	mainAccount := keyStore.GetAccountByIndex(0)
	return db.AddAddress(mainAccount.ProgramHash(),
		mainAccount.RedeemScript(), sutil.TypeMaster)
}

func Open() (*Wallet, error) {
	db, err := database.New(dataPath)
	if err != nil {
		return nil, err
	}

	return &Wallet{
		Database: db,
	}, nil
}

func (wallet *Wallet) VerifyPassword(password []byte) error {
	keyStore, err := OpenKeystore(password)
	if err != nil {
		return err
	}
	wallet.Keystore = keyStore
	return nil
}

func (wallet *Wallet) NewSubAccount(password []byte) (*common.Uint168, error) {
	err := wallet.VerifyPassword(password)
	if err != nil {
		return nil, err
	}

	account := wallet.Keystore.NewAccount()
	err = wallet.AddAddress(account.ProgramHash(), account.RedeemScript(), sutil.TypeSub)
	if err != nil {
		return nil, err
	}

	// Notify SPV service to reload bloom filter with the new address
	jsonrpc.CallArray(jsonRpcUrl, "notifynewaddress", account.ProgramHash().String())

	return account.ProgramHash(), nil
}

func (wallet *Wallet) AddMultiSignAccount(m int, publicKeys ...*crypto.PublicKey) (*common.Uint168, error) {
	contract, err := contract.CreateMultiSigContract(m, publicKeys)
	if err != nil {
		return nil, errors.New("[Wallet], CreateStandardRedeemScript failed")
	}

	programHash := contract.ToProgramHash()

	err = wallet.AddAddress(programHash, contract.Code, sutil.TypeMulti)
	if err != nil {
		return nil, err
	}

	// Notify SPV service to reload bloom filter with the new address
	jsonrpc.CallArray(jsonRpcUrl, "notifynewaddress", programHash.String())

	return programHash, nil
}

func (wallet *Wallet) CreateTransaction(fromAddress, toAddress string, amount,
	fee *common.Fixed64) (*types.Transaction, error) {
	return wallet.CreateLockedTransaction(fromAddress, toAddress, amount, fee, uint32(0))
}

func (wallet *Wallet) CreateLockedTransaction(fromAddress, toAddress string,
	amount, fee *common.Fixed64, lockedUntil uint32) (*types.Transaction, error) {
	return wallet.CreateLockedMultiOutputTransaction(fromAddress, fee, lockedUntil,
		&Transfer{toAddress, amount})
}

func (wallet *Wallet) CreateMultiOutputTransaction(fromAddress string, fee *common.Fixed64,
	outputs ...*Transfer) (*types.Transaction, error) {
	return wallet.CreateLockedMultiOutputTransaction(fromAddress, fee, uint32(0), outputs...)
}

func (wallet *Wallet) CreateLockedMultiOutputTransaction(fromAddress string, fee *common.Fixed64,
	lockedUntil uint32, outputs ...*Transfer) (*types.Transaction, error) {
	return wallet.createTransaction(fromAddress, fee, lockedUntil, outputs...)
}

func (wallet *Wallet) createTransaction(fromAddress string, fee *common.Fixed64, lockedUntil uint32,
	outputs ...*Transfer) (*types.Transaction, error) {
	// Check if output is valid
	if outputs == nil || len(outputs) == 0 {
		return nil, errors.New("[Wallet], Invalid transaction target")
	}

	// Check if from address is valid
	spender, err := common.Uint168FromAddress(fromAddress)
	if err != nil {
		return nil, errors.New("[Wallet], Invalid spender address")
	}
	// Create transaction outputs
	var totalOutputValue = common.Fixed64(0) // The total value will be spend
	var txOutputs []*types.Output            // The outputs in transaction
	totalOutputValue += *fee                 // Add transaction fee

	for _, output := range outputs {
		receiver, err := common.Uint168FromAddress(output.Address)
		if err != nil {
			return nil, errors.New("[Wallet], Invalid receiver address")
		}
		txOutput := &types.Output{
			AssetID:     sysAssetId,
			ProgramHash: *receiver,
			Value:       *output.Value,
			OutputLock:  lockedUntil,
		}
		totalOutputValue += *output.Value
		txOutputs = append(txOutputs, txOutput)
	}
	// Get spender's UTXOs
	utxos, err := wallet.GetAddressUTXOs(spender)
	if err != nil {
		return nil, errors.New("[Wallet], Get spender's UTXOs failed")
	}
	availableUTXOs := wallet.removeLockedUTXOs(utxos)  // Remove locked UTXOs
	availableUTXOs = sutil.SortByValue(availableUTXOs) // Sort available UTXOs by value ASC

	// Create transaction inputs
	var txInputs []*types.Input // The inputs in transaction
	for _, utxo := range availableUTXOs {
		txInputs = append(txInputs, InputFromUTXO(utxo))
		if utxo.Value < totalOutputValue {
			totalOutputValue -= utxo.Value
		} else if utxo.Value == totalOutputValue {
			totalOutputValue = 0
			break
		} else if utxo.Value > totalOutputValue {
			change := &types.Output{
				AssetID:     sysAssetId,
				Value:       utxo.Value - totalOutputValue,
				OutputLock:  uint32(0),
				ProgramHash: *spender,
			}
			txOutputs = append(txOutputs, change)
			totalOutputValue = 0
			break
		}
	}
	if totalOutputValue > 0 {
		return nil, errors.New("[Wallet], Available token is not enough")
	}

	addr, err := wallet.GetAddress(spender)
	if err != nil {
		return nil, errors.New("[Wallet], Get spenders redeem script failed")
	}

	return wallet.newTransaction(addr.Script(), txInputs, txOutputs), nil
}

func (wallet *Wallet) Sign(password []byte, tx *types.Transaction) (*types.Transaction, error) {
	// Verify password
	err := wallet.VerifyPassword(password)
	if err != nil {
		return nil, err
	}
	// Get sign type
	signType, err := crypto.GetScriptType(tx.Programs[0].Code)
	if err != nil {
		return nil, err
	}
	// Look up transaction type
	if signType == common.STANDARD {

		// Sign single transaction
		tx, err = wallet.signStandardTransaction(tx)
		if err != nil {
			return nil, err
		}

	} else if signType == common.MULTISIG {

		// Sign multi sign transaction
		tx, err = wallet.signMultiSigTransaction(tx)
		if err != nil {
			return nil, err
		}
	}

	return tx, nil
}

func (wallet *Wallet) signStandardTransaction(tx *types.Transaction) (*types.Transaction, error) {
	code := tx.Programs[0].Code
	// Get signer
	programHash := common.ToProgramHash(byte(contract.PrefixStandard), code)
	// Check if current user is a valid signer
	account := wallet.Keystore.GetAccountByProgramHash(programHash)
	if account == nil {
		return nil, errors.New("[Wallet], Invalid signer")
	}
	// Sign transaction
	buf := new(bytes.Buffer)
	tx.SerializeUnsigned(buf)
	signature, err := account.Sign(buf.Bytes())
	if err != nil {
		return nil, err
	}
	// Add signature
	buf = new(bytes.Buffer)
	buf.WriteByte(byte(len(signature)))
	buf.Write(signature)
	// Set program
	tx.Programs = []*program.Program{{code, buf.Bytes()}}

	return tx, nil
}

func (wallet *Wallet) signMultiSigTransaction(tx *types.Transaction) (*types.Transaction, error) {
	code := tx.Programs[0].Code
	param := tx.Programs[0].Parameter
	// Check if current user is a valid signer
	var signerIndex = -1
	publicKeys, err := crypto.ParseMultisigScript(code)
	if err != nil {
		return nil, err
	}
	var account *sdk.Account
	for i, publicKey := range publicKeys {
		code := append(publicKey, common.STANDARD)
		programHash := common.ToProgramHash(byte(contract.PrefixStandard), code)
		account = wallet.Keystore.GetAccountByProgramHash(programHash)
		if account != nil {
			signerIndex = i
			break
		}
	}
	if signerIndex == -1 {
		return nil, errors.New("[Wallet], Invalid multi sign signer")
	}
	// Sign transaction
	buf := new(bytes.Buffer)
	tx.SerializeUnsigned(buf)
	signedTx, err := account.Sign(buf.Bytes())
	if err != nil {
		return nil, err
	}
	// Append signature
	tx.Programs[0].Parameter, err = crypto.AppendSignature(signerIndex, signedTx, buf.Bytes(), code, param)
	if err != nil {
		return nil, err
	}

	return tx, nil
}

func (wallet *Wallet) SendTransaction(tx *types.Transaction) error {
	buf := new(bytes.Buffer)
	if err := tx.Serialize(buf); err != nil {
		return err
	}
	rawData := hex.EncodeToString(buf.Bytes())

	_, err := jsonrpc.CallArray(jsonRpcUrl, "sendrawtransaction", rawData)
	return err
}

func (wallet *Wallet) removeLockedUTXOs(utxos []*sutil.UTXO) []*sutil.UTXO {
	var availableUTXOs []*sutil.UTXO
	var currentHeight = wallet.BestHeight()
	for _, utxo := range utxos {
		if utxo.AtHeight == 0 { // remove unconfirmed UTOXs
			continue
		}
		if utxo.LockTime > 0 {
			if utxo.LockTime > currentHeight {
				continue
			}
			utxo.LockTime = math.MaxUint32 - 1
		}
		availableUTXOs = append(availableUTXOs, utxo)
	}
	return availableUTXOs
}

func InputFromUTXO(utxo *sutil.UTXO) *types.Input {
	input := new(types.Input)
	input.Previous.TxID = utxo.Op.TxID
	input.Previous.Index = utxo.Op.Index
	input.Sequence = utxo.LockTime
	return input
}

func (wallet *Wallet) newTransaction(redeemScript []byte, inputs []*types.Input, outputs []*types.Output) *types.Transaction {
	// Create payload
	txPayload := &payload.TransferAsset{}
	// Create attributes
	txAttr := types.NewAttribute(types.Nonce, []byte(strconv.FormatInt(rand.Int63(), 10)))
	attributes := make([]*types.Attribute, 0)
	attributes = append(attributes, &txAttr)
	// Create transaction
	return &types.Transaction{
		TxType:     types.TransferAsset,
		Payload:    txPayload,
		Attributes: attributes,
		Inputs:     inputs,
		Outputs:    outputs,
		Programs:   []*program.Program{{redeemScript, nil}},
		LockTime:   wallet.BestHeight(),
	}
}
