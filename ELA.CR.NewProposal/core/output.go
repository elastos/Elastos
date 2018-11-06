package core

import (
	"fmt"
	"io"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type OutputPayloadType byte

const (
	DefaultOutput OutputPayloadType = 0x00
)

type OutputPayload interface {
	// Get payload data
	Data() []byte
	Serialize(w io.Writer) error
	Deserialize(r io.Reader) error
	GetType() (OutputPayloadType, error)
	GetVersion() (byte, error)
	String() string
}

type Output struct {
	AssetID       Uint256
	Value         Fixed64
	OutputLock    uint32
	ProgramHash   Uint168
	OutputPayload OutputPayload
}

func (o Output) String() string {
	return "Output: {\n\t\t" +
		"AssetID: " + o.AssetID.String() + "\n\t\t" +
		"Value: " + o.Value.String() + "\n\t\t" +
		"OutputLock: " + fmt.Sprint(o.OutputLock) + "\n\t\t" +
		"ProgramHash: " + o.ProgramHash.String() + "\n\t\t" +
		"OutputPayload: " + o.OutputPayload.String() + "\n\t\t" +
		"}"
}

func (o *Output) Serialize(w io.Writer, txVersion TransactionVersion) error {
	if err := o.AssetID.Serialize(w); err != nil {
		return err
	}

	if err := o.Value.Serialize(w); err != nil {
		return err
	}

	if err := WriteUint32(w, o.OutputLock); err != nil {
		return err
	}

	if err := o.ProgramHash.Serialize(w); err != nil {
		return err
	}

	if txVersion == TxVersionC0 {
		if err := o.OutputPayload.Serialize(w); err != nil {
			return err
		}
	}

	return nil
}

func (o *Output) Deserialize(r io.Reader, txVersion TransactionVersion) error {
	if err := o.AssetID.Deserialize(r); err != nil {
		return err
	}

	if err := o.Value.Deserialize(r); err != nil {
		return err
	}

	temp, err := ReadUint32(r)
	if err != nil {
		return err
	}
	o.OutputLock = temp

	if err := o.ProgramHash.Deserialize(r); err != nil {
		return err
	}

	if txVersion == TxVersionC0 {
		if err := o.OutputPayload.Deserialize(r); err != nil {
			return err
		}
	}

	return nil
}
