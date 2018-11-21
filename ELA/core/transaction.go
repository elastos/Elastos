package core

import (
	"bytes"
	"errors"
	"fmt"
	"io"

	. "github.com/elastos/Elastos.ELA.Utility/common"
	pg "github.com/elastos/Elastos.ELA/core/contract/program"
)

const (
	InvalidTransactionSize = -1
)

// TransactionType represents different transaction types with different payload format.
// The TransactionType range is 0x00 - 0xBF. When it is greater than 0xBF it will be
// interpreted as a TransactionVersion.
type TransactionType byte

const (
	CoinBase                TransactionType = 0x00
	RegisterAsset           TransactionType = 0x01
	TransferAsset           TransactionType = 0x02
	Record                  TransactionType = 0x03
	Deploy                  TransactionType = 0x04
	SideChainPow            TransactionType = 0x05
	RechargeToSideChain     TransactionType = 0x06
	WithdrawFromSideChain   TransactionType = 0x07
	TransferCrossChainAsset TransactionType = 0x08

	RegisterProducer TransactionType = 0xc1
	CancelProducer   TransactionType = 0xc2
	UpdateProducer   TransactionType = 0xc3
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
	case SideChainPow:
		return "SideChainPow"
	case RechargeToSideChain:
		return "RechargeToSideChain"
	case WithdrawFromSideChain:
		return "WithdrawFromSideChain"
	case TransferCrossChainAsset:
		return "TransferCrossChainAsset"
	case RegisterProducer:
		return "RegisterProducer"
	case CancelProducer:
		return "CancelProducer"
	case UpdateProducer:
		return "UpdateProducer"
	default:
		return "Unknown"
	}
}

type TransactionVersion byte

const (
	TxVersionDefault TransactionVersion = 0x00
	TxVersionC0      TransactionVersion = 0xC0
)

type Transaction struct {
	Version        TransactionVersion // New field added in TxVersionC0
	TxType         TransactionType
	PayloadVersion byte
	Payload        Payload
	Attributes     []*Attribute
	Inputs         []*Input
	Outputs        []*Output
	LockTime       uint32
	Programs       []*pg.Program
	Fee            Fixed64
	FeePerKB       Fixed64

	hash *Uint256
}

func (tx *Transaction) String() string {
	hash := tx.Hash()
	return fmt.Sprint("Transaction: {\n\t",
		"Hash: ", hash.String(), "\n\t",
		"Version: ", tx.Version, "\n\t",
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

// Serialize the Transaction
func (tx *Transaction) Serialize(w io.Writer) error {
	if err := tx.SerializeUnsigned(w); err != nil {
		return errors.New("Transaction txSerializeUnsigned Serialize failed, " + err.Error())
	}
	//Serialize  Transaction's programs
	if err := WriteVarUint(w, uint64(len(tx.Programs))); err != nil {
		return errors.New("Transaction program count failed.")
	}
	for _, program := range tx.Programs {
		if err := program.Serialize(w); err != nil {
			return errors.New("Transaction Programs Serialize failed, " + err.Error())
		}
	}
	return nil
}

// Serialize the Transaction data without contracts
func (tx *Transaction) SerializeUnsigned(w io.Writer) error {
	// Version
	if tx.Version >= TxVersionC0 {
		if _, err := w.Write([]byte{byte(tx.Version)}); err != nil {
			return err
		}
	}
	// TxType
	if _, err := w.Write([]byte{byte(tx.TxType)}); err != nil {
		return err
	}
	// PayloadVersion
	if _, err := w.Write([]byte{tx.PayloadVersion}); err != nil {
		return err
	}
	// Payload
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
		if err := output.Serialize(w, tx.Version); err != nil {
			return err
		}
	}

	return WriteUint32(w, tx.LockTime)
}

// Deserialize the Transaction
func (tx *Transaction) Deserialize(r io.Reader) error {
	// tx deserialize
	if err := tx.DeserializeUnsigned(r); err != nil {
		return errors.New("transaction Deserialize error: " + err.Error())
	}

	// tx program
	count, err := ReadVarUint(r, 0)
	if err != nil {
		return errors.New("transaction write program count error: " + err.Error())
	}
	for i := uint64(0); i < count; i++ {
		var program pg.Program
		if err := program.Deserialize(r); err != nil {
			return errors.New("transaction deserialize program error: " + err.Error())
		}
		tx.Programs = append(tx.Programs, &program)
	}
	return nil
}

func (tx *Transaction) DeserializeUnsigned(r io.Reader) error {
	flagByte, err := ReadBytes(r, 1)
	if err != nil {
		return err
	}

	if TransactionVersion(flagByte[0]) >= TxVersionC0 {
		tx.Version = TransactionVersion(flagByte[0])
		txType, err := ReadBytes(r, 1)
		if err != nil {
			return err
		}
		tx.TxType = TransactionType(txType[0])
	} else {
		tx.Version = TxVersionDefault
		tx.TxType = TransactionType(flagByte[0])
	}

	payloadVersion, err := ReadBytes(r, 1)
	if err != nil {
		return err
	}
	tx.PayloadVersion = payloadVersion[0]

	tx.Payload, err = GetPayload(tx.TxType)
	if err != nil {
		return err
	}

	err = tx.Payload.Deserialize(r, tx.PayloadVersion)
	if err != nil {
		return errors.New("deserialize Payload failed")
	}
	// attributes
	count, err := ReadVarUint(r, 0)
	if err != nil {
		return err
	}
	for i := uint64(0); i < count; i++ {
		var attr Attribute
		if err := attr.Deserialize(r); err != nil {
			return err
		}
		tx.Attributes = append(tx.Attributes, &attr)
	}
	// inputs
	count, err = ReadVarUint(r, 0)
	if err != nil {
		return err
	}
	for i := uint64(0); i < count; i++ {
		var input Input
		if err := input.Deserialize(r); err != nil {
			return err
		}
		tx.Inputs = append(tx.Inputs, &input)
	}
	// outputs
	count, err = ReadVarUint(r, 0)
	if err != nil {
		return err
	}
	for i := uint64(0); i < count; i++ {
		var output Output
		if err := output.Deserialize(r, tx.Version); err != nil {
			return err
		}
		tx.Outputs = append(tx.Outputs, &output)
	}

	tx.LockTime, err = ReadUint32(r)
	if err != nil {
		return err
	}

	return nil
}

func (tx *Transaction) GetSize() int {
	buf := new(bytes.Buffer)
	if err := tx.Serialize(buf); err != nil {
		return InvalidTransactionSize
	}
	return buf.Len()
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

func (tx *Transaction) IsUpdateProducerTx() bool {
	return tx.TxType == UpdateProducer
}

func (tx *Transaction) IsCancelProducerTx() bool {
	return tx.TxType == CancelProducer
}

func (tx *Transaction) IsRegisterProducerTx() bool {
	return tx.TxType == RegisterProducer
}

func (tx *Transaction) IsSideChainPowTx() bool {
	return tx.TxType == SideChainPow
}

func (tx *Transaction) IsTransferCrossChainAssetTx() bool {
	return tx.TxType == TransferCrossChainAsset
}

func (tx *Transaction) IsWithdrawFromSideChainTx() bool {
	return tx.TxType == WithdrawFromSideChain
}

func (tx *Transaction) IsRechargeToSideChainTx() bool {
	return tx.TxType == RechargeToSideChain
}

func (tx *Transaction) IsCoinBaseTx() bool {
	return tx.TxType == CoinBase
}

func NewTrimmedTx(hash Uint256) *Transaction {
	tx := new(Transaction)
	tx.hash, _ = Uint256FromBytes(hash[:])
	return tx
}
