package message

import (
	"bytes"
	"encoding/binary"

	. "Elastos.ELA/bloom"
	. "Elastos.ELA/common"
	"Elastos.ELA/common/serialize"
	"Elastos.ELA/core/ledger"
)

type MerkleBlock struct {
	Header
	BlockHeader  ledger.Header
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
		txHash := tx.Hash()
		mBlock.AllHashes = append(mBlock.AllHashes, &txHash)
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
		BlockHeader:  *block.Header,
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

	err := serialize.WriteUint32(buf, mb.Transactions)
	if err != nil {
		return nil, err
	}

	// Write hashes length
	err = serialize.WriteUint32(buf, uint32(len(mb.Hashes)))
	if err != nil {
		return nil, err
	}

	for _, hash := range mb.Hashes {
		err := hash.Serialize(buf)
		if err != nil {
			return nil, err
		}
	}

	err = serialize.WriteVarBytes(buf, mb.Flags)
	if err != nil {
		return nil, err
	}

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

	mb.Transactions, err = serialize.ReadUint32(buf)
	if err != nil {
		return err
	}

	hashes, err := serialize.ReadUint32(buf)
	if err != nil {
		return err
	}

	for i := uint32(0); i < hashes; i++ {
		var txId Uint256
		err := txId.Deserialize(buf)
		if err != nil {
			return err
		}
		mb.Hashes = append(mb.Hashes, &txId)
	}

	mb.Flags, err = serialize.ReadVarBytes(buf)
	if err != nil {
		return err
	}

	return nil
}
