package spvwallet

import (
	"bytes"
	"errors"
	"math"
	"math/rand"
	"strconv"

	"github.com/elastos/Elastos.ELA.SPV/log"
	"github.com/elastos/Elastos.ELA.SPV/sdk"
	. "github.com/elastos/Elastos.ELA.SPV/spvwallet/db"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/rpc"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/crypto"
	"github.com/elastos/Elastos.ELA/core"
)

var SystemAssetId = getSystemAssetId()

type Transfer struct {
	Address string
	Value   *common.Fixed64
}

var wallet Wallet // Single instance of wallet

type Wallet interface {
	Database

	VerifyPassword(password []byte) error
	ChangePassword(oldPassword, newPassword []byte) error

	NewSubAccount(password []byte) (*common.Uint168, error)
	AddMultiSignAccount(M uint, publicKey ...*crypto.PublicKey) (*common.Uint168, error)

	CreateTransaction(fromAddress, toAddress string, amount, fee *common.Fixed64) (*core.Transaction, error)
	CreateLockedTransaction(fromAddress, toAddress string, amount, fee *common.Fixed64, lockedUntil uint32) (*core.Transaction, error)
	CreateMultiOutputTransaction(fromAddress string, fee *common.Fixed64, output ...*Transfer) (*core.Transaction, error)
	CreateLockedMultiOutputTransaction(fromAddress string, fee *common.Fixed64, lockedUntil uint32, output ...*Transfer) (*core.Transaction, error)
	Sign(password []byte, transaction *core.Transaction) (*core.Transaction, error)
	SendTransaction(txn *core.Transaction) error
}

type WalletImpl struct {
	Database
	Keystore
}

func Create(password []byte) (Wallet, error) {
	keyStore, err := CreateKeystore(password)
	if err != nil {
		log.Error("Wallet create keystore failed:", err)
		return nil, err
	}

	database, err := GetDatabase()
	if err != nil {
		log.Error("Wallet create database failed:", err)
		return nil, err
	}

	mainAccount := keyStore.GetAccountByIndex(0)
	database.AddAddress(mainAccount.ProgramHash(), mainAccount.RedeemScript(), TypeMaster)

	wallet = &WalletImpl{
		Database: database,
		Keystore: keyStore,
	}
	return wallet, nil
}

func Open() (Wallet, error) {
	if wallet == nil {
		database, err := GetDatabase()
		if err != nil {
			log.Error("Wallet open database failed:", err)
			return nil, err
		}

		wallet = &WalletImpl{
			Database: database,
		}
	}
	return wallet, nil
}

func (wallet *WalletImpl) VerifyPassword(password []byte) error {
	keyStore, err := OpenKeystore(password)
	if err != nil {
		return err
	}
	wallet.Keystore = keyStore
	return nil
}

func (wallet *WalletImpl) NewSubAccount(password []byte) (*common.Uint168, error) {
	err := wallet.VerifyPassword(password)
	if err != nil {
		return nil, err
	}

	account := wallet.Keystore.NewAccount()
	err = wallet.AddAddress(account.ProgramHash(), account.RedeemScript(), TypeSub)
	if err != nil {
		return nil, err
	}

	// Notify SPV service to reload bloom filter with the new address
	rpc.GetClient().NotifyNewAddress(account.ProgramHash().Bytes())

	return account.ProgramHash(), nil
}

func (wallet *WalletImpl) AddMultiSignAccount(M uint, publicKeys ...*crypto.PublicKey) (*common.Uint168, error) {
	redeemScript, err := crypto.CreateMultiSignRedeemScript(M, publicKeys)
	if err != nil {
		return nil, errors.New("[Wallet], CreateStandardRedeemScript failed")
	}

	programHash, err := crypto.ToProgramHash(redeemScript)
	if err != nil {
		return nil, errors.New("[Wallet], CreateMultiSignAddress failed")
	}

	err = wallet.AddAddress(programHash, redeemScript, TypeMulti)
	if err != nil {
		return nil, err
	}

	// Notify SPV service to reload bloom filter with the new address
	rpc.GetClient().NotifyNewAddress(programHash.Bytes())

	return programHash, nil
}

func (wallet *WalletImpl) CreateTransaction(fromAddress, toAddress string, amount, fee *common.Fixed64) (*core.Transaction, error) {
	return wallet.CreateLockedTransaction(fromAddress, toAddress, amount, fee, uint32(0))
}

func (wallet *WalletImpl) CreateLockedTransaction(fromAddress, toAddress string, amount, fee *common.Fixed64, lockedUntil uint32) (*core.Transaction, error) {
	return wallet.CreateLockedMultiOutputTransaction(fromAddress, fee, lockedUntil, &Transfer{toAddress, amount})
}

func (wallet *WalletImpl) CreateMultiOutputTransaction(fromAddress string, fee *common.Fixed64, outputs ...*Transfer) (*core.Transaction, error) {
	return wallet.CreateLockedMultiOutputTransaction(fromAddress, fee, uint32(0), outputs...)
}

func (wallet *WalletImpl) CreateLockedMultiOutputTransaction(fromAddress string, fee *common.Fixed64, lockedUntil uint32, outputs ...*Transfer) (*core.Transaction, error) {
	return wallet.createTransaction(fromAddress, fee, lockedUntil, outputs...)
}

func (wallet *WalletImpl) createTransaction(fromAddress string, fee *common.Fixed64, lockedUntil uint32, outputs ...*Transfer) (*core.Transaction, error) {
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
	var txOutputs []*core.Output             // The outputs in transaction
	totalOutputValue += *fee                 // Add transaction fee

	for _, output := range outputs {
		receiver, err := common.Uint168FromAddress(output.Address)
		if err != nil {
			return nil, errors.New("[Wallet], Invalid receiver address")
		}
		txOutput := &core.Output{
			AssetID:     SystemAssetId,
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
	availableUTXOs := wallet.removeLockedUTXOs(utxos) // Remove locked UTXOs
	availableUTXOs = SortUTXOs(availableUTXOs)        // Sort available UTXOs by value ASC

	// Create transaction inputs
	var txInputs []*core.Input // The inputs in transaction
	for _, utxo := range availableUTXOs {
		txInputs = append(txInputs, InputFromUTXO(utxo))
		if utxo.Value < totalOutputValue {
			totalOutputValue -= utxo.Value
		} else if utxo.Value == totalOutputValue {
			totalOutputValue = 0
			break
		} else if utxo.Value > totalOutputValue {
			change := &core.Output{
				AssetID:     SystemAssetId,
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

func (wallet *WalletImpl) Sign(password []byte, txn *core.Transaction) (*core.Transaction, error) {
	// Verify password
	err := wallet.VerifyPassword(password)
	if err != nil {
		return nil, err
	}
	// Get sign type
	signType, err := crypto.GetScriptType(txn.Programs[0].Code)
	if err != nil {
		return nil, err
	}
	// Look up transaction type
	if signType == common.STANDARD {

		// Sign single transaction
		txn, err = wallet.signStandardTransaction(txn)
		if err != nil {
			return nil, err
		}

	} else if signType == common.MULTISIG {

		// Sign multi sign transaction
		txn, err = wallet.signMultiSigTransaction(txn)
		if err != nil {
			return nil, err
		}
	}

	return txn, nil
}

func (wallet *WalletImpl) signStandardTransaction(txn *core.Transaction) (*core.Transaction, error) {
	code := txn.Programs[0].Code
	// Get signer
	programHash, err := crypto.GetSigner(code)
	// Check if current user is a valid signer
	account := wallet.Keystore.GetAccountByProgramHash(programHash)
	if account == nil {
		return nil, errors.New("[Wallet], Invalid signer")
	}
	// Sign transaction
	buf := new(bytes.Buffer)
	txn.SerializeUnsigned(buf)
	signature, err := account.Sign(buf.Bytes())
	if err != nil {
		return nil, err
	}
	// Add signature
	buf = new(bytes.Buffer)
	buf.WriteByte(byte(len(signature)))
	buf.Write(signature)
	// Set program
	var program = &core.Program{code, buf.Bytes()}
	txn.Programs = []*core.Program{program}

	return txn, nil
}

func (wallet *WalletImpl) signMultiSigTransaction(txn *core.Transaction) (*core.Transaction, error) {
	code := txn.Programs[0].Code
	param := txn.Programs[0].Parameter
	// Check if current user is a valid signer
	var signerIndex = -1
	programHashes, err := crypto.GetSigners(code)
	if err != nil {
		return nil, err
	}
	var account *sdk.Account
	for i, programHash := range programHashes {
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
	txn.SerializeUnsigned(buf)
	signedTx, err := account.Sign(buf.Bytes())
	if err != nil {
		return nil, err
	}
	// Append signature
	txn.Programs[0].Parameter, err = crypto.AppendSignature(signerIndex, signedTx, buf.Bytes(), code, param)
	if err != nil {
		return nil, err
	}

	return txn, nil
}

func (wallet *WalletImpl) SendTransaction(txn *core.Transaction) error {
	// Send transaction through P2P network
	return rpc.GetClient().SendTransaction(txn)
}

func getSystemAssetId() common.Uint256 {
	systemToken := &core.Transaction{
		TxType:         core.RegisterAsset,
		PayloadVersion: 0,
		Payload: &core.PayloadRegisterAsset{
			Asset: core.Asset{
				Name:      "ELA",
				Precision: 0x08,
				AssetType: 0x00,
			},
			Amount:     0 * 100000000,
			Controller: common.Uint168{},
		},
		Attributes: []*core.Attribute{},
		Inputs:     []*core.Input{},
		Outputs:    []*core.Output{},
		Programs:   []*core.Program{},
	}
	return systemToken.Hash()
}

func (wallet *WalletImpl) removeLockedUTXOs(utxos []*UTXO) []*UTXO {
	var availableUTXOs []*UTXO
	var currentHeight = wallet.ChainHeight()
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

func InputFromUTXO(utxo *UTXO) *core.Input {
	input := new(core.Input)
	input.Previous.TxID = utxo.Op.TxID
	input.Previous.Index = utxo.Op.Index
	input.Sequence = utxo.LockTime
	return input
}

func (wallet *WalletImpl) newTransaction(redeemScript []byte, inputs []*core.Input, outputs []*core.Output) *core.Transaction {
	// Create payload
	txPayload := &core.PayloadTransferAsset{}
	// Create attributes
	txAttr := core.NewAttribute(core.Nonce, []byte(strconv.FormatInt(rand.Int63(), 10)))
	attributes := make([]*core.Attribute, 0)
	attributes = append(attributes, &txAttr)
	// Create program
	var program = &core.Program{redeemScript, nil}
	// Create transaction
	return &core.Transaction{
		TxType:     core.TransferAsset,
		Payload:    txPayload,
		Attributes: attributes,
		Inputs:     inputs,
		Outputs:    outputs,
		Programs:   []*core.Program{program},
		LockTime:   wallet.ChainHeight(),
	}
}
