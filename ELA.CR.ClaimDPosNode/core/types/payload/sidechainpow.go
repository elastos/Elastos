package payload

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

const SideChainPowPayloadVersion byte = 0x00

type PayloadSideChainPow struct {
	SideBlockHash   common.Uint256
	SideGenesisHash common.Uint256
	BlockHeight     uint32
	SignedData      []byte
}

func (a *PayloadSideChainPow) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := a.Serialize(buf, version); err != nil {
		return []byte{0}
	}

	return buf.Bytes()
}

func (a *PayloadSideChainPow) Serialize(w io.Writer, version byte) error {
	err := a.SideBlockHash.Serialize(w)
	if err != nil {
		return errors.New("[PayloadSideChainPow], SideBlockHash serialize failed.")
	}
	err = a.SideGenesisHash.Serialize(w)
	if err != nil {
		return errors.New("[PayloadSideChainPow], SideGenesisHash serialize failed.")
	}
	err = common.WriteUint32(w, a.BlockHeight)
	if err != nil {
		return errors.New("[PayloadSideChainPow], BlockHeight serialize failed.")
	}
	err = common.WriteVarBytes(w, a.SignedData)
	if err != nil {
		return errors.New("[PayloadSideChainPow], SignatureData serialize failed.")
	}
	return nil
}

func (a *PayloadSideChainPow) Deserialize(r io.Reader, version byte) error {
	err := a.SideBlockHash.Deserialize(r)
	if err != nil {
		return errors.New("[PayloadSideChainPow], SignatureData dserialize failed.")
	}
	err = a.SideGenesisHash.Deserialize(r)
	if err != nil {
		return errors.New("[PayloadSideChainPow], SignatureData dserialize failed.")
	}
	a.BlockHeight, err = common.ReadUint32(r)
	if err != nil {
		return errors.New("[PayloadSideChainPow], SignatureData dserialize failed.")
	}
	if a.SignedData, err = common.ReadVarBytes(r, MaxCoinbasePayloadDataSize,
		"payload sidechainpow signed data"); err != nil {
		return errors.New("[PayloadSideChainPow], SignatureData dserialize failed.")
	}
	return nil
}
