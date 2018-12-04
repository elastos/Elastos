package msg

import (
	"fmt"
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

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

func (msg *MerkleBlock) Serialize(writer io.Writer) error {
	err := msg.Header.Serialize(writer)
	if err != nil {
		return err
	}

	return common.WriteElements(writer,
		msg.Transactions,
		uint32(len(msg.Hashes)),
		msg.Hashes, msg.Flags)
}

func (msg *MerkleBlock) Deserialize(reader io.Reader) error {
	err := msg.Header.Deserialize(reader)
	if err != nil {
		return err
	}

	msg.Transactions, err = common.ReadUint32(reader)
	if err != nil {
		return err
	}

	count, err := common.ReadUint32(reader)
	if err != nil {
		return err
	}
	if count > MaxTxPerBlock {
		return fmt.Errorf("MerkleBlock.Deserialize too many transaction"+
			" hashes for message [count %v, max %v]", count, MaxTxPerBlock)
	}

	msg.Hashes = make([]*common.Uint256, 0, count)
	for i := uint32(0); i < count; i++ {
		var hash common.Uint256
		if err := hash.Deserialize(reader); err != nil {
			return err
		}
		msg.Hashes = append(msg.Hashes, &hash)
	}

	msg.Flags, err = common.ReadVarBytes(reader)
	return err
}
