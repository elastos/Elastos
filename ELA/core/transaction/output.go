package transaction

import (
	"io"
	"fmt"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/serialize"
)

type Output struct {
	AssetID     common.Uint256
	Value       common.Fixed64
	OutputLock  uint32
	ProgramHash common.Uint168
}

func (o Output) String() string {
	return "Output: {\n\t\t" +
		"AssetID: " + o.AssetID.String() + "\n\t\t" +
		"Value: " + o.Value.String() + "\n\t\t" +
		"OutputLock: " + fmt.Sprint(o.OutputLock) + "\n\t\t" +
		"ProgramHash: " + o.ProgramHash.String() + "\n\t\t" +
		"}"
}

func (o *Output) Serialize(w io.Writer) error {
	err := o.AssetID.Serialize(w)
	if err != nil {
		return err
	}

	err = o.Value.Serialize(w)
	if err != nil {
		return err
	}

	serialize.WriteUint32(w, o.OutputLock)

	err = o.ProgramHash.Serialize(w)
	if err != nil {
		return err
	}

	return nil
}

func (o *Output) Deserialize(r io.Reader) error {
	err := o.AssetID.Deserialize(r)
	if err != nil {
		return err
	}

	err = o.Value.Deserialize(r)
	if err != nil {
		return err
	}

	temp, err := serialize.ReadUint32(r)
	if err != nil {
		return err
	}
	o.OutputLock = uint32(temp)

	err = o.ProgramHash.Deserialize(r)
	if err != nil {
		return err
	}

	return nil
}
