package msg

import (
	"fmt"
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

// maxFlagsPerMerkleBlock is the maximum number of flag bytes that could
// possibly fit into a merkle block.  Since each transaction is represented by
// a single bit, this is the max number of transactions per block divided by
// 8 bits per byte.  Then an extra one to cover partials.
const maxFlagsPerMerkleBlock = MaxTxPerBlock / 8

// Ensure MerkleBlock implement p2p.Message interface.
var _ p2p.Message = (*MerkleBlock)(nil)

type MerkleBlock struct {
	Header       common.Serializable
	Transactions uint32
	Hashes       []*common.Uint256
	Flags        []byte
}

func NewMerkleBlock(header common.Serializable) *MerkleBlock {
	return &MerkleBlock{Header: header}
}

func (msg *MerkleBlock) CMD() string {
	return p2p.CmdMerkleBlock
}

func (msg *MerkleBlock) MaxLength() uint32 {
	return MaxBlockSize
}

func (msg *MerkleBlock) Serialize(w io.Writer) error {
	// Read num transaction hashes and limit to max.
	numHashes := len(msg.Hashes)
	if numHashes > MaxTxPerBlock {
		str := fmt.Sprintf("too many transaction hashes for message "+
			"[count %v, max %v]", numHashes, MaxTxPerBlock)
		return common.FuncError("MerkleBlock.Serialize", str)
	}
	numFlagBytes := len(msg.Flags)
	if numFlagBytes > maxFlagsPerMerkleBlock {
		str := fmt.Sprintf("too many flag bytes for message [count %v, "+
			"max %v]", numFlagBytes, maxFlagsPerMerkleBlock)
		return common.FuncError("MerkleBlock.Serialize", str)
	}

	err := msg.Header.Serialize(w)
	if err != nil {
		return err
	}

	err = common.WriteElements(w, msg.Transactions, uint32(numHashes))
	if err != nil {
		return err
	}

	for _, hash := range msg.Hashes {
		if err := hash.Serialize(w); err != nil {
			return err
		}
	}

	return common.WriteVarBytes(w, msg.Flags)
}

func (msg *MerkleBlock) Deserialize(r io.Reader) error {
	err := msg.Header.Deserialize(r)
	if err != nil {
		return err
	}

	var numHashes uint32
	err = common.ReadElements(r, &msg.Transactions, &numHashes)
	if err != nil {
		return err
	}

	if numHashes > MaxTxPerBlock {
		return fmt.Errorf("MerkleBlock.Deserialize too many transaction"+
			" hashes for message [count %v, max %v]", numHashes, MaxTxPerBlock)
	}

	hashes := make([]common.Uint256, numHashes)
	msg.Hashes = make([]*common.Uint256, 0, numHashes)
	for i := uint32(0); i < numHashes; i++ {
		hash := &hashes[i]
		if err := hash.Deserialize(r); err != nil {
			return err
		}
		msg.Hashes = append(msg.Hashes, hash)
	}

	msg.Flags, err = common.ReadVarBytes(r, maxFlagsPerMerkleBlock,
		"merkle block flags size")
	return err
}
