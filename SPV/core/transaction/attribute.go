package transaction

import (
	"io"
	"fmt"
	"errors"

	"SPVWallet/core"
	"SPVWallet/core/serialization"
)

type AttributeUsage byte

const (
	Nonce          AttributeUsage = 0x00
	Script         AttributeUsage = 0x20
	DescriptionUrl AttributeUsage = 0x81
	Description    AttributeUsage = 0x90
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
		usage == DescriptionUrl || usage == Description
}

type Attribute struct {
	Usage AttributeUsage
	Data  []byte
	Size  uint32
}

func (attr Attribute) String() string {
	return "Attribute: {\n\t\t" +
		"Usage: " + attr.Usage.Name() + "\n\t\t" +
		"Data: " + core.BytesToHexString(attr.Data) + "\n\t\t" +
		"Size: " + fmt.Sprint(attr.Size) + "\n\t" +
		"}"
}

func NewAttribute(u AttributeUsage, d []byte) Attribute {
	tx := Attribute{u, d, 0}
	tx.Size = tx.GetSize()
	return tx
}

func (attr *Attribute) GetSize() uint32 {
	if attr.Usage == DescriptionUrl {
		return uint32(len([]byte{(byte(0xff))}) + len([]byte{(byte(0xff))}) + len(attr.Data))
	}
	return 0
}

func (attr *Attribute) Serialize(w io.Writer) error {
	if err := serialization.WriteUint8(w, byte(attr.Usage)); err != nil {
		return errors.New("Transaction attribute Usage serialization error.")
	}
	if !IsValidAttributeType(attr.Usage) {
		return errors.New("[Attribute] error: Unsupported attribute Description.")
	}
	if err := serialization.WriteVarBytes(w, attr.Data); err != nil {
		return errors.New("Transaction attribute Data serialization error.")
	}
	return nil
}

func (attr *Attribute) Deserialize(r io.Reader) error {
	val, err := serialization.ReadBytes(r, 1)
	if err != nil {
		return errors.New("Transaction attribute Usage deserialization error.")
	}
	attr.Usage = AttributeUsage(val[0])
	if !IsValidAttributeType(attr.Usage) {
		return errors.New("[Attribute] error: Unsupported attribute Description.")
	}
	attr.Data, err = serialization.ReadVarBytes(r)
	if err != nil {
		return errors.New("Transaction attribute Data deserialization error.")
	}
	return nil

}
