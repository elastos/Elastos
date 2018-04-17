package transaction

import (
	"Elastos.ELA/common/serialize"
	"errors"
	"io"
	"fmt"
	"Elastos.ELA/common"
)

type TransactionAttributeUsage byte

const (
	Nonce          TransactionAttributeUsage = 0x00
	Script         TransactionAttributeUsage = 0x20
	DescriptionUrl TransactionAttributeUsage = 0x81
	Description    TransactionAttributeUsage = 0x90
)

func (self TransactionAttributeUsage) Name() string {
	switch self {
	case Nonce:
		return "Nonce"
	case Script:
		return "Script"
	case DescriptionUrl:
		return "DescriptionUrl"
	case Description:
		return "Description"
	default:
		return "Unknown"
	}
}

func IsValidAttributeType(usage TransactionAttributeUsage) bool {
	return usage == Nonce || usage == Script ||
		usage == DescriptionUrl || usage == Description
}

type TxAttribute struct {
	Usage TransactionAttributeUsage
	Data  []byte
	Size  uint32
}

func (self TxAttribute) String() string {
	return "TxAttribute: {\n\t\t" +
		"Usage: " + self.Usage.Name() + "\n\t\t" +
		"Data: " + common.BytesToHexString(self.Data) + "\n\t\t" +
		"Size: " + fmt.Sprint(self.Size) + "\n\t" +
		"}"
}

func NewTxAttribute(u TransactionAttributeUsage, d []byte) TxAttribute {
	tx := TxAttribute{u, d, 0}
	tx.Size = tx.GetSize()
	return tx
}

func (u *TxAttribute) GetSize() uint32 {
	if u.Usage == DescriptionUrl {
		return uint32(len([]byte{(byte(0xff))}) + len([]byte{(byte(0xff))}) + len(u.Data))
	}
	return 0
}

func (tx *TxAttribute) Serialize(w io.Writer) error {
	if err := serialize.WriteUint8(w, byte(tx.Usage)); err != nil {
		return errors.New("Transaction attribute Usage serialization error.")
	}
	if !IsValidAttributeType(tx.Usage) {
		return errors.New("[TxAttribute error] Unsupported attribute Description.")
	}
	if err := serialize.WriteVarBytes(w, tx.Data); err != nil {
		return errors.New("Transaction attribute Data serialization error.")
	}
	return nil
}

func (tx *TxAttribute) Deserialize(r io.Reader) error {
	val, err := serialize.ReadBytes(r, 1)
	if err != nil {
		return errors.New("Transaction attribute Usage deserialization error.")
	}
	tx.Usage = TransactionAttributeUsage(val[0])
	if !IsValidAttributeType(tx.Usage) {
		return errors.New("[TxAttribute error] Unsupported attribute Description.")
	}
	tx.Data, err = serialize.ReadVarBytes(r)
	if err != nil {
		return errors.New("Transaction attribute Data deserialization error.")
	}
	return nil
}
