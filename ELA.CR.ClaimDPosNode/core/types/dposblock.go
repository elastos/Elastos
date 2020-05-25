// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package types

import (
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types/payload"
)

type DposBlock struct {
	*Block
	HaveConfirm bool
	Confirm     *payload.Confirm
}

func (b *DposBlock) Serialize(w io.Writer) error {
	if err := b.Block.Serialize(w); err != nil {
		return errors.New("Block serialize failed," + err.Error())
	}

	if err := common.WriteElement(w, b.HaveConfirm); err != nil {
		return errors.New("Confirm flag serialize failed," + err.Error())
	}

	if b.HaveConfirm {
		if err := b.Confirm.Serialize(w); err != nil {
			return errors.New("Confirm serialize failed," + err.Error())
		}
	}
	return nil
}

func (b *DposBlock) Deserialize(r io.Reader) error {
	b.Block = new(Block)
	if err := b.Block.Deserialize(r); err != nil {
		return errors.New("Block dserialize failed," + err.Error())
	}

	if err := common.ReadElement(r, &b.HaveConfirm); err != nil {
		return errors.New("Confirm flag dserialize failed," + err.Error())
	}

	if b.HaveConfirm {
		b.Confirm = new(payload.Confirm)
		if err := b.Confirm.Deserialize(r); err != nil {
			return errors.New("Confirm serialize failed," + err.Error())
		}
	}
	return nil
}
