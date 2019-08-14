// Copyright (c) 2017-2019 Elastos Foundation
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
	InvalidBlockSize int    = -1
)

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
	buf  := new(bytes.Buffer)
	b.Serialize(buf)

	return len(buf.Bytes())
}