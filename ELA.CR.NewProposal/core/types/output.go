package types

import (
	"errors"
	"fmt"
	"io"

	"github.com/elastos/Elastos.ELA/core/types/outputpayload"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type OutputType byte

const (
	DefaultOutput OutputType = 0x00
	VoteOutput    OutputType = 0x01
)

type OutputPayload interface {
	// Get payload data
	Data() []byte
	Serialize(w io.Writer) error
	Deserialize(r io.Reader) error
	GetVersion() byte
	Validate() error
}

type Output struct {
	AssetID       Uint256
	Value         Fixed64
	OutputLock    uint32
	ProgramHash   Uint168
	OutputType    OutputType
	OutputPayload OutputPayload
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

	if txVersion >= TxVersion09 {
		if err := WriteUint8(w, byte(o.OutputType)); err != nil {
			return err
		}
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

	if txVersion >= TxVersion09 {
		outputType, err := ReadUint8(r)
		if err != nil {
			return err
		}
		o.OutputType = OutputType(outputType)
		o.OutputPayload, err = getOutputPayload(OutputType(outputType))
		if err != nil {
			return err
		}
		if err = o.OutputPayload.Deserialize(r); err != nil {
			return err
		}
	}

	return nil
}

func (o *Output) String() string {
	outputStr := fmt.Sprint("Output: {\n\t",
		"AssetID: ", o.AssetID.String(), "\n\t",
		"Value: ", o.Value.String(), "\n\t",
		"OutputLock: ", o.OutputLock, "\n\t",
		"ProgramHash: ", o.ProgramHash.String(), "\n\t")

	if o.OutputPayload != nil {
		outputStr += fmt.Sprint("OutputType: ", o.OutputType, "\n\t", "OutputPayload: ", o.OutputPayload, "\n\t")
	}
	outputStr += "}"

	return outputStr
}

func getOutputPayload(outputType OutputType) (OutputPayload, error) {
	var op OutputPayload
	switch outputType {
	case DefaultOutput:
		op = new(outputpayload.DefaultOutput)
	case VoteOutput:
		op = new(outputpayload.VoteOutput)
	default:
		return nil, errors.New("invalid transaction output type")
	}
	return op, nil
}
