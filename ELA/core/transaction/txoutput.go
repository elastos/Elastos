package transaction

import (
	"io"
	"fmt"

	"Elastos.ELA/common"
	"Elastos.ELA/common/serialize"
)

type TxOutput struct {
	AssetID     common.Uint256
	Value       common.Fixed64
	OutputLock  uint32
	ProgramHash common.Uint168
}

func (self TxOutput) String() string {
	return "TxOutput: {\n\t\t" +
		"AssetID: " + self.AssetID.String() + "\n\t\t" +
		"Value: " + self.Value.String() + "\n\t\t" +
		"OutputLock: " + fmt.Sprint(self.OutputLock) + "\n\t\t" +
		"ProgramHash: " + self.ProgramHash.String() + "\n\t\t" +
		"}"
}

func (o *TxOutput) Serialize(w io.Writer) error {
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

func (o *TxOutput) Deserialize(r io.Reader) error {
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
