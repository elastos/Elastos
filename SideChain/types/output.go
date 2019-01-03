package types

import (
	"fmt"
	"io"
	"math/big"

	"github.com/elastos/Elastos.ELA/common"
)

type Output struct {
	AssetID     common.Uint256
	Value       common.Fixed64
	TokenValue  big.Int
	OutputLock  uint32
	ProgramHash common.Uint168
}

func (o Output) String() string {
	return "Output: {\n\t\t" +
		"AssetID: " + o.AssetID.String() + "\n\t\t" +
		"Value: " + o.Value.String() + "\n\t\t" +
		"TokenValue: " + o.TokenValue.String() + "\n\t\t" +
		"OutputLock: " + fmt.Sprint(o.OutputLock) + "\n\t\t" +
		"ProgramHash: " + o.ProgramHash.String() + "\n\t\t" +
		"}"
}

func (o *Output) Serialize(w io.Writer) error {
	return SerializeOutput(o, w)
}

func (o *Output) Deserialize(r io.Reader) error {
	return DeserializeOutput(o, r)
}

var SerializeOutput = func(output *Output, w io.Writer) error {
	err := output.AssetID.Serialize(w)
	if err != nil {
		return err
	}

	err = output.Value.Serialize(w)
	if err != nil {
		return err
	}

	common.WriteUint32(w, output.OutputLock)

	err = output.ProgramHash.Serialize(w)
	if err != nil {
		return err
	}

	return nil
}

var DeserializeOutput = func(output *Output, r io.Reader) error {
	err := output.AssetID.Deserialize(r)
	if err != nil {
		return err
	}

	err = output.Value.Deserialize(r)
	if err != nil {
		return err
	}

	temp, err := common.ReadUint32(r)
	if err != nil {
		return err
	}
	output.OutputLock = uint32(temp)

	err = output.ProgramHash.Deserialize(r)
	if err != nil {
		return err
	}

	return nil
}
