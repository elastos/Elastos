package types

import (
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

type AttributeUsage byte

const (
	Nonce          AttributeUsage = 0x00
	Script         AttributeUsage = 0x20
	Memo           AttributeUsage = 0x81
	Description    AttributeUsage = 0x90
	DescriptionUrl AttributeUsage = 0x91
)

func (u AttributeUsage) Name() string {
	switch u {
	case Nonce:
		return "Nonce"
	case Script:
		return "Script"
	case Memo:
		return "Memo"
	case Description:
		return "Description"
	case DescriptionUrl:
		return "DescriptionUrl"
	default:
		return "Unknown"
	}
}

func IsValidAttributeType(usage AttributeUsage) bool {
	switch usage {
	case Nonce, Script, Memo, Description, DescriptionUrl:
		return true
	}
	return false
}

type Attribute struct {
	Usage AttributeUsage
	Data  []byte
}

func (a Attribute) String() string {
	return "Attribute: {\n\t\t" +
		"Usage: " + a.Usage.Name() + "\n\t\t" +
		"Data: " + common.BytesToHexString(a.Data) + "\n\t\t" +
		"}"
}

func NewAttribute(u AttributeUsage, d []byte) Attribute {
	return Attribute{u, d}
}

func (a *Attribute) Serialize(w io.Writer) error {
	if err := common.WriteUint8(w, byte(a.Usage)); err != nil {
		return errors.New("Transaction attribute Usage serialization error.")
	}
	if !IsValidAttributeType(a.Usage) {
		return errors.New("[Attribute error] Unsupported attribute Description.")
	}
	if err := common.WriteVarBytes(w, a.Data); err != nil {
		return errors.New("Transaction attribute Data serialization error.")
	}
	return nil
}

func (a *Attribute) Deserialize(r io.Reader) error {
	val, err := common.ReadBytes(r, 1)
	if err != nil {
		return errors.New("Transaction attribute Usage deserialization error.")
	}
	a.Usage = AttributeUsage(val[0])
	if !IsValidAttributeType(a.Usage) {
		return errors.New("[Attribute error] Unsupported attribute Description.")
	}
	a.Data, err = common.ReadVarBytes(r, common.MaxVarStringLength,
		"attribute data")
	if err != nil {
		return errors.New("Transaction attribute Data deserialization error.")
	}
	return nil
}
