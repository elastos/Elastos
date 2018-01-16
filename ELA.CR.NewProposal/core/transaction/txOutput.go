package transaction

import (

	"Elastos.ELA/common"
	"Elastos.ELA/common/serialization"
	"io"
)

type TxOutput struct {
	AssetID     common.Uint256
	Value       common.Fixed64
	OutputLock  uint32
	ProgramHash common.Uint168
}

func (o *TxOutput) Serialize(w io.Writer) {
	o.AssetID.Serialize(w)
	o.Value.Serialize(w)
	serialization.WriteUint32(w, o.OutputLock)
	o.ProgramHash.Serialize(w)
}

func (o *TxOutput) Deserialize(r io.Reader) {
	o.AssetID.Deserialize(r)
	o.Value.Deserialize(r)
	temp, _ := serialization.ReadUint32(r)
	o.OutputLock = uint32(temp)
	o.ProgramHash.Deserialize(r)
}
