// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package indexers

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/checkpoint"
	"github.com/elastos/Elastos.ELA/core/types"
)

const (
	// checkpointKey defines key of unspent txn checkpoint.
	checkpointKey = "utx"

	// checkpointExtension defines checkpoint file extension of unspent txn checkpoint.
	checkpointExtension = ".txcp"

	// checkpointHeight defines interval height between two neighbor check points.
	checkpointHeight = uint32(720)
)

// Checkpoint hold all unspent txn cache to recover from scratch.
type Checkpoint struct {
	height  uint32
	txCache *TxCache
}

func (c *Checkpoint) OnBlockSaved(block *types.DposBlock) {

}

func (c *Checkpoint) OnRollbackTo(height uint32) error {
	return nil
}

func (c *Checkpoint) Key() string {
	return checkpointKey
}

func (c *Checkpoint) Snapshot() checkpoint.ICheckPoint {
	buf := new(bytes.Buffer)
	if err := c.Serialize(buf); err != nil {
		c.LogError(err)
		return nil
	}
	result := &Checkpoint{
		height:  uint32(0),
		txCache: NewTxCache(),
	}
	if err := result.Deserialize(buf); err != nil {
		c.LogError(err)
		return nil
	}
	return result
}

func (c *Checkpoint) GetHeight() uint32 {
	return c.height
}

func (c *Checkpoint) SetHeight(height uint32) {
	c.height = height
}

func (c *Checkpoint) SavePeriod() uint32 {
	return checkpointHeight
}

func (c *Checkpoint) EffectivePeriod() uint32 {
	return checkpointHeight
}

func (c *Checkpoint) DataExtension() string {
	return checkpointExtension
}

func (c *Checkpoint) Generator() func(buf []byte) checkpoint.ICheckPoint {
	return func(buf []byte) checkpoint.ICheckPoint {
		stream := new(bytes.Buffer)
		stream.Write(buf)

		result := &Checkpoint{
			height:  uint32(0),
			txCache: NewTxCache(),
		}
		if err := result.Deserialize(stream); err != nil {
			c.LogError(err)
			return nil
		}
		return result
	}
}

func (c *Checkpoint) LogError(err error) {
	log.Warn(err)
}

func (c *Checkpoint) Priority() checkpoint.Priority {
	return checkpoint.MediumHigh
}

func (c *Checkpoint) OnInit() {

}

func (c *Checkpoint) StartHeight() uint32 {
	return 500000
}

func (c *Checkpoint) Serialize(w io.Writer) (err error) {
	err = common.WriteUint32(w, c.height)
	if err != nil {
		return
	}
	return c.txCache.Serialize(w)
}

func (c *Checkpoint) Deserialize(r io.Reader) (err error) {
	c.height, err = common.ReadUint32(r)
	if err != nil {
		return
	}
	return c.txCache.Deserialize(r)
}

func NewCheckpoint(unspentIndex *UnspentIndex) *Checkpoint {
	cp := &Checkpoint{
		height:  uint32(0),
		txCache: unspentIndex.txCache,
	}

	return cp
}
