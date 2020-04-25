// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package payload

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
)

const InactiveArbitratorsVersion byte = 0x00

type InactiveArbitrators struct {
	Sponsor     []byte
	Arbitrators [][]byte
	BlockHeight uint32

	hash *common.Uint256
}

func (i *InactiveArbitrators) Type() IllegalDataType {
	return InactiveArbitrator
}

func (i *InactiveArbitrators) GetBlockHeight() uint32 {
	return i.BlockHeight
}

func (i *InactiveArbitrators) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := i.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (i *InactiveArbitrators) SerializeUnsigned(w io.Writer, version byte) error {
	if err := common.WriteVarBytes(w, i.Sponsor); err != nil {
		return err
	}

	if err := common.WriteUint32(w, i.BlockHeight); err != nil {
		return err
	}

	return nil
}

func (i *InactiveArbitrators) Serialize(w io.Writer, version byte) error {
	if err := i.SerializeUnsigned(w, version); err != nil {
		return err
	}

	if err := common.WriteVarUint(w, uint64(len(i.Arbitrators))); err != nil {
		return err
	}
	for _, v := range i.Arbitrators {
		if err := common.WriteVarBytes(w, v); err != nil {
			return err
		}
	}

	return nil
}

func (i *InactiveArbitrators) DeserializeUnsigned(r io.Reader,
	version byte) (err error) {
	if i.Sponsor, err = common.ReadVarBytes(r, crypto.NegativeBigLength,
		"public key"); err != nil {
		return err
	}

	if i.BlockHeight, err = common.ReadUint32(r); err != nil {
		return err
	}

	return err
}

func (i *InactiveArbitrators) Deserialize(r io.Reader,
	version byte) (err error) {
	if err = i.DeserializeUnsigned(r, version); err != nil {
		return err
	}

	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return err
	}
	i.Arbitrators = make([][]byte, count)
	for u := uint64(0); u < count; u++ {
		if i.Arbitrators[u], err = common.ReadVarBytes(r,
			crypto.NegativeBigLength, "public key"); err != nil {
			return err
		}
	}

	return nil
}

func (i *InactiveArbitrators) Hash() common.Uint256 {
	if i.hash == nil {
		buf := new(bytes.Buffer)
		i.SerializeUnsigned(buf, InactiveArbitratorsVersion)
		hash := common.Hash(buf.Bytes())
		i.hash = &hash
	}
	return *i.hash
}
