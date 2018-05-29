package core

import (
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type PayloadTransferCrossChainAsset struct {
	CrossChainAddress []string
	OutputIndex       []uint64
	CrossChainAmount  []common.Fixed64
}

func (a *PayloadTransferCrossChainAsset) Data(version byte) []byte {
	//TODO: implement TransferCrossChainAsset.Data()
	return []byte{0}
}

func (a *PayloadTransferCrossChainAsset) Serialize(w io.Writer, version byte) error {
	if len(a.CrossChainAddress) != len(a.OutputIndex) || len(a.OutputIndex) != len(a.CrossChainAmount) {
		return errors.New("Invalid cross chain asset")
	}

	if err := common.WriteVarUint(w, uint64(len(a.CrossChainAddress))); err != nil {
		return errors.New("PayloadTransferCrossChainAsset length serialize failed")
	}

	for i := 0; i < len(a.CrossChainAddress); i++ {
		if err := common.WriteVarString(w, a.CrossChainAddress[i]); err != nil {
			return errors.New("CrossChainAddress serialize failed")
		}

		if err := common.WriteVarUint(w, a.OutputIndex[i]); err != nil {
			return errors.New("OutputIndex serialize failed")
		}

		err := a.CrossChainAmount[i].Serialize(w)
		if err != nil {
			return errors.New("CrossChainAmount serialize failed")
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
			return errors.New("CrossChainAddress deserialize failed")
		}

		index, err := common.ReadVarUint(r, 0)
		if err != nil {
			return errors.New("OutputIndex index deserialize failed")
		}

		var amount common.Fixed64
		err = amount.Deserialize(r)
		if err != nil {
			return errors.New("CrossChainAmount deserialize failed")
		}

		a.CrossChainAddress = append(a.CrossChainAddress, address)
		a.OutputIndex = append(a.OutputIndex, index)
		a.CrossChainAmount = append(a.CrossChainAmount, amount)
	}

	return nil
}
