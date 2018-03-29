package transaction

import (
	"bytes"
	"errors"
	"fmt"
	"io"

	. "SPVWallet/core"
	"SPVWallet/core/contract/program"
	"SPVWallet/core/serialization"
	"SPVWallet/core/transaction/payload"
	"SPVWallet/crypto"
)

//for different transaction types with different payload format
//and transaction process methods
type TransactionType byte

const (
	CoinBase                TransactionType = 0x00
	RegisterAsset           TransactionType = 0x01
	TransferAsset           TransactionType = 0x02
	Record                  TransactionType = 0x03
	Deploy                  TransactionType = 0x04
	IssueToken              TransactionType = 0x05
	TransferCrossChainAsset TransactionType = 0x06

	PUSH1 = 0x51

	STANDARD   = 0xAC
	MULTISIG   = 0xAE
	CROSSCHAIN = 0xAF
)

func (self TransactionType) Name() string {
	switch self {
	case CoinBase:
		return "CoinBase"
	case RegisterAsset:
		return "RegisterAsset"
	case TransferAsset:
		return "TransferAsset"
	case Record:
		return "Record"
	case Deploy:
		return "Deploy"
	case IssueToken:
		return "IssueToken"
	case TransferCrossChainAsset:
		return "TransferCrossChainAsset"
	default:
		return "Unknown"
	}
}

const (
	InvalidTransactionSize = -1

	// encoded public key length 0x21 || encoded public key (33 bytes) || OP_CHECKSIG(0xac)
	PublicKeyScriptLength = 35

	// signature length(0x40) || 64 bytes signature
	SignatureScriptLength = 65

	// 1byte m || 3 encoded public keys with leading 0x40 (34 bytes * 3) ||
	// 1byte n + 1byte OP_CHECKMULTISIG
	// FIXME: if want to support 1/2 multisig
	MinMultiSignCodeLength = 105
)

//Payload define the func for loading the payload data
//base on payload type which have different struture
type Payload interface {
	//  Get payload data
	Data(version byte) []byte

	//Serialize payload data
	Serialize(w io.Writer, version byte) error

	Deserialize(r io.Reader, version byte) error
}

type Transaction struct {
	TxType         TransactionType
	PayloadVersion byte
	Payload        Payload
	Attributes     []*Attribute
	Inputs         []*Input
	Outputs        []*Output
	LockTime       uint32
	Programs       []*program.Program
}

func (tx *Transaction) String() string {
	return "Transaction: {\n\t" +
		"Hash: " + tx.Hash().String() + "\n\t" +
		"TxType: " + tx.TxType.Name() + "\n\t" +
		"PayloadVersion: " + fmt.Sprint(tx.PayloadVersion) + "\n\t" +
		"Payload: " + BytesToHexString(tx.Payload.Data(tx.PayloadVersion)) + "\n\t" +
		"Attributes: " + fmt.Sprint(tx.Attributes) + "\n\t" +
		"Inputs: " + fmt.Sprint(tx.Inputs) + "\n\t" +
		"Outputs: " + fmt.Sprint(tx.Outputs) + "\n\t" +
		"LockTime: " + fmt.Sprint(tx.LockTime) + "\n\t" +
		"Programs: " + fmt.Sprint(tx.Programs) + "\n\t" +
		"}\n"
}

//Serialize the Transaction
func (tx *Transaction) Serialize(w io.Writer) error {

	err := tx.SerializeUnsigned(w)
	if err != nil {
		return errors.New("Transaction txSerializeUnsigned Serialize failed.")
	}
	//Serialize  Transaction's programs
	lens := uint64(len(tx.Programs))
	err = serialization.WriteVarUint(w, lens)
	if err != nil {
		return errors.New("Transaction WriteVarUint failed.")
	}
	if lens > 0 {
		for _, p := range tx.Programs {
			err = p.Serialize(w)
			if err != nil {
				return errors.New("Transaction Programs Serialize failed.")
			}
		}
	}
	return nil
}

//Serialize the Transaction data without contracts
func (tx *Transaction) SerializeUnsigned(w io.Writer) error {
	//txType
	w.Write([]byte{byte(tx.TxType)})
	//PayloadVersion
	w.Write([]byte{tx.PayloadVersion})
	//Payload
	if tx.Payload == nil {
		return errors.New("Transaction Payload is nil.")
	}
	tx.Payload.Serialize(w, tx.PayloadVersion)
	//[]*txAttribute
	err := serialization.WriteVarUint(w, uint64(len(tx.Attributes)))
	if err != nil {
		return errors.New("Transaction item txAttribute length serialization failed.")
	}
	if len(tx.Attributes) > 0 {
		for _, attr := range tx.Attributes {
			attr.Serialize(w)
		}
	}
	//[]*Inputs
	err = serialization.WriteVarUint(w, uint64(len(tx.Inputs)))
	if err != nil {
		return errors.New("Transaction item Inputs length serialization failed.")
	}
	if len(tx.Inputs) > 0 {
		for _, utxo := range tx.Inputs {
			utxo.Serialize(w)
		}
	}
	// TODO BalanceInputs
	//[]*Outputs
	err = serialization.WriteVarUint(w, uint64(len(tx.Outputs)))
	if err != nil {
		return errors.New("Transaction item Outputs length serialization failed.")
	}
	if len(tx.Outputs) > 0 {
		for _, output := range tx.Outputs {
			output.Serialize(w)
		}
	}

	serialization.WriteUint32(w, tx.LockTime)

	return nil
}

//deserialize the Transaction
func (tx *Transaction) Deserialize(r io.Reader) error {
	// tx deserialize
	err := tx.DeserializeUnsigned(r)
	if err != nil {
		return errors.New("transaction Deserialize error")
	}

	// tx program
	lens, err := serialization.ReadVarUint(r, 0)
	if err != nil {
		return errors.New("transaction tx program Deserialize error")
	}

	programHashes := []*program.Program{}
	if lens > 0 {
		for i := 0; i < int(lens); i++ {
			outputHashes := new(program.Program)
			outputHashes.Deserialize(r)
			programHashes = append(programHashes, outputHashes)
		}
		tx.Programs = programHashes
	}
	return nil
}

func (tx *Transaction) DeserializeUnsigned(r io.Reader) error {
	var txType [1]byte
	_, err := io.ReadFull(r, txType[:])
	if err != nil {
		return err
	}
	tx.TxType = TransactionType(txType[0])
	return tx.DeserializeUnsignedWithoutType(r)
}

func (tx *Transaction) DeserializeUnsignedWithoutType(r io.Reader) error {
	var payloadVersion [1]byte
	_, err := io.ReadFull(r, payloadVersion[:])
	tx.PayloadVersion = payloadVersion[0]
	if err != nil {
		return err
	}

	switch tx.TxType {
	case CoinBase:
		tx.Payload = new(payload.CoinBase)
	case RegisterAsset:
		tx.Payload = new(payload.RegisterAsset)
	case TransferAsset:
		tx.Payload = new(payload.TransferAsset)
	case Record:
		tx.Payload = new(payload.Record)
	case Deploy:
		tx.Payload = new(payload.DeployCode)
	case IssueToken:
		tx.Payload = new(payload.IssueToken)
	case TransferCrossChainAsset:
		tx.Payload = new(payload.TransferCrossChainAsset)
	default:
		return errors.New("[Transaction], invalid transaction type.")
	}
	err = tx.Payload.Deserialize(r, tx.PayloadVersion)
	if err != nil {
		return errors.New("Payload Parse error")
	}
	//attributes
	Len, err := serialization.ReadVarUint(r, 0)
	if err != nil {
		return err
	}
	if Len > uint64(0) {
		for i := uint64(0); i < Len; i++ {
			attr := new(Attribute)
			err = attr.Deserialize(r)
			if err != nil {
				return err
			}
			tx.Attributes = append(tx.Attributes, attr)
		}
	}
	//Inputs
	Len, err = serialization.ReadVarUint(r, 0)
	if err != nil {
		return err
	}
	if Len > uint64(0) {
		for i := uint64(0); i < Len; i++ {
			utxo := new(Input)
			err = utxo.Deserialize(r)
			if err != nil {
				return err
			}
			tx.Inputs = append(tx.Inputs, utxo)
		}
	}
	//TODO balanceInputs
	//Outputs
	Len, err = serialization.ReadVarUint(r, 0)
	if err != nil {
		return err
	}
	if Len > uint64(0) {
		for i := uint64(0); i < Len; i++ {
			output := new(Output)
			output.Deserialize(r)

			tx.Outputs = append(tx.Outputs, output)
		}
	}

	temp, err := serialization.ReadUint32(r)
	tx.LockTime = uint32(temp)
	if err != nil {
		return err
	}

	return nil
}

func (tx *Transaction) GetSize() int {
	var buffer bytes.Buffer
	if err := tx.Serialize(&buffer); err != nil {
		return InvalidTransactionSize
	}

	return buffer.Len()
}

func (tx *Transaction) SetPrograms(programs []*program.Program) {
	tx.Programs = programs
}

func (tx *Transaction) GetPrograms() []*program.Program {
	return tx.Programs
}

func (tx *Transaction) Hash() *Uint256 {
	var hash Uint256
	buf := new(bytes.Buffer)
	tx.SerializeUnsigned(buf)
	hash = Sha256D(buf.Bytes())
	return &hash
}

func (tx *Transaction) IsCoinBaseTx() bool {
	return tx.TxType == CoinBase
}

func (tx *Transaction) GetTransactionCode() ([]byte, error) {
	code := tx.GetPrograms()[0].Code
	if code == nil {
		return nil, errors.New("invalid transaction type, redeem script not found")
	}
	return code, nil
}

func (tx *Transaction) GetStandardSigner() (*Uint168, error) {
	code, err := tx.GetTransactionCode()
	if err != nil {
		return nil, err
	}
	if len(code) != PublicKeyScriptLength || code[len(code)-1] != STANDARD {
		return nil, errors.New("not a valid standard transaction code, length not match")
	}
	// remove last byte STANDARD
	code = code[:len(code)-1]
	script := make([]byte, PublicKeyScriptLength)
	copy(script, code[:PublicKeyScriptLength])

	return ToProgramHash(script)
}

func (tx *Transaction) GetMultiSignSigners() ([]*Uint168, error) {
	scripts, err := tx.GetMultiSignPublicKeys()
	if err != nil {
		return nil, err
	}

	var signers []*Uint168
	for _, script := range scripts {
		script = append(script, STANDARD)
		hash, _ := ToProgramHash(script)
		signers = append(signers, hash)
	}

	return signers, nil
}

func (tx *Transaction) GetMultiSignPublicKeys() ([][]byte, error) {
	code, err := tx.GetTransactionCode()
	if err != nil {
		return nil, err
	}
	if len(code) < MinMultiSignCodeLength || code[len(code)-1] != MULTISIG {
		return nil, errors.New("not a valid multi sign transaction code, length not enough")
	}
	// remove last byte MULTISIG
	code = code[:len(code)-1]
	// remove m
	code = code[1:]
	// remove n
	code = code[:len(code)-1]
	if len(code)%(PublicKeyScriptLength-1) != 0 {
		return nil, errors.New("not a valid multi sign transaction code, length not match")
	}

	var publicKeys [][]byte
	i := 0
	for i < len(code) {
		script := make([]byte, PublicKeyScriptLength-1)
		copy(script, code[i:i+PublicKeyScriptLength-1])
		i += PublicKeyScriptLength - 1
		publicKeys = append(publicKeys, script)
	}
	return publicKeys, nil
}

func (tx *Transaction) GetTransactionType() (byte, error) {
	code, err := tx.GetTransactionCode()
	if err != nil {
		return 0, err
	}
	if len(code) != PublicKeyScriptLength && len(code) < MinMultiSignCodeLength {
		return 0, errors.New("invalid transaction type, redeem script not a standard or multi sign type")
	}
	return code[len(code)-1], nil
}

func (tx *Transaction) getM() int {
	return int(tx.Programs[0].Code[0] - PUSH1 + 1)
}

func (tx *Transaction) GetSignStatus() (haveSign, needSign int, err error) {
	if len(tx.Programs) <= 0 {
		return -1, -1, errors.New("missing transaction program")
	}

	signType, err := tx.GetTransactionType()
	if err != nil {
		return -1, -1, err
	}

	if signType == STANDARD {
		signed := len(tx.Programs[0].Parameter) / SignatureScriptLength
		return signed, 1, nil

	} else if signType == MULTISIG {

		haveSign = len(tx.Programs[0].Parameter) / SignatureScriptLength

		return haveSign, tx.getM(), nil
	}

	return -1, -1, errors.New("invalid transaction type")
}

func (tx *Transaction) AppendSignature(signerIndex int, signature []byte) error {
	if len(tx.Programs) <= 0 {
		return errors.New("missing transaction program")
	}
	// Create new signature
	newSign := append([]byte{}, byte(len(signature)))
	newSign = append(newSign, signature...)

	param := tx.Programs[0].Parameter

	// Check if is first signature
	if param == nil {
		param = []byte{}
	} else {
		// Check if singer already signed
		publicKeys, err := tx.GetMultiSignPublicKeys()
		if err != nil {
			return err
		}
		buf := new(bytes.Buffer)
		tx.SerializeUnsigned(buf)
		for i := 0; i < len(param); i += SignatureScriptLength {
			// Remove length byte
			sign := param[i: i+SignatureScriptLength][1:]
			publicKey := publicKeys[signerIndex][1:]
			pubKey, err := crypto.DecodePoint(publicKey)
			if err != nil {
				return err
			}
			err = crypto.Verify(*pubKey, buf.Bytes(), sign)
			if err == nil {
				return errors.New("signer already signed")
			}
		}
	}

	buf := new(bytes.Buffer)
	buf.Write(param)
	buf.Write(newSign)

	tx.Programs[0].Parameter = buf.Bytes()

	return nil
}
