package core

import (
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

const WithdrawAssetPayloadVersion byte = 0x00

type PayloadWithdrawAsset struct {
	BlockHeight              uint32
	GenesisBlockAddress      string
	SideChainTransactionHash string
}

func (t *PayloadWithdrawAsset) Data(version byte) []byte {
	return []byte{0}
}

func (t *PayloadWithdrawAsset) Serialize(w io.Writer, version byte) error {
	if err := common.WriteUint32(w, t.BlockHeight); err != nil {
		return errors.New("[WithdrawAsset], BlockHeight serialize failed.")
	}
	if err := common.WriteVarString(w, t.GenesisBlockAddress); err != nil {
		return errors.New("[WithdrawAsset], GenesisBlockAddress serialize failed.")
	}
	if err := common.WriteVarString(w, t.SideChainTransactionHash); err != nil {
		return errors.New("[WithdrawAsset], SideChainTransactionHash serialize failed.")
	}

	return nil
}

func (t *PayloadWithdrawAsset) Deserialize(r io.Reader, version byte) error {
	height, err := common.ReadUint32(r)
	if err != nil {
		return errors.New("[WithdrawAsset], BlockHeight deserialize failed.")
	}
	address, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[WithdrawAsset], GenesisBlockAddress deserialize failed.")
	}
	hash, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[WithdrawAsset], SideChainTransactionHash deserialize failed.")
	}

	t.BlockHeight = height
	t.GenesisBlockAddress = address
	t.SideChainTransactionHash = hash

	return nil
}
