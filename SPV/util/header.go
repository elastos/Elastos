package util

import (
	"bytes"
	"fmt"
	"math/big"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/elanet/pact"
)

// Header is a data structure stored in database.
type Header struct {
	// The origin header of the block
	BlockHeader

	Height uint32

	// MerkleProof for transactions packed in this block
	NumTxs uint32
	Hashes []*common.Uint256
	Flags  []byte

	// The total work from the genesis block to this
	// current block
	TotalWork *big.Int
}

func (h *Header) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := h.BlockHeader.Serialize(buf)
	if err != nil {
		return nil, err
	}

	err = common.WriteUint32(buf, h.Height)
	if err != nil {
		return nil, err
	}

	err = common.WriteUint32(buf, h.NumTxs)
	if err != nil {
		return nil, err
	}

	err = common.WriteVarUint(buf, uint64(len(h.Hashes)))
	if err != nil {
		return nil, err
	}

	for _, hash := range h.Hashes {
		if err := hash.Serialize(buf); err != nil {
			return nil, err
		}
	}

	err = common.WriteVarBytes(buf, h.Flags)
	if err != nil {
		return nil, err
	}

	biBytes := h.TotalWork.Bytes()
	pad := make([]byte, 32-len(biBytes))
	serializedBI := append(pad, biBytes...)
	buf.Write(serializedBI)
	return buf.Bytes(), nil
}

func (h *Header) Deserialize(b []byte) error {
	r := bytes.NewReader(b)
	err := h.BlockHeader.Deserialize(r)
	if err != nil {
		return err
	}

	h.Height, err = common.ReadUint32(r)
	if err != nil {
		return err
	}

	h.NumTxs, err = common.ReadUint32(r)
	if err != nil {
		return err
	}

	count, err := common.ReadVarUint(r, 0)
	if err != nil {
		return err
	}
	if count > pact.MaxTxPerBlock {
		str := fmt.Sprintf("too many transactions to fit into a block "+
			"[count %d, max %d]", count, pact.MaxTxPerBlock)
		return common.FuncError("Header.Deserialize", str)
	}

	hashes := make([]common.Uint256, count)
	h.Hashes = make([]*common.Uint256, 0, count)
	for i := uint64(0); i < count; i++ {
		hash := &hashes[i]
		if err := hash.Deserialize(r); err != nil {
			return err
		}
		h.Hashes = append(h.Hashes, hash)
	}

	h.Flags, err = common.ReadVarBytes(r, pact.MaxTxPerBlock,
		"header merkle proof flags")
	if err != nil {
		return err
	}

	biBytes := make([]byte, 32)
	_, err = r.Read(biBytes)
	if err != nil {
		return err
	}
	h.TotalWork = new(big.Int)
	h.TotalWork.SetBytes(biBytes)

	return nil
}
