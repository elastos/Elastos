// Copyright (c) 2017-2019 The Elastos Foundation
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
	Index uint32
	Value common.Fixed64
}

func (uu *UTXO) Serialize(w io.Writer) {
	uu.TxID.Serialize(w)
	common.WriteUint32(w, uu.Index)
	uu.Value.Serialize(w)
}

func (uu *UTXO) Deserialize(r io.Reader) error {
	uu.TxID.Deserialize(r)

	index, err := common.ReadUint32(r)
	uu.Index = uint32(index)
	if err != nil {
		return err
	}

	uu.Value.Deserialize(r)

	return nil
}
func (uu *UTXO) Hash() common.Uint256 {
	buf := new(bytes.Buffer)
	uu.Serialize(buf)
	return common.Uint256(common.Sha256D(buf.Bytes()))
}
