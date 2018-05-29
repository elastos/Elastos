package core

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA.SideChain/log"
	"github.com/elastos/Elastos.ELA.Utility/common"
)

const (
	BlockVersion     uint32 = 0
	GenesisNonce     uint32 = 2083236893
	InvalidBlockSize int    = -1
)

type Block struct {
	Header
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

	//Transactions
	len, err := common.ReadUint32(r)
	if err != nil {
		return err
	}

	for i := uint32(0); i < len; i++ {
		transaction := new(Transaction)
		transaction.Deserialize(r)
		b.Transactions = append(b.Transactions, transaction)
	}

	return nil
}

func (b *Block) Trim(w io.Writer) error {
	if err := b.Header.Serialize(w); err != nil {
		log.Error("Trim block serialize header failed:", err)
		return err
	}

	if err := common.WriteUint32(w, uint32(len(b.Transactions))); err != nil {
		log.Error("Trim block write txs len failed:", err)
		return errors.New("Block item Transactions length serialization failed.")
	}

	for _, transaction := range b.Transactions {
		hash := transaction.Hash()
		if err := hash.Serialize(w); err != nil {
			log.Error("Trim block serialize tx hash failed:", err)
			return err
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
	var buffer bytes.Buffer
	if err := b.Serialize(&buffer); err != nil {
		return InvalidBlockSize
	}

	return buffer.Len()
}

func (b *Block) Hash() common.Uint256 {
	return b.Header.Hash()
}
