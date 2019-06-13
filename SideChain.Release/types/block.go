package types

import (
	"bytes"
	"errors"
	"fmt"
	"io"

	"github.com/elastos/Elastos.ELA/common"

	"github.com/elastos/Elastos.ELA.SideChain/interfaces"
)

const (
	// BlockVersion is the version of block.
	BlockVersion     uint32 = 0

	// GenesisNonce is the nonce of genesis block.
	GenesisNonce     uint32 = 3194347904

	// MaxBlockSize is the maximum size of a block.
	MaxBlockSize = 8000000

	// MaxTxPerBlock is the maximum transactions can be included in
	// a block.
	MaxTxPerBlock = 100000
)

type Block struct {
	interfaces.Header

	Transactions []*Transaction
}

func (b *Block) Serialize(w io.Writer) error {
	if err := b.Header.Serialize(w); err != nil {
		return err
	}

	if err := common.WriteUint32(w, uint32(len(b.Transactions))); err != nil {
		return errors.New("Block item Transactions length serialization failed.")
	}

	for _, transaction := range b.Transactions {
		transaction.Serialize(w)
	}
	return nil
}

func (b *Block) Deserialize(r io.Reader) error {
	if err := b.Header.Deserialize(r); err != nil {
		return err
	}

	txCount, err := common.ReadUint32(r)
	if err != nil {
		return err
	}

	// Prevent more transactions than could possibly fit into a block.
	// It would be possible to cause memory exhaustion and panics without
	// a sane upper bound on this count.
	if txCount > MaxTxPerBlock {
		str := fmt.Sprintf("too many transactions to fit into a block "+
			"[count %d, max %d]", txCount, MaxTxPerBlock)
		return common.FuncError("Inv.Deserialize", str)
	}

	// Deserialize each transaction while keeping track of its location
	// within the byte stream.
	transactions := make([]Transaction, txCount)
	b.Transactions = make([]*Transaction, 0, txCount)
	for i := uint32(0); i < txCount; i++ {
		tx := &transactions[i]
		if err := tx.Deserialize(r); err != nil {
			return err
		}
		b.Transactions = append(b.Transactions, tx)
	}

	return nil
}

func (b *Block) Trim(w io.Writer) error {
	if err := b.Header.Serialize(w); err != nil {
		return fmt.Errorf("Trim block serialize header failed:", err)
	}

	if err := common.WriteUint32(w, uint32(len(b.Transactions))); err != nil {
		return fmt.Errorf("Trim block write txs len failed:", err)
	}

	for _, transaction := range b.Transactions {
		hash := transaction.Hash()
		if err := hash.Serialize(w); err != nil {
			return fmt.Errorf("Trim block serialize tx hash failed:", err)
		}
	}

	return nil
}

func (b *Block) FromTrimmedData(r io.Reader) error {
	err := b.Header.Deserialize(r)
	if err != nil {
		return err
	}

	//Transactions
	len, err := common.ReadUint32(r)
	if err != nil {
		return err
	}

	var hash common.Uint256
	for i := uint32(0); i < len; i++ {
		hash.Deserialize(r)
		b.Transactions = append(b.Transactions, NewTrimmedTx(hash))
	}

	return nil
}

func (b *Block) GetSize() int {
	var buf bytes.Buffer
	if err := b.Serialize(&buf); err != nil {
		return -1
	}

	return buf.Len()
}

func (b *Block) Hash() common.Uint256 {
	return b.Header.Hash()
}

func NewBlock() *Block {
	b := &Block{
		Header: &Header{},
	}
	return b
}