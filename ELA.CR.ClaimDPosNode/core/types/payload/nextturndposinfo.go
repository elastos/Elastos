package payload

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
)

const NextTurnDPOSInfoVersion byte = 0x00

type NextTurnDPOSInfo struct {
	WorkingHeight  uint32
	CRPublickeys   [][]byte
	DPOSPublicKeys [][]byte

	hash *common.Uint256
}

func (n *NextTurnDPOSInfo) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := n.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (n *NextTurnDPOSInfo) Serialize(w io.Writer, version byte) error {
	err := n.SerializeUnsigned(w, version)
	if err != nil {
		return err
	}

	return nil
}

func (n *NextTurnDPOSInfo) SerializeUnsigned(w io.Writer, version byte) error {

	if err := common.WriteUint32(w, n.WorkingHeight); err != nil {
		return err
	}
	if err := common.WriteVarUint(w, uint64(len(n.CRPublickeys))); err != nil {
		return err
	}

	for _, v := range n.CRPublickeys {
		if err := common.WriteVarBytes(w, v); err != nil {
			return err
		}
	}

	if err := common.WriteVarUint(w, uint64(len(n.DPOSPublicKeys))); err != nil {
		return err
	}

	for _, v := range n.DPOSPublicKeys {
		if err := common.WriteVarBytes(w, v); err != nil {
			return err
		}
	}
	return nil
}

func (n *NextTurnDPOSInfo) Deserialize(r io.Reader, version byte) error {
	err := n.DeserializeUnsigned(r, version)
	if err != nil {
		return err
	}
	return nil
}

func (n *NextTurnDPOSInfo) DeserializeUnsigned(r io.Reader, version byte) error {
	var err error
	var len uint64

	var workingHeight uint32
	if workingHeight, err = common.ReadUint32(r); err != nil {
		return err
	}
	n.WorkingHeight = workingHeight

	if len, err = common.ReadVarUint(r, 0); err != nil {
		return err
	}

	n.CRPublickeys = make([][]byte, 0, len)
	for i := uint64(0); i < len; i++ {
		var CRPublickey []byte
		if CRPublickey, err = common.ReadVarBytes(r, crypto.COMPRESSEDLEN,
			"cr public key"); err != nil {
			return err
		}
		n.CRPublickeys = append(n.CRPublickeys, CRPublickey)
	}

	if len, err = common.ReadVarUint(r, 0); err != nil {
		return err
	}

	n.DPOSPublicKeys = make([][]byte, 0, len)
	for i := uint64(0); i < len; i++ {
		var DPOSPublicKey []byte
		if DPOSPublicKey, err = common.ReadVarBytes(r, crypto.COMPRESSEDLEN,
			"dpos public key"); err != nil {
			return err
		}
		n.DPOSPublicKeys = append(n.DPOSPublicKeys, DPOSPublicKey)
	}
	return nil
}

func (n *NextTurnDPOSInfo) Hash() common.Uint256 {
	if n.hash == nil {
		buf := new(bytes.Buffer)
		n.SerializeUnsigned(buf, NextTurnDPOSInfoVersion)
		hash := common.Hash(buf.Bytes())
		n.hash = &hash
	}
	return *n.hash
}
