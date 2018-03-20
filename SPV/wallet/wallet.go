package wallet

import (
	"math"
	"bytes"
	"errors"
	"strconv"
	"math/rand"

	"SPVWallet/core/asset"
	. "SPVWallet/core"
	"SPVWallet/crypto"
	pg "SPVWallet/core/contract/program"
	tx "SPVWallet/core/transaction"
	"SPVWallet/core/transaction/payload"
	. "SPVWallet/db"
	"SPVWallet/log"
)

var SystemAssetId = *getSystemAssetId()

type Output struct {
	Address string
	Value   *Fixed64
}

var wallet Wallet // Single instance of wallet

type Wallet interface {
	Database

	VerifyPassword(password []byte) error
	ChangePassword(oldPassword, newPassword []byte) error

	NewSubAccount(password []byte) (*Uint168, error)
	AddMultiSignAccount(M int, publicKey ...*crypto.PublicKey) (*Uint168, error)

	CreateTransaction(fromAddress, toAddress string, amount, fee *Fixed64) (*tx.Transaction, error)
	CreateLockedTransaction(fromAddress, toAddress string, amount, fee *Fixed64, lockedUntil uint32) (*tx.Transaction, error)
	CreateMultiOutputTransaction(fromAddress string, fee *Fixed64, output ...*Output) (*tx.Transaction, error)
	CreateLockedMultiOutputTransaction(fromAddress string, fee *Fixed64, lockedUntil uint32, output ...*Output) (*tx.Transaction, error)
	Sign(password []byte, transaction *tx.Transaction) (*tx.Transaction, error)
	SendTransaction(txn *tx.Transaction) error
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
	database.AddAddress(mainAccount.ProgramHash(), mainAccount.RedeemScript())

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
	err = wallet.AddAddress(account.ProgramHash(), account.RedeemScript())
	if err != nil {
		return nil, err
	}

	return account.ProgramHash(), nil
}

func (wallet *WalletImpl) AddMultiSignAccount(M int, publicKeys ...*crypto.PublicKey) (*Uint168, error) {
	redeemScript, err := tx.CreateMultiSignRedeemScript(M, publicKeys)
	if err != nil {
		return nil, errors.New("[Wallet], CreateStandardRedeemScript failed")
	}

	programHash, err := tx.ToProgramHash(redeemScript)
	if err != nil {
		return nil, errors.New("[Wallet], CreateMultiSignAddress failed")
	}

	err = wallet.AddAddress(programHash, redeemScript)
	if err != nil {
		return nil, err
	}

	return programHash, nil
}

func (wallet *WalletImpl) CreateTransaction(fromAddress, toAddress string, amount, fee *Fixed64) (*tx.Transaction, error) {
	return wallet.CreateLockedTransaction(fromAddress, toAddress, amount, fee, uint32(0))
}

func (wallet *WalletImpl) CreateLockedTransaction(fromAddress, toAddress string, amount, fee *Fixed64, lockedUntil uint32) (*tx.Transaction, error) {
	return wallet.CreateLockedMultiOutputTransaction(fromAddress, fee, lockedUntil, &Output{toAddress, amount})
}

func (wallet *WalletImpl) CreateMultiOutputTransaction(fromAddress string, fee *Fixed64, outputs ...*Output) (*tx.Transaction, error) {
	return wallet.CreateLockedMultiOutputTransaction(fromAddress, fee, uint32(0), outputs...)
}

func (wallet *WalletImpl) CreateLockedMultiOutputTransaction(fromAddress string, fee *Fixed64, lockedUntil uint32, outputs ...*Output) (*tx.Transaction, error) {
	return wallet.createTransaction(fromAddress, fee, lockedUntil, outputs...)
}

func (wallet *WalletImpl) createTransaction(fromAddress string, fee *Fixed64, lockedUntil uint32, outputs ...*Output) (*tx.Transaction, error) {
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
	var txOutputs []*tx.Output        // The outputs in transaction
	totalOutputValue += *fee          // Add transaction fee

	for _, output := range outputs {
		receiver, err := Uint168FromAddress(output.Address)
		if err != nil {
			return nil, errors.New("[Wallet], Invalid receiver address")
		}
		txOutput := &tx.Output{
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
	var txInputs []*tx.Input // The inputs in transaction
	for _, utxo := range availableUTXOs {
		txInputs = append(txInputs, InputFromStoreUTXO(utxo))
		if utxo.Value < totalOutputValue {
			totalOutputValue -= utxo.Value
		} else if utxo.Value == totalOutputValue {
			totalOutputValue = 0
			break
		} else if utxo.Value > totalOutputValue {
			change := &tx.Output{
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

	script, err := wallet.GetScript(spender)
	if err != nil {
		return nil, errors.New("[Wallet], Get spenders redeem script failed")
	}

	return wallet.newTransaction(script, txInputs, txOutputs), nil
}

func (wallet *WalletImpl) Sign(password []byte, txn *tx.Transaction) (*tx.Transaction, error) {
	// Verify password
	err := wallet.VerifyPassword(password)
	if err != nil {
		return nil, err
	}
	// Get sign type
	signType, err := txn.GetTransactionType()
	if err != nil {
		return nil, err
	}
	// Look up transaction type
	if signType == tx.STANDARD {

		// Sign single transaction
		txn, err = wallet.signStandardTransaction(txn)
		if err != nil {
			return nil, err
		}

	} else if signType == tx.MULTISIG {

		// Sign multi sign transaction
		txn, err = wallet.signMultiSigTransaction(txn)
		if err != nil {
			return nil, err
		}
	}

	return txn, nil
}

func (wallet *WalletImpl) signStandardTransaction(txn *tx.Transaction) (*tx.Transaction, error) {
	// Get signer
	programHash, err := txn.GetStandardSigner()
	// Check if current user is a valid signer
	account := wallet.Keystore.GetAccountByProgramHash(programHash)
	if account == nil {
		return nil, errors.New("[Wallet], Invalid signer")
	}
	// Sign transaction
	signedTx, err := account.Sign(txn)
	if err != nil {
		return nil, err
	}
	// Add verify program for transaction
	buf := new(bytes.Buffer)
	buf.WriteByte(byte(len(signedTx)))
	buf.Write(signedTx)
	// Add signature
	code, _ := txn.GetTransactionCode()
	var program = &pg.Program{code, buf.Bytes()}
	txn.SetPrograms([]*pg.Program{program})

	return txn, nil
}

func (wallet *WalletImpl) signMultiSigTransaction(txn *tx.Transaction) (*tx.Transaction, error) {
	// Check if current user is a valid signer
	var signerIndex = -1
	programHashes, err := txn.GetMultiSignSigners()
	if err != nil {
		return nil, err
	}
	var account *Account
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
	signedTx, err := account.Sign(txn)
	if err != nil {
		return nil, err
	}
	// Append signature
	err = txn.AppendSignature(signerIndex, signedTx)
	if err != nil {
		return nil, err
	}

	return txn, nil
}

func (wallet *WalletImpl) SendTransaction(txn *tx.Transaction) error {

	// Send transaction through P2P network
	spv.SendTransaction(txn)

	return nil
}

func getSystemAssetId() *Uint256 {
	systemToken := &tx.Transaction{
		TxType:         tx.RegisterAsset,
		PayloadVersion: 0,
		Payload: &payload.RegisterAsset{
			Asset: &asset.Asset{
				Name:      "ELA",
				Precision: 0x08,
				AssetType: 0x00,
			},
			Amount:     0 * 100000000,
			Controller: Uint168{},
		},
		Attributes: []*tx.Attribute{},
		Inputs:     []*tx.Input{},
		Outputs:    []*tx.Output{},
		Programs:   []*pg.Program{},
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

func (wallet *WalletImpl) newTransaction(redeemScript []byte, inputs []*tx.Input, outputs []*tx.Output) *tx.Transaction {
	// Create payload
	txPayload := &payload.TransferAsset{}
	// Create attributes
	txAttr := tx.NewAttribute(tx.Nonce, []byte(strconv.FormatInt(rand.Int63(), 10)))
	attributes := make([]*tx.Attribute, 0)
	attributes = append(attributes, &txAttr)
	// Create program
	var program = &pg.Program{redeemScript, nil}
	// Create transaction
	return &tx.Transaction{
		TxType:     tx.TransferAsset,
		Payload:    txPayload,
		Attributes: attributes,
		Inputs:     inputs,
		Outputs:    outputs,
		Programs:   []*pg.Program{program},
		LockTime:   wallet.ChainHeight(),
	}
}
