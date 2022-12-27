package types

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
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

func (t *PayloadRechargeToSideChain) GetMainchainTxHash(payloadVersion byte) (*common.Uint256, error) {
	if payloadVersion == RechargeToSideChainPayloadVersion0 {
		mainchainTx := new(types.Transaction)
		reader := bytes.NewReader(t.MainChainTransaction)
		if err := mainchainTx.Deserialize(reader); err != nil {
			return nil, errors.New("[GetMainchainTxHash] RechargeToSideChain mainChainTransaction deserialize failed")
		}

		hash := mainchainTx.Hash()
		return &hash, nil
	} else if payloadVersion == RechargeToSideChainPayloadVersion1 {
		return &t.MainChainTransactionHash, nil
	} else {
		return nil, errors.New("[GetMainchainTxHash] Invalid payload version")
	}

}
