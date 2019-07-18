// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package state

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA/core/checkpoint"
	"github.com/elastos/Elastos.ELA/core/types"
)

const (
	checkpointKey = "cr"

	checkpointExtension = ".ccp"
)

// Checkpoint hold all CR related states to recover from scratch.
type Checkpoint struct {
	KeyFrame
	StateKeyFrame

	height    uint32
	committee *Committee
}

func (c *Checkpoint) OnBlockSaved(block *types.DposBlock) {
	if block.Height <= c.committee.state.history.Height() {
		return
	}
	c.committee.ProcessBlock(block.Block, block.Confirm)
}

func (c *Checkpoint) OnRollbackTo(height uint32) error {
	return c.committee.RollbackTo(height)
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
	result := &Checkpoint{}
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
	return c.committee.params.CRDutyPeriod
}

func (c *Checkpoint) EffectivePeriod() uint32 {
	return c.committee.params.CRDutyPeriod
}

func (c *Checkpoint) DataExtension() string {
	return checkpointExtension
}

func (c *Checkpoint) Generator() func(buf []byte) checkpoint.ICheckPoint {
	return func(buf []byte) checkpoint.ICheckPoint {
		stream := new(bytes.Buffer)
		stream.Write(buf)

		result := &Checkpoint{}
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
	c.committee.Recover(c)
}

func (c *Checkpoint) StartHeight() uint32 {
	return c.committee.params.CRVotingStartHeight
}

func (c *Checkpoint) Serialize(w io.Writer) (err error) {
	if err = c.KeyFrame.Serialize(w); err != nil {
		return
	}

	return c.StateKeyFrame.Serialize(w)
}

func (c *Checkpoint) Deserialize(r io.Reader) (err error) {
	if err = c.KeyFrame.Deserialize(r); err != nil {
		return
	}

	return c.StateKeyFrame.Deserialize(r)
}

func NewCheckpoint(committee *Committee) *Checkpoint {
	cp := &Checkpoint{
		StateKeyFrame: *NewStateKeyFrame(),
		KeyFrame:      *NewKeyFrame(),
		height:        uint32(0),
		committee:     committee,
	}
	committee.getCheckpoint = func(height uint32) *Checkpoint {
		if height > cp.GetHeight() {
			return cp
		}
		return nil
	}

	return cp
}
