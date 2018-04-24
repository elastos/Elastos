package core

import (
	"bytes"
	"errors"
	"io"

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

func (b *Block) CMD() string {
	return "block"
}

func (b *Block) Serialize(w io.Writer) error {
	b.Header.Serialize(w)
	err := common.WriteUint32(w, uint32(len(b.Transactions)))
	if err != nil {
		return errors.New("Block item Transactions length serialization failed.")
	}

	for _, transaction := range b.Transactions {
		transaction.Serialize(w)
	}
	return nil
}

func (b *Block) Deserialize(r io.Reader) error {
	err := b.Header.Deserialize(r)
	if err != nil {
		return err
	}

	//Transactions
	var i uint32
	len, err := common.ReadUint32(r)
	if err != nil {
		return err
	}
	var tharray []common.Uint256
	for i = 0; i < len; i++ {
		transaction := new(Transaction)
		transaction.Deserialize(r)
		b.Transactions = append(b.Transactions, transaction)
		tharray = append(tharray, transaction.Hash())
	}

	return nil
}

func (b *Block) Trim(w io.Writer) error {
	b.Header.Serialize(w)
	err := common.WriteUint32(w, uint32(len(b.Transactions)))
	if err != nil {
		return errors.New("Block item Transactions length serialization failed.")
	}
	for _, transaction := range b.Transactions {
		temp := *transaction
		hash := temp.Hash()
		hash.Serialize(w)
	}
	return nil
}

func (b *Block) FromTrimmedData(r io.Reader) error {
	err := b.Header.Deserialize(r)
	if err != nil {
		return err
	}

	//Transactions
	var i uint32
	Len, err := common.ReadUint32(r)
	if err != nil {
		return err
	}
	var txhash common.Uint256
	var tharray []common.Uint256
	for i = 0; i < Len; i++ {
		txhash.Deserialize(r)
		b.Transactions = append(b.Transactions, NewTrimmedTx(txhash))
		tharray = append(tharray, txhash)
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
