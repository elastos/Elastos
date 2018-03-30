package transaction

import (
	"io"
	"fmt"

	"github.com/elastos/Elastos.ELA.SPV/core"
	"github.com/elastos/Elastos.ELA.SPV/core/serialization"
)

type Input struct {
	//Indicate the previous Tx which include the UTXO output for usage
	ReferTxID core.Uint256

	//The index of output in the referTx output list
	ReferTxOutputIndex uint16

	// Sequence number
	Sequence uint32
}

func (self Input) String() string {
	return "Input: {\n\t\t" +
		"ReferTxID: " + self.ReferTxID.String() + "\n\t\t" +
		"ReferTxOutputIndex: " + fmt.Sprint(self.ReferTxOutputIndex) + "\n\t\t" +
		"Sequence: " + fmt.Sprint(self.Sequence) + "\n\t\t" +
		"}"
}

func (ui *Input) Serialize(w io.Writer) {
	ui.ReferTxID.Serialize(w)
	serialization.WriteUint16(w, ui.ReferTxOutputIndex)
	serialization.WriteUint32(w, ui.Sequence)
}

func (ui *Input) Deserialize(r io.Reader) error {
	//referTxID
	err := ui.ReferTxID.Deserialize(r)
	if err != nil {
		return err
	}

	//Output Index
	temp, err := serialization.ReadUint16(r)
	ui.ReferTxOutputIndex = uint16(temp)
	if err != nil {
		return err
	}

	temp2, err := serialization.ReadUint32(r)
	ui.Sequence = uint32(temp2)
	if err != nil {
		return err
	}

	return nil
}

func (ui *Input) Equals(other *Input) bool {
	if ui == other {
		return true
	}
	if other == nil {
		return false
	}
	if ui.ReferTxID == other.ReferTxID &&
		ui.ReferTxOutputIndex == other.ReferTxOutputIndex &&
		ui.Sequence == other.Sequence {
		return true
	} else {
		return false
	}
}
