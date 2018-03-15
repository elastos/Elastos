package message

import (
	"bytes"
	"encoding/binary"

	. "Elastos.ELA/bloom"
	. "Elastos.ELA/common"
	"Elastos.ELA/common/serialization"
	"Elastos.ELA/core/ledger"
)

type MerkleBlock struct {
	Header
	BlockHeader  ledger.Blockdata
	Transactions uint32
	Hashes       []*Uint256
	Flags        []byte
}

// NewMerkleBlock returns a new *MerkleBlock
func NewMerkleBlockMsg(block *ledger.Block, filter *Filter) ([]byte, error) {
	NumTx := uint32(len(block.Transactions))
	mBlock := MBlock{
		NumTx:       NumTx,
		AllHashes:   make([]*Uint256, 0, NumTx),
		MatchedBits: make([]byte, 0, NumTx),
	}

	// Find and keep track of any transactions that match the filter.
	for _, tx := range block.Transactions {
		if filter.MatchTxAndUpdate(tx) {
			mBlock.MatchedBits = append(mBlock.MatchedBits, 0x01)
		} else {
			mBlock.MatchedBits = append(mBlock.MatchedBits, 0x00)
		}
		mBlock.AllHashes = append(mBlock.AllHashes, tx.Hash())
	}

	// Calculate the number of merkle branches (height) in the tree.
	height := uint32(0)
	for mBlock.CalcTreeWidth(height) > 1 {
		height++
	}

	// Build the depth-first partial merkle tree.
	mBlock.TraverseAndBuild(height, 0)

	// Create and return the merkle block.
	merkleBlock := MerkleBlock{
		BlockHeader:  *block.Blockdata,
		Transactions: mBlock.NumTx,
		Hashes:       make([]*Uint256, 0, len(mBlock.FinalHashes)),
		Flags:        make([]byte, (len(mBlock.Bits)+7)/8),
	}
	for _, hash := range mBlock.FinalHashes {
		merkleBlock.AddTxHash(hash)
	}
	for i := uint32(0); i < uint32(len(mBlock.Bits)); i++ {
		merkleBlock.Flags[i/8] |= mBlock.Bits[i] << (i % 8)
	}

	body, err := merkleBlock.Serialize()
	if err != nil {
		return nil, err
	}

	return BuildMessage("merkleblock", body)
}

// AddTxHash adds a new transaction hash to the message.
func (mb *MerkleBlock) AddTxHash(hash *Uint256) error {
	mb.Hashes = append(mb.Hashes, hash)
	return nil
}

func (mb *MerkleBlock) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	mb.BlockHeader.Serialize(buf)

	err := serialization.WriteUint32(buf, mb.Transactions)
	if err != nil {
		return nil, err
	}

	for _, hash := range mb.Hashes {
		_, err := hash.Serialize(buf)
		if err != nil {
			return nil, err
		}
	}

	err = serialization.WriteVarBytes(buf, mb.Flags)

	return buf.Bytes(), nil
}

func (mb *MerkleBlock) Deserialize(msg []byte) error {
	buf := bytes.NewReader(msg)
	err := binary.Read(buf, binary.LittleEndian, &mb.Header)
	if err != nil {
		return err
	}

	err = mb.BlockHeader.Deserialize(buf)
	if err != nil {
		return err
	}

	mb.Transactions, err = serialization.ReadUint32(buf)
	if err != nil {
		return err
	}

	for i := uint32(0); i < mb.Transactions; i++ {
		var txId Uint256
		err := txId.Deserialize(buf)
		if err != nil {
			return err
		}
		mb.Hashes = append(mb.Hashes, &txId)
	}

	mb.Flags, err = serialization.ReadVarBytes(buf)
	if err != nil {
		return err
	}

	return nil
}
