package util

import (
	"bytes"
	"math/big"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/core"
)

// Header is a data structure stored in database.
type Header struct {
	// The origin header of the block
	core.Header

	// MerkleProof for transactions packed in this block
	NumTxs uint32
	Hashes []*common.Uint256
	Flags  []byte

	// The total work from the genesis block to this
	// current block
	TotalWork *big.Int
}

func (sh *Header) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := sh.Header.Serialize(buf)
	if err != nil {
		return nil, err
	}

	err = common.WriteUint32(buf, sh.NumTxs)
	if err != nil {
		return nil, err
	}

	err = common.WriteVarUint(buf, uint64(len(sh.Hashes)))
	if err != nil {
		return nil, err
	}

	err = common.WriteElement(buf, sh.Hashes)
	if err != nil {
		return nil, err
	}

	err = common.WriteVarBytes(buf, sh.Flags)
	if err != nil {
		return nil, err
	}

	biBytes := sh.TotalWork.Bytes()
	pad := make([]byte, 32-len(biBytes))
	serializedBI := append(pad, biBytes...)
	buf.Write(serializedBI)
	return buf.Bytes(), nil
}

func (sh *Header) Deserialize(b []byte) error {
	r := bytes.NewReader(b)
	err := sh.Header.Deserialize(r)
	if err != nil {
		return err
	}

	sh.NumTxs, err = common.ReadUint32(r)
	if err != nil {
		return err
	}

	count, err := common.ReadVarUint(r, 0)
	if err != nil {
		return err
	}

	sh.Hashes = make([]*common.Uint256, count)
	err = common.ReadElement(r, &sh.Hashes)
	if err != nil {
		return err
	}

	sh.Flags, err = common.ReadVarBytes(r)
	if err != nil {
		return err
	}

	biBytes := make([]byte, 32)
	_, err = r.Read(biBytes)
	if err != nil {
		return err
	}
	sh.TotalWork = new(big.Int)
	sh.TotalWork.SetBytes(biBytes)

	return nil
}
