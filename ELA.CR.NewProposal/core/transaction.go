package core

import (
	"bytes"
	"errors"
	"fmt"
	"io"

	. "github.com/elastos/Elastos.ELA.Utility/common"
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
	SideMining              TransactionType = 0x05
	IssueToken              TransactionType = 0x06
	WithdrawAsset           TransactionType = 0x07
	TransferCrossChainAsset TransactionType = 0x08
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
	case SideMining:
		return "SideMining"
	case IssueToken:
		return "IssueToken"
	case WithdrawAsset:
		return "WithdrawAsset"
	case TransferCrossChainAsset:
		return "TransferCrossChainAsset"
	default:
		return "Unknown"
	}
}

const (
	InvalidTransactionSize = -1
)

type Transaction struct {
	TxType         TransactionType
	PayloadVersion byte
	Payload        Payload
	Attributes     []*Attribute
	Inputs         []*Input
	Outputs        []*Output
	LockTime       uint32
	Programs       []*Program
	Fee            Fixed64
	FeePerKB       Fixed64

	hash *Uint256
}

func (tx *Transaction) String() string {
	hash := tx.Hash()
	return fmt.Sprint("Transaction: {\n\t",
		"Hash: ", hash.String(), "\n\t",
		"TxType: ", tx.TxType.Name(), "\n\t",
		"PayloadVersion: ", tx.PayloadVersion, "\n\t",
		"Payload: ", BytesToHexString(tx.Payload.Data(tx.PayloadVersion)), "\n\t",
		"Attributes: ", tx.Attributes, "\n\t",
		"Inputs: ", tx.Inputs, "\n\t",
		"Outputs: ", tx.Outputs, "\n\t",
		"LockTime: ", tx.LockTime, "\n\t",
		"Programs: ", tx.Programs, "\n\t",
		"}\n")
}

func (tx *Transaction) CMD() string {
	return "tx"
}

//Serialize the Transaction
func (tx *Transaction) Serialize(w io.Writer) error {
	if err := tx.SerializeUnsigned(w); err != nil {
		return errors.New("Transaction txSerializeUnsigned Serialize failed, " + err.Error())
	}
	//Serialize  Transaction's programs
	if err := WriteVarUint(w, uint64(len(tx.Programs))); err != nil {
		return errors.New("Transaction program count failed.")
	}
	for _, p := range tx.Programs {
		if err := p.Serialize(w); err != nil {
			return errors.New("Transaction Programs Serialize failed, " + err.Error())
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
	if err := tx.Payload.Serialize(w, tx.PayloadVersion); err != nil {
		return err
	}

	//[]*txAttribute
	if err := WriteVarUint(w, uint64(len(tx.Attributes))); err != nil {
		return errors.New("Transaction item txAttribute length serialization failed.")
	}
	for _, attr := range tx.Attributes {
		if err := attr.Serialize(w); err != nil {
			return err
		}
	}

	//[]*Inputs
	if err := WriteVarUint(w, uint64(len(tx.Inputs))); err != nil {
		return errors.New("Transaction item Inputs length serialization failed.")
	}
	for _, utxo := range tx.Inputs {
		if err := utxo.Serialize(w); err != nil {
			return err
		}
	}

	//[]*Outputs
	if err := WriteVarUint(w, uint64(len(tx.Outputs))); err != nil {
		return errors.New("Transaction item Outputs length serialization failed.")
	}
	for _, output := range tx.Outputs {
		if err := output.Serialize(w); err != nil {
			return err
		}
	}

	return WriteUint32(w, tx.LockTime)
}

//deserialize the Transaction
func (tx *Transaction) Deserialize(r io.Reader) error {
	// tx deserialize
	err := tx.DeserializeUnsigned(r)
	if err != nil {
		return errors.New("transaction Deserialize error: " + err.Error())
	}

	// tx program
	count, err := ReadVarUint(r, 0)
	if err != nil {
		return errors.New("transaction write program count error: " + err.Error())
	}

	programHashes := make([]*Program, 0, count)
	for i := uint64(0); i < count; i++ {
		outputHashes := new(Program)
		err = outputHashes.Deserialize(r)
		if err != nil {
			return errors.New("transaction deserialize program error: " + err.Error())
		}
		programHashes = append(programHashes, outputHashes)
	}
	tx.Programs = programHashes
	return nil
}

func (tx *Transaction) DeserializeUnsigned(r io.Reader) error {
	var txType = make([]byte, 1)
	_, err := r.Read(txType)
	if err != nil {
		return err
	}
	tx.TxType = TransactionType(txType[0])

	var payloadVersion = make([]byte, 1)
	_, err = r.Read(payloadVersion)
	tx.PayloadVersion = payloadVersion[0]
	if err != nil {
		return err
	}

	tx.Payload, err = GetPayload(tx.TxType)
	if err != nil {
		return err
	}

	err = tx.Payload.Deserialize(r, tx.PayloadVersion)
	if err != nil {
		return errors.New("Payload Parse error")
	}
	//attributes
	Len, err := ReadVarUint(r, 0)
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
	Len, err = ReadVarUint(r, 0)
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
	Len, err = ReadVarUint(r, 0)
	if err != nil {
		return err
	}
	if Len > uint64(0) {
		for i := uint64(0); i < Len; i++ {
			output := new(Output)
			err = output.Deserialize(r)
			if err != nil {
				return err
			}
			tx.Outputs = append(tx.Outputs, output)
		}
	}

	temp, err := ReadUint32(r)
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

func (tx *Transaction) Hash() Uint256 {
	if tx.hash == nil {
		buf := new(bytes.Buffer)
		tx.SerializeUnsigned(buf)
		hash := Uint256(Sha256D(buf.Bytes()))
		tx.hash = &hash
	}
	return *tx.hash
}

func (tx *Transaction) IsWithdrawTx() bool {
	return tx.TxType == WithdrawAsset
}

func (tx *Transaction) IsIssueTokenTx() bool {
	return tx.TxType == IssueToken
}

func (tx *Transaction) IsCoinBaseTx() bool {
	return tx.TxType == CoinBase
}

func NewTrimmedTx(hash Uint256) *Transaction {
	tx := new(Transaction)
	tx.hash, _ = Uint256FromBytes(hash[:])
	return tx
}
