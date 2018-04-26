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
	if err := b.Header.Serialize(w); err != nil {
		return errors.New("Block header serialize failed, " + err.Error())
	}
	if err := common.WriteUint32(w, uint32(len(b.Transactions))); err != nil {
		return errors.New("Block item transactions count serialize failed.")
	}
	for _, transaction := range b.Transactions {
		if err := transaction.Serialize(w); err != nil {
			return errors.New("Block item transaction serialize failed, " + err.Error())
		}
	}
	return nil
}

func (b *Block) Deserialize(r io.Reader) error {
	if err := b.Header.Deserialize(r); err != nil {
		return errors.New("Block header deserialize failed, " + err.Error())
	}

	//Transactions
	count, err := common.ReadUint32(r)
	if err != nil {
		return errors.New("Block item transactions count deserialize failed.")
	}
	for i := uint32(0); i < count; i++ {
		transaction := new(Transaction)
		if err := transaction.Deserialize(r); err != nil {
			return errors.New("Block item transaction deserialize failed, " + err.Error())
		}
		b.Transactions = append(b.Transactions, transaction)
	}

	return nil
}

func (b *Block) Trim(w io.Writer) error {
	if err := b.Header.Serialize(w); err != nil {
		return errors.New("Block header serialize failed, " + err.Error())
	}
	if err := common.WriteUint32(w, uint32(len(b.Transactions))); err != nil {
		return errors.New("Block item transactions length serialize failed.")
	}
	for _, transaction := range b.Transactions {
		hash := transaction.Hash()
		if err := hash.Serialize(w); err != nil {
			return errors.New("Block item transaction hash serialize failed, " + err.Error())
		}
	}
	return nil
}

func (b *Block) FromTrimmedData(r io.Reader) error {
	if err := b.Header.Deserialize(r); err != nil {
		return errors.New("Block header deserialize failed, " + err.Error())
	}

	//Transactions
	count, err := common.ReadUint32(r)
	if err != nil {
		return errors.New("Block item transactions count deserialize failed.")
	}
	for i := uint32(0); i < count; i++ {
		var hash common.Uint256
		if err := hash.Deserialize(r); err != nil {
			return errors.New("Block item transaction hash deserialize failed, " + err.Error())
		}
		b.Transactions = append(b.Transactions, NewTrimmedTx(hash))
	}

	return nil
}

func (b *Block) GetSize() int {
	buf := new(bytes.Buffer)
	if err := b.Serialize(buf); err != nil {
		return InvalidBlockSize
	}

	return buf.Len()
}

func (b *Block) Hash() common.Uint256 {
	return b.Header.Hash()
}
