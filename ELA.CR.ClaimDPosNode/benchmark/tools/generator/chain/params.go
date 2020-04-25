// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package chain

import (
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

type GenerationMod byte

const (
	Fast    GenerationMod = 0x00
	Normal  GenerationMod = 0x01
	Minimal GenerationMod = 0x02
)

type GenerationParams struct {
	Mode GenerationMod

	PrepareStartHeight uint32
	RandomStartHeight  uint32

	InputsPerBlock uint32
	MaxRefersCount uint32
	MinRefersCount uint32
	AddressCount   uint32
}

func (p *GenerationParams) Serialize(w io.Writer) error {
	if err := common.WriteUint8(w, uint8(p.Mode)); err != nil {
		return err
	}

	return common.WriteElements(w,
		p.PrepareStartHeight,
		p.RandomStartHeight,
		p.InputsPerBlock,
		p.MaxRefersCount,
		p.MinRefersCount,
		p.AddressCount)
}

func (p *GenerationParams) Deserialize(r io.Reader) (err error) {
	var mode uint8
	if mode, err = common.ReadUint8(r); err != nil {
		return
	}
	p.Mode = GenerationMod(mode)

	return common.ReadElements(r,
		&p.PrepareStartHeight,
		&p.RandomStartHeight,
		&p.InputsPerBlock,
		&p.MaxRefersCount,
		&p.MinRefersCount,
		&p.AddressCount)
}
