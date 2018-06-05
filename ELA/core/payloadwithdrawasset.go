package core

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

const WithdrawAssetPayloadVersion byte = 0x00

type PayloadWithdrawAsset struct {
	BlockHeight              uint32
	GenesisBlockAddress      string
	SideChainTransactionHash []string
}

func (t *PayloadWithdrawAsset) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := t.Serialize(buf, version); err != nil {
		return []byte{0}
	}

	return buf.Bytes()
}

func (t *PayloadWithdrawAsset) Serialize(w io.Writer, version byte) error {
	if err := common.WriteUint32(w, t.BlockHeight); err != nil {
		return errors.New("[WithdrawAsset], BlockHeight serialize failed.")
	}
	if err := common.WriteVarString(w, t.GenesisBlockAddress); err != nil {
		return errors.New("[WithdrawAsset], GenesisBlockAddress serialize failed.")
	}

	if err := common.WriteVarUint(w, uint64(len(t.SideChainTransactionHash))); err != nil {
		return errors.New("PayloadTransferCrossChainAsset length serialize failed")
	}

	for _, txHash := range t.SideChainTransactionHash {
		if err := common.WriteVarString(w, txHash); err != nil {
			return errors.New("[WithdrawAsset], SideChainTransactionHash serialize failed.")
		}
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

	length, err := common.ReadVarUint(r, 0)
	if err != nil {
		return errors.New("PayloadTransferCrossChainAsset length deserialize failed")
	}

	t.SideChainTransactionHash = nil
	t.SideChainTransactionHash = make([]string, length)
	for i := uint64(0); i < length; i++ {
		hash, err := common.ReadVarString(r)
		if err != nil {
			return errors.New("[WithdrawAsset], SideChainTransactionHash deserialize failed.")
		}
		t.SideChainTransactionHash[i] = hash
	}

	t.BlockHeight = height
	t.GenesisBlockAddress = address

	return nil
}
