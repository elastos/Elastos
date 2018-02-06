package transaction

import (
	"Elastos.ELA/common"
	"io"
)


type BalanceTxInput struct {
	AssetID common.Uint256
	Value common.Fixed64
	ProgramHash common.Uint168
}

func (self BalanceTxInput) String() string {
	return "BalanceTxInput: {\n\t\t" +
		"AssetID: " + self.AssetID.String() + "\n\t\t" +
		"Value: " + self.Value.String() + "\n\t\t" +
		"ProgramHash: " + self.ProgramHash.String() + "\n\t\t" +
		"}"
}

func (bi *BalanceTxInput) Serialize(w io.Writer)  {
	bi.AssetID.Serialize(w)
	bi.Value.Serialize(w)
	bi.ProgramHash.Serialize(w)
}

func (bi *BalanceTxInput) Deserialize(r io.Reader) error  {
	err := bi.AssetID.Deserialize(r)
	if err != nil {return err}

	err = bi.Value.Deserialize(r)
	if err != nil {return err}

	err = bi.ProgramHash.Deserialize(r)
	if err != nil {return err}

	return nil
}
