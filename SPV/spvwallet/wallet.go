package spvwallet

import (
	"math"
	"bytes"
	"errors"
	"strconv"
	"math/rand"

	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/crypto"
	. "github.com/elastos/Elastos.ELA.Utility/core"
	. "github.com/elastos/Elastos.ELA.SPV/spvwallet/db"
	"github.com/elastos/Elastos.ELA.SPV/log"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/rpc"
	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA.Client/core/transaction/payload"
)

var SystemAssetId = getSystemAssetId()

type Transfer struct {
	Address string
	Value   *Fixed64
}

var wallet Wallet // Single instance of wallet

type Wallet interface {
	Database

	VerifyPassword(password []byte) error
	ChangePassword(oldPassword, newPassword []byte) error

	NewSubAccount(password []byte) (*Uint168, error)
	AddMultiSignAccount(M uint, publicKey ...*crypto.PublicKey) (*Uint168, error)

	CreateTransaction(fromAddress, toAddress string, amount, fee *Fixed64) (*Transaction, error)
	CreateLockedTransaction(fromAddress, toAddress string, amount, fee *Fixed64, lockedUntil uint32) (*Transaction, error)
	CreateMultiOutputTransaction(fromAddress string, fee *Fixed64, output ...*Transfer) (*Transaction, error)
	CreateLockedMultiOutputTransaction(fromAddress string, fee *Fixed64, lockedUntil uint32, output ...*Transfer) (*Transaction, error)
	Sign(password []byte, transaction *Transaction) (*Transaction, error)
	SendTransaction(txn *Transaction) error
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

func (wallet *WalletImpl) NewSubAccount(password []byte) (*Uint168, error) {
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

func (wallet *WalletImpl) AddMultiSignAccount(M uint, publicKeys ...*crypto.PublicKey) (*Uint168, error) {
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

func (wallet *WalletImpl) CreateTransaction(fromAddress, toAddress string, amount, fee *Fixed64) (*Transaction, error) {
	return wallet.CreateLockedTransaction(fromAddress, toAddress, amount, fee, uint32(0))
}

func (wallet *WalletImpl) CreateLockedTransaction(fromAddress, toAddress string, amount, fee *Fixed64, lockedUntil uint32) (*Transaction, error) {
	return wallet.CreateLockedMultiOutputTransaction(fromAddress, fee, lockedUntil, &Transfer{toAddress, amount})
}

func (wallet *WalletImpl) CreateMultiOutputTransaction(fromAddress string, fee *Fixed64, outputs ...*Transfer) (*Transaction, error) {
	return wallet.CreateLockedMultiOutputTransaction(fromAddress, fee, uint32(0), outputs...)
}

func (wallet *WalletImpl) CreateLockedMultiOutputTransaction(fromAddress string, fee *Fixed64, lockedUntil uint32, outputs ...*Transfer) (*Transaction, error) {
	return wallet.createTransaction(fromAddress, fee, lockedUntil, outputs...)
}

func (wallet *WalletImpl) createTransaction(fromAddress string, fee *Fixed64, lockedUntil uint32, outputs ...*Transfer) (*Transaction, error) {
	// Check if output is valid
	if outputs == nil || len(outputs) == 0 {
		return nil, errors.New("[Wallet], Invalid transaction target")
	}

	// Check if from address is valid
	spender, err := Uint168FromAddress(fromAddress)
	if err != nil {
		return nil, errors.New("[Wallet], Invalid spender address")
	}
	// Create transaction outputs
	var totalOutputValue = Fixed64(0) // The total value will be spend
	var txOutputs []*Output           // The outputs in transaction
	totalOutputValue += *fee          // Add transaction fee

	for _, output := range outputs {
		receiver, err := Uint168FromAddress(output.Address)
		if err != nil {
			return nil, errors.New("[Wallet], Invalid receiver address")
		}
		txOutput := &Output{
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
	var txInputs []*Input // The inputs in transaction
	for _, utxo := range availableUTXOs {
		txInputs = append(txInputs, InputFromUTXO(utxo))
		if utxo.Value < totalOutputValue {
			totalOutputValue -= utxo.Value
		} else if utxo.Value == totalOutputValue {
			totalOutputValue = 0
			break
		} else if utxo.Value > totalOutputValue {
			change := &Output{
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

func (wallet *WalletImpl) Sign(password []byte, txn *Transaction) (*Transaction, error) {
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
	if signType == crypto.STANDARD {

		// Sign single transaction
		txn, err = wallet.signStandardTransaction(txn)
		if err != nil {
			return nil, err
		}

	} else if signType == crypto.MULTISIG {

		// Sign multi sign transaction
		txn, err = wallet.signMultiSigTransaction(txn)
		if err != nil {
			return nil, err
		}
	}

	return txn, nil
}

func (wallet *WalletImpl) signStandardTransaction(txn *Transaction) (*Transaction, error) {
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
	signedTx, err := account.Sign(buf.Bytes())
	if err != nil {
		return nil, err
	}
	// Add signature
	buf = new(bytes.Buffer)
	buf.WriteByte(byte(len(signedTx)))
	buf.Write(signedTx)
	// Set program
	var program = &Program{code, buf.Bytes()}
	txn.Programs = []*Program{program}

	return txn, nil
}

func (wallet *WalletImpl) signMultiSigTransaction(txn *Transaction) (*Transaction, error) {
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

func (wallet *WalletImpl) SendTransaction(txn *Transaction) error {

	// Send transaction through P2P network
	rpc.GetClient().SendTransaction(txn)

	return nil
}

func getSystemAssetId() Uint256 {
	systemToken := &Transaction{
		TxType:         RegisterAsset,
		PayloadVersion: 0,
		Payload: &PayloadRegisterAsset{
			Asset: Asset{
				Name:      "ELA",
				Precision: 0x08,
				AssetType: 0x00,
			},
			Amount:     0 * 100000000,
			Controller: Uint168{},
		},
		Attributes: []*Attribute{},
		Inputs:     []*Input{},
		Outputs:    []*Output{},
		Programs:   []*Program{},
	}
	return systemToken.Hash()
}

func (wallet *WalletImpl) removeLockedUTXOs(utxos []*UTXO) []*UTXO {
	var availableUTXOs []*UTXO
	var currentHeight = wallet.ChainHeight()
	for _, utxo := range utxos {
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

func InputFromUTXO(utxo *UTXO) *Input {
	input := new(Input)
	input.Previous.TxID = utxo.Op.TxID
	input.Previous.Index = utxo.Op.Index
	input.Sequence = utxo.LockTime
	return input
}

func (wallet *WalletImpl) newTransaction(redeemScript []byte, inputs []*Input, outputs []*Output) *Transaction {
	// Create payload
	txPayload := &payload.TransferAsset{}
	// Create attributes
	txAttr := NewAttribute(Nonce, []byte(strconv.FormatInt(rand.Int63(), 10)))
	attributes := make([]*Attribute, 0)
	attributes = append(attributes, &txAttr)
	// Create program
	var program = &Program{redeemScript, nil}
	// Create transaction
	return &Transaction{
		TxType:     TransferAsset,
		Payload:    txPayload,
		Attributes: attributes,
		Inputs:     inputs,
		Outputs:    outputs,
		Programs:   []*Program{program},
		LockTime:   wallet.ChainHeight(),
	}
}
