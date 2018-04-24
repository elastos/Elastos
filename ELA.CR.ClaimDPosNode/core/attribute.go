package core

import (
	"errors"
	"io"
	"fmt"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type AttributeUsage byte

const (
	Nonce          AttributeUsage = 0x00
	Script         AttributeUsage = 0x20
	DescriptionUrl AttributeUsage = 0x81
	Description    AttributeUsage = 0x90
	Memo           AttributeUsage = 0x91
)

func (self AttributeUsage) Name() string {
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

func IsValidAttributeType(usage AttributeUsage) bool {
	return usage == Nonce || usage == Script ||
		usage == DescriptionUrl || usage == Description || usage == Memo
}

type Attribute struct {
	Usage AttributeUsage
	Data  []byte
	Size  uint32
}

func (self Attribute) String() string {
	return "Attribute: {\n\t\t" +
		"Usage: " + self.Usage.Name() + "\n\t\t" +
		"Data: " + BytesToHexString(self.Data) + "\n\t\t" +
		"Size: " + fmt.Sprint(self.Size) + "\n\t" +
		"}"
}

func NewAttribute(u AttributeUsage, d []byte) Attribute {
	tx := Attribute{u, d, 0}
	tx.Size = tx.GetSize()
	return tx
}

func (u *Attribute) GetSize() uint32 {
	if u.Usage == DescriptionUrl {
		return uint32(len([]byte{(byte(0xff))}) + len([]byte{(byte(0xff))}) + len(u.Data))
	}
	return 0
}

func (tx *Attribute) Serialize(w io.Writer) error {
	if err := WriteUint8(w, byte(tx.Usage)); err != nil {
		return errors.New("Transaction attribute Usage serialization error.")
	}
	if !IsValidAttributeType(tx.Usage) {
		return errors.New("[Attribute error] Unsupported attribute Description.")
	}
	if err := WriteVarBytes(w, tx.Data); err != nil {
		return errors.New("Transaction attribute Data serialization error.")
	}
	return nil
}

func (tx *Attribute) Deserialize(r io.Reader) error {
	val, err := ReadBytes(r, 1)
	if err != nil {
		return errors.New("Transaction attribute Usage deserialization error.")
	}
	tx.Usage = AttributeUsage(val[0])
	if !IsValidAttributeType(tx.Usage) {
		return errors.New("[Attribute error] Unsupported attribute Description.")
	}
	tx.Data, err = ReadVarBytes(r)
	if err != nil {
		return errors.New("Transaction attribute Data deserialization error.")
	}
	return nil
}
