package core

import (
	"bytes"
	"errors"
	"io"

	ela "github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA.Utility/common"
)

const RechargeToSideChainPayloadVersion byte = 0x00

type PayloadRechargeToSideChain struct {
	MerkleProof          []byte
	MainChainTransaction []byte
}

func (t *PayloadRechargeToSideChain) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := t.Serialize(buf, version); err != nil {
		return []byte{0}
	}

	return buf.Bytes()
}

func (t *PayloadRechargeToSideChain) Serialize(w io.Writer, version byte) error {
	err := common.WriteVarBytes(w, t.MerkleProof)
	if err != nil {
		return errors.New("[PayloadRechargeToSideChain], MerkleProof serialize failed.")
	}
	err = common.WriteVarBytes(w, t.MainChainTransaction)
	if err != nil {
		return errors.New("[PayloadRechargeToSideChain], DepositTransaction serialize failed.")
	}
	return nil
}

func (t *PayloadRechargeToSideChain) Deserialize(r io.Reader, version byte) error {
	var err error
	if t.MerkleProof, err = common.ReadVarBytes(r); err != nil {
		return errors.New("[PayloadRechargeToSideChain], MerkleProof deserialize failed.")
	}

	if t.MainChainTransaction, err = common.ReadVarBytes(r); err != nil {
		return errors.New("[PayloadRechargeToSideChain], DepositTransaction deserialize failed.")
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