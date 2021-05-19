package types

import (
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

type PayloadTransferCrossChainAsset struct {
	CrossChainAddresses []string
	OutputIndexes       []uint64
	CrossChainAmounts   []common.Fixed64
}

func (a *PayloadTransferCrossChainAsset) Data(version byte) []byte {
	//TODO: implement TransferCrossChainAsset.Data()
	return []byte{0}
}

func (a *PayloadTransferCrossChainAsset) Serialize(w io.Writer, version byte) error {
	if len(a.CrossChainAddresses) != len(a.OutputIndexes) || len(a.OutputIndexes) != len(a.CrossChainAmounts) {
		return errors.New("Invalid cross chain asset")
	}

	if err := common.WriteVarUint(w, uint64(len(a.CrossChainAddresses))); err != nil {
		return errors.New("PayloadTransferCrossChainAsset length serialize failed")
	}

	for i := 0; i < len(a.CrossChainAddresses); i++ {
		if err := common.WriteVarString(w, a.CrossChainAddresses[i]); err != nil {
			return errors.New("CrossChainAddresses serialize failed")
		}

		if err := common.WriteVarUint(w, a.OutputIndexes[i]); err != nil {
			return errors.New("OutputIndexes serialize failed")
		}

		err := a.CrossChainAmounts[i].Serialize(w)
		if err != nil {
			return errors.New("CrossChainAmounts serialize failed")
		}
	}

	return nil
}

func (a *PayloadTransferCrossChainAsset) Deserialize(r io.Reader, version byte) error {
	length, err := common.ReadVarUint(r, 0)
	if err != nil {
		return errors.New("PayloadTransferCrossChainAsset length deserialize failed")
	}

	for i := uint64(0); i < length; i++ {
		address, err := common.ReadVarString(r)
		if err != nil {
			return errors.New("CrossChainAddresses deserialize failed")
		}

		index, err := common.ReadVarUint(r, 0)
		if err != nil {
			return errors.New("OutputIndexes deserialize failed")
		}

		var amount common.Fixed64
		err = amount.Deserialize(r)
		if err != nil {
			return errors.New("CrossChainAmounts deserialize failed")
		}

		a.CrossChainAddresses = append(a.CrossChainAddresses, address)
		a.OutputIndexes = append(a.OutputIndexes, index)
		a.CrossChainAmounts = append(a.CrossChainAmounts, amount)
	}

	return nil
}
