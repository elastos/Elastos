package types

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
	ela "github.com/elastos/Elastos.ELA/core"
)

const (
	RechargeToSideChainPayloadVersion0 byte = 0x00
	RechargeToSideChainPayloadVersion1 byte = 0x01
)

type PayloadRechargeToSideChain struct {
	MerkleProof              []byte
	MainChainTransaction     []byte
	MainChainTransactionHash common.Uint256
}

func (t *PayloadRechargeToSideChain) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := t.Serialize(buf, version); err != nil {
		return []byte{0}
	}

	return buf.Bytes()
}

func (t *PayloadRechargeToSideChain) Serialize(w io.Writer, version byte) error {
	if version == RechargeToSideChainPayloadVersion0 {
		err := common.WriteVarBytes(w, t.MerkleProof)
		if err != nil {
			return errors.New("[PayloadRechargeToSideChain], MerkleProof serialize failed.")
		}
		err = common.WriteVarBytes(w, t.MainChainTransaction)
		if err != nil {
			return errors.New("[PayloadRechargeToSideChain], DepositTransaction serialize failed.")
		}
	} else if version == RechargeToSideChainPayloadVersion1 {
		err := t.MainChainTransactionHash.Serialize(w)
		if err != nil {
			return errors.New("[PayloadRechargeToSideChain], MainChainTransactionHash serialize failed.")
		}
	} else {
		return errors.New("[PayloadRechargeToSideChain] serialize: invalid payload version.")
	}

	return nil
}

func (t *PayloadRechargeToSideChain) Deserialize(r io.Reader, version byte) error {
	if version == RechargeToSideChainPayloadVersion0 {
		var err error
		if t.MerkleProof, err = common.ReadVarBytes(r, MaxPayloadDataSize,
			"PayloadRechargeToSideChain MerkleProof"); err != nil {
			return errors.New("[PayloadRechargeToSideChain], MerkleProof deserialize failed.")
		}

		if t.MainChainTransaction, err = common.ReadVarBytes(r, MaxPayloadDataSize,
			"PayloadRechargeToSideChain MainChainTransaction"); err != nil {
			return errors.New("[PayloadRechargeToSideChain], DepositTransaction deserialize failed.")
		}
	} else if version == RechargeToSideChainPayloadVersion1 {
		if err := t.MainChainTransactionHash.Deserialize(r); err != nil {
			return errors.New("[PayloadRechargeToSideChain], MainChainTransactionHash deserialize failed.")
		}
	} else {
		return errors.New("[PayloadRechargeToSideChain] deserialize: invalid payload version.")
	}

	return nil
}

func (t *PayloadRechargeToSideChain) GetMainchainTxHash() (*common.Uint256, error) {
	mainchainTx := new(ela.Transaction)
	reader := bytes.NewReader(t.MainChainTransaction)
	if err := mainchainTx.Deserialize(reader); err != nil {
		return nil, errors.New("RechargeToSideChain mainChainTransaction deserialize failed")
	}

	hash := mainchainTx.Hash()
	return &hash, nil
}
