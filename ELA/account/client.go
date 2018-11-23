package account

import (
	"bytes"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"math/rand"
	"strconv"

	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/core"
	pg "github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/outputpayload"
	"github.com/elastos/Elastos.ELA/log"
	"github.com/elastos/Elastos.ELA/servers"
	"github.com/elastos/Elastos.ELA/vm"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/crypto"
	"github.com/elastos/Elastos.ELA.Utility/http/jsonrpc"
	"github.com/elastos/Elastos.ELA.Utility/http/util"
)

const (
	DESTROY_ADDRESS = "0000000000000000000000000000000000"
)

var IDReverse, _ = hex.DecodeString("a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0")
var SystemAssetID, _ = common.Uint256FromBytes(common.BytesReverse(IDReverse))

type Transfer struct {
	Address string
	Amount  *common.Fixed64
}

type CrossChainOutput struct {
	Address           string
	Amount            *common.Fixed64
	CrossChainAddress string
}

var wallet Wallet // Single instance of wallet

type Wallet interface {
	Open(name string, password []byte) error
	//ChangePassword(oldPassword, newPassword []byte) error

	AddStandardAccount(publicKey *crypto.PublicKey) (*common.Uint168, error)
	AddMultiSignAccount(M uint, publicKey ...*crypto.PublicKey) (*common.Uint168, error)

	CreateTransaction(fromAddress, toAddress string, amount, fee *common.Fixed64) (*core.Transaction, error)
	CreateLockedTransaction(fromAddress, toAddress string, amount, fee *common.Fixed64, lockedUntil uint32) (*core.Transaction, error)
	CreateMultiOutputTransaction(fromAddress string, fee *common.Fixed64, output ...*Transfer) (*core.Transaction, error)
	CreateLockedMultiOutputTransaction(fromAddress string, fee *common.Fixed64, lockedUntil uint32, output ...*Transfer) (*core.Transaction, error)
	//CreateCrossChainTransaction(fromAddress, toAddress, crossChainAddress string, amount, fee *common.Fixed64) (*core.Transaction, error)

	Sign(name string, password []byte, transaction *core.Transaction) (*core.Transaction, error)

	Address() string

	//Reset() error
}

type WalletImpl struct {
	Keystore
}

func Create(name string, password []byte) (*WalletImpl, error) {
	keyStore, err := CreateKeystore(name, password)
	if err != nil {
		log.Error("Wallet create key store failed:", err)
		return nil, err
	}

	return &WalletImpl{
		Keystore: keyStore,
	}, nil
}

func (wallet *WalletImpl) Open(name string, password []byte) error {
	keyStore, err := OpenKeystore(name, password)
	if err != nil {
		return err
	}
	wallet.Keystore = keyStore
	return nil
}

func (wallet *WalletImpl) AddStandardAccount(publicKey *crypto.PublicKey) (*common.Uint168, error) {
	redeemScript, err := crypto.CreateStandardRedeemScript(publicKey)
	if err != nil {
		return nil, errors.New("[Wallet], CreateStandardRedeemScript failed")
	}

	programHash, err := crypto.ToProgramHash(redeemScript)
	if err != nil {
		return nil, errors.New("[Wallet], CreateStandardAddress failed")
	}

	//err = wallet.AddAddress(programHash, redeemScript, TypeStand)
	//if err != nil {
	//	return nil, err
	//}

	return programHash, nil
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

	//err = wallet.AddAddress(programHash, redeemScript, TypeMulti)
	//if err != nil {
	//	return nil, err
	//}

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
	if len(outputs) == 0 {
		return nil, errors.New("[Wallet], Invalid transaction target")
	}

	// Check if from address is valid
	spender, err := common.Uint168FromAddress(fromAddress)
	if err != nil {
		return nil, errors.New(fmt.Sprint("[Wallet], Invalid spender address: ", fromAddress, ", error: ", err))
	}
	// Create transaction outputs
	var totalOutputAmount = common.Fixed64(0) // The total amount will be spend
	var txOutputs []*core.Output              // The outputs in transaction
	totalOutputAmount += *fee                 // Add transaction fee

	for _, output := range outputs {
		receiver, err := common.Uint168FromAddress(output.Address)
		if err != nil {
			return nil, errors.New(fmt.Sprint("[Wallet], Invalid receiver address: ", output.Address, ", error: ", err))
		}

		txOutput := &core.Output{
			AssetID:       *SystemAssetID,
			ProgramHash:   *receiver,
			Value:         *output.Amount,
			OutputLock:    lockedUntil,
			OutputType:    core.DefaultOutput,
			OutputPayload: &outputpayload.DefaultOutput{},
		}
		totalOutputAmount += *output.Amount
		txOutputs = append(txOutputs, txOutput)
	}

	result, err := jsonrpc.CallParams(ElaServer(), "listunspent", util.Params{
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
		if core.TransactionType(utxo.TxType) == core.CoinBase && utxo.Confirmations < 100 {
			continue
		}
		availabelUtxos = append(availabelUtxos, utxo)
	}

	// Create transaction inputs
	var txInputs []*core.Input // The inputs in transaction
	for _, utxo := range availabelUtxos {
		txIDReverse, _ := hex.DecodeString(utxo.TxID)
		txID, _ := common.Uint256FromBytes(common.BytesReverse(txIDReverse))
		input := &core.Input{
			Previous: core.OutPoint{
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
			change := &core.Output{
				AssetID:       *SystemAssetID,
				Value:         *amount - totalOutputAmount,
				OutputLock:    uint32(0),
				ProgramHash:   *spender,
				OutputType:    core.DefaultOutput,
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

	keystoreFile, err := OpenKeystoreFile(DefaultKeystoreFile)
	if err != nil {
		return nil, err
	}

	redeemScript, err := keystoreFile.GetRedeemScript()
	if err != nil {
		return nil, err
	}

	return wallet.newTransaction(redeemScript, txInputs, txOutputs, core.TransferAsset), nil
}

func (wallet *WalletImpl) Sign(name string, password []byte, txn *core.Transaction) (*core.Transaction, error) {
	// Verify password
	//err := wallet.Open(name, password)
	//if err != nil {
	//	return nil, err
	//}
	// Get sign type
	signType, err := crypto.GetScriptType(txn.Programs[0].Code)
	if err != nil {
		return nil, err
	}
	// Look up transaction type
	if signType == vm.CHECKSIG {

		// Sign single transaction
		txn, err = wallet.signStandardTransaction(txn)
		if err != nil {
			return nil, err
		}

	} else if signType == vm.CHECKMULTISIG {

		// Sign multi sign transaction
		txn, err = wallet.signMultiSignTransaction(txn)
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
	if *programHash != *wallet.Keystore.GetProgramHash() {
		return nil, errors.New("[Wallet], Invalid signer")
	}
	// Sign transaction
	signedTx, err := wallet.Keystore.Sign(txn)
	if err != nil {
		return nil, err
	}
	// Add verify program for transaction
	buf := new(bytes.Buffer)
	buf.WriteByte(byte(len(signedTx)))
	buf.Write(signedTx)
	// Add signature
	txn.Programs[0].Parameter = buf.Bytes()

	return txn, nil
}

func (wallet *WalletImpl) signMultiSignTransaction(txn *core.Transaction) (*core.Transaction, error) {
	code := txn.Programs[0].Code
	param := txn.Programs[0].Parameter
	// Check if current user is a valid signer
	var signerIndex = -1
	programHashes, err := crypto.GetSigners(code)
	if err != nil {
		return nil, err
	}
	userProgramHash := wallet.Keystore.GetProgramHash()
	for i, programHash := range programHashes {
		if *userProgramHash == *programHash {
			signerIndex = i
			break
		}
	}
	if signerIndex == -1 {
		return nil, errors.New("[Wallet], Invalid multi sign signer")
	}
	// Sign transaction
	signature, err := wallet.Keystore.Sign(txn)
	if err != nil {
		return nil, err
	}
	// Append signature
	buf := new(bytes.Buffer)
	txn.SerializeUnsigned(buf)
	txn.Programs[0].Parameter, err = crypto.AppendSignature(signerIndex, signature, buf.Bytes(), code, param)
	if err != nil {
		return nil, err
	}

	return txn, nil
}

func (wallet *WalletImpl) newTransaction(redeemScript []byte, inputs []*core.Input, outputs []*core.Output, txType core.TransactionType) *core.Transaction {
	// Create payload
	txPayload := &core.PayloadTransferAsset{}
	// Create attributes
	txAttr := core.NewAttribute(core.Nonce, []byte(strconv.FormatInt(rand.Int63(), 10)))
	attributes := make([]*core.Attribute, 0)
	attributes = append(attributes, &txAttr)
	// Create program
	var program = &pg.Program{redeemScript, nil}
	// Create transaction
	return &core.Transaction{
		Version:    core.TxVersionC0,
		TxType:     txType,
		Payload:    txPayload,
		Attributes: attributes,
		Inputs:     inputs,
		Outputs:    outputs,
		Programs:   []*pg.Program{program},
		LockTime:   0,
	}
}

func ElaServer() string {
	return "http://localhost" + ":" + strconv.Itoa(config.Parameters.HttpJsonPort)
}

func GetKeystore() {

}
