package transaction

import (
	"Elastos.ELA/common"
	"Elastos.ELA/common/serialize"
	"fmt"
	"io"
)

type UTXOTxInput struct {

	//Indicate the previous Tx which include the UTXO output for usage
	ReferTxID common.Uint256

	//The index of output in the referTx output list
	ReferTxOutputIndex uint16

	// Sequence number
	Sequence uint32
}

func (self UTXOTxInput) String() string {
	return "UTXOTxInput: {\n\t\t" +
		"ReferTxID: " + self.ReferTxID.String() + "\n\t\t" +
		"ReferTxOutputIndex: " + fmt.Sprint(self.ReferTxOutputIndex) + "\n\t\t" +
		"Sequence: " + fmt.Sprint(self.Sequence) + "\n\t\t" +
		"}"
}

func (ui *UTXOTxInput) Serialize(w io.Writer) {
	ui.ReferTxID.Serialize(w)
	serialize.WriteUint16(w, ui.ReferTxOutputIndex)
	serialize.WriteUint32(w, ui.Sequence)
}

func (ui *UTXOTxInput) Deserialize(r io.Reader) error {
	//referTxID
	err := ui.ReferTxID.Deserialize(r)
	if err != nil {
		return err
	}

	//Output Index
	temp, err := serialize.ReadUint16(r)
	ui.ReferTxOutputIndex = uint16(temp)
	if err != nil {
		return err
	}

	temp2, err := serialize.ReadUint32(r)
	ui.Sequence = uint32(temp2)
	if err != nil {
		return err
	}

	return nil
}

func (ui *UTXOTxInput) ToString() string {
	return fmt.Sprintf("%x%x", ui.ReferTxID.String(), ui.ReferTxOutputIndex)
}

func (ui *UTXOTxInput) Equals(other *UTXOTxInput) bool {
	if ui == other {
		return true
	}
	if other == nil {
		return false
	}
	if ui.ReferTxID == other.ReferTxID && ui.ReferTxOutputIndex == other.ReferTxOutputIndex {
		return true
	} else {
		return false
	}
}
