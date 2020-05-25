// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package auxpow

import (
	"bytes"
	"io"

	. "github.com/elastos/Elastos.ELA/common"
)

type BtcHeader struct {
	Version    uint32
	Previous   Uint256
	MerkleRoot Uint256
	Timestamp  uint32
	Bits       uint32
	Nonce      uint32
}

func (bh *BtcHeader) Serialize(w io.Writer) error {
	err := WriteUint32(w, bh.Version)
	if err != nil {
		return err
	}

	err = bh.Previous.Serialize(w)
	if err != nil {
		return err
	}

	err = bh.MerkleRoot.Serialize(w)
	if err != nil {
		return err
	}

	err = WriteUint32(w, bh.Timestamp)
	if err != nil {
		return err
	}

	err = WriteUint32(w, bh.Bits)
	if err != nil {
		return err
	}

	err = WriteUint32(w, bh.Nonce)
	if err != nil {
		return err
	}

	return nil
}

func (bh *BtcHeader) Deserialize(r io.Reader) error {
	var err error
	//Version
	bh.Version, err = ReadUint32(r)
	if err != nil {
		return err
	}

	//PrevBlockHash
	err = bh.Previous.Deserialize(r)
	if err != nil {
		return err
	}

	//TransactionsRoot
	err = bh.MerkleRoot.Deserialize(r)
	if err != nil {
		return err
	}

	//Timestamp
	bh.Timestamp, err = ReadUint32(r)
	if err != nil {
		return err
	}

	//Bits
	bh.Bits, err = ReadUint32(r)
	if err != nil {
		return err
	}

	//Nonce
	bh.Nonce, err = ReadUint32(r)
	if err != nil {
		return err
	}

	return nil
}

func (bh *BtcHeader) Hash() Uint256 {
	buf := new(bytes.Buffer)
	bh.Serialize(buf)
	return Sha256D(buf.Bytes())
}
