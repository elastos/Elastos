// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package types

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

type UTXO struct {
	TxID  common.Uint256
	Index uint16
	Value common.Fixed64
}

func (u *UTXO) Serialize(w io.Writer) error {
	if err := u.TxID.Serialize(w); err != nil {
		return err
	}
	if err := common.WriteUint16(w, u.Index); err != nil {
		return err
	}
	if err := u.Value.Serialize(w); err != nil {
		return err
	}
	return nil
}

func (u *UTXO) Deserialize(r io.Reader) error {
	if err := u.TxID.Deserialize(r); err != nil {
		return err
	}
	index, err := common.ReadUint16(r)
	if err != nil {
		return err
	}
	u.Index = index
	if err := u.Value.Deserialize(r); err != nil {
		return err
	}
	return nil
}
func (u *UTXO) Hash() common.Uint256 {
	buf := new(bytes.Buffer)
	u.Serialize(buf)
	return common.Uint256(common.Sha256D(buf.Bytes()))
}
