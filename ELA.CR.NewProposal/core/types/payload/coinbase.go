// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package payload

import (
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

const (
	// MaxPayloadDataSize is the maximum allowed length of payload data.
	MaxPayloadDataSize = 1024 * 1024 // 1MB
)

const CoinBaseVersion byte = 0x04

type CoinBase struct {
	Content []byte
}

func (a *CoinBase) Data(version byte) []byte {
	return a.Content
}

func (a *CoinBase) Serialize(w io.Writer, version byte) error {
	return common.WriteVarBytes(w, a.Content)
}

func (a *CoinBase) Deserialize(r io.Reader, version byte) error {
	temp, err := common.ReadVarBytes(r, MaxPayloadDataSize,
		"payload coinbase data")
	a.Content = temp
	return err
}
