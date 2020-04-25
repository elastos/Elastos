// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package types

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

const (
	InvalidBlockSize int = -1
)

// TxLoc holds locator data for the offset and length of where a transaction is
// located within a MsgBlock data buffer.
type TxLoc struct {
	TxStart int
	TxLen   int
}

type Block struct {
	Header
	Transactions []*Transaction
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

// DeserializeTxLoc decodes r in the same manner Deserialize does, but it takes
// a byte buffer instead of a generic reader and returns a slice containing the
// start and length of each transaction within the raw data that is being
// deserialized.
func (b *Block) DeserializeTxLoc(r *bytes.Buffer) ([]TxLoc, error) {
	fullLen := r.Len()

	if err := b.Header.Deserialize(r); err != nil {
		return nil, err
	}

	txCount, err := common.ReadUint32(r)
	if err != nil {
		return nil, errors.New("transactions count deserialize failed")
	}

	// Deserialize each transaction while keeping track of its location
	// within the byte stream.
	b.Transactions = make([]*Transaction, 0, txCount)
	txLocs := make([]TxLoc, txCount)
	for i := uint32(0); i < txCount; i++ {
		txLocs[i].TxStart = fullLen - r.Len()
		tx := Transaction{}
		err := tx.Deserialize(r)
		if err != nil {
			return nil, err
		}
		b.Transactions = append(b.Transactions, &tx)
		txLocs[i].TxLen = (fullLen - r.Len()) - txLocs[i].TxStart
	}

	return txLocs, nil
}

// TxLoc returns the offsets and lengths of each transaction in a raw block.
// It is used to allow fast indexing into transactions within the raw byte
// stream.
func (b *Block) TxLoc() ([]TxLoc, error) {
	buf := new(bytes.Buffer)
	if err := b.Serialize(buf); err != nil {
		return nil, err
	}

	var block Block
	txLocs, err := block.DeserializeTxLoc(buf)
	if err != nil {
		return nil, err
	}
	return txLocs, err
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

// SerializeSizeStripped returns the number of bytes it would take to serialize
// the block, excluding any witness data (if any).
func (b *Block) SerializeSizeStripped() int {
	// todo add cache for size according to btcd
	return b.GetSize()
}
