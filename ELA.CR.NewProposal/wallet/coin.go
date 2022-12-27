// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package wallet

import (
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
)

type Coin struct {
	TxVersion types.TransactionVersion
	Output    *types.Output
	Height    uint32
}

func (coin *Coin) Serialize(w io.Writer) error {
	if err := common.WriteUint8(w, uint8(coin.TxVersion)); err != nil {
		return err
	}
	if err := coin.Output.Serialize(w, coin.TxVersion); err != nil {
		return err
	}

	return common.WriteUint32(w, coin.Height)
}

func (coin *Coin) Deserialize(r io.Reader) error {
	txVersion, err := common.ReadUint8(r)
	if err != nil {
		return err
	}
	coin.TxVersion = types.TransactionVersion(txVersion)
	coin.Output = new(types.Output)
	if err := coin.Output.Deserialize(r, types.TransactionVersion(txVersion)); err != nil {
		return err
	}

	height, err := common.ReadUint32(r)
	if err != nil {
		return err
	}
	coin.Height = height

	return nil
}
