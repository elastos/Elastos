// Copyright (c) 2017-2019 Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package state

import (
	"bytes"
	"io"
	"math"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/checkpoint"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/crypto"
)

const (
	// checkpointKey defines key of DPoS checkpoint.
	checkpointKey = "dpos"

	// checkpointExtension defines checkpoint file extension of DPoS checkpoint.
	checkpointExtension = ".dcp"

	// CheckPointInterval defines interval height between two neighbor check
	// points.
	CheckPointInterval = uint32(720)

	// checkpointEffectiveHeight defines the minimal height arbitrators obj
	// should scan to recover effective state.
	checkpointEffectiveHeight = uint32(720)
)

// CheckPoint defines all variables need record in database
type CheckPoint struct {
	KeyFrame
	StateKeyFrame
	Height            uint32
	DutyIndex         int
	NextArbitrators   [][]byte
	NextCandidates    [][]byte
	CurrentCandidates [][]byte
	CurrentReward     RewardData
	NextReward        RewardData

	arbitrators *arbitrators
}

func (c *CheckPoint) StartHeight() uint32 {
	return uint32(math.Min(float64(c.arbitrators.chainParams.VoteStartHeight),
		float64(c.arbitrators.chainParams.CRCOnlyDPOSHeight-
			c.arbitrators.chainParams.PreConnectOffset)))
}

func (c *CheckPoint) OnBlockSaved(block *types.DposBlock) {
	if block.Height <= c.GetHeight() {
		return
	}
	c.arbitrators.ProcessBlock(block.Block, block.Confirm)
}

func (c *CheckPoint) OnRollbackTo(height uint32) error {
	if height < c.StartHeight() {
		ar := &arbitrators{}
		if err := ar.initArbitrators(c.arbitrators.chainParams); err != nil {
			return err
		}
		c.initFromArbitrators(ar)
		c.arbitrators.RecoverFromCheckPoints(c)
		return nil
	}
	return c.arbitrators.RollbackTo(height)
}

func (c *CheckPoint) Key() string {
	return checkpointKey
}

func (c *CheckPoint) OnInit() {
	c.arbitrators.RecoverFromCheckPoints(c)
}

func (c *CheckPoint) Snapshot() checkpoint.ICheckPoint {
	point := &CheckPoint{
		Height:            c.Height,
		DutyIndex:         c.arbitrators.dutyIndex,
		CurrentCandidates: make([][]byte, 0),
		NextArbitrators:   make([][]byte, 0),
		NextCandidates:    make([][]byte, 0),
		CurrentReward:     *NewRewardData(),
		NextReward:        *NewRewardData(),
		KeyFrame: KeyFrame{
			CurrentArbitrators: c.arbitrators.CurrentArbitrators,
		},
		StateKeyFrame: *c.arbitrators.StateKeyFrame.snapshot(),
	}
	point.CurrentArbitrators = copyByteList(c.arbitrators.CurrentArbitrators)
	point.CurrentCandidates = copyByteList(c.arbitrators.currentCandidates)
	point.NextArbitrators = copyByteList(c.arbitrators.nextArbitrators)
	point.NextCandidates = copyByteList(c.arbitrators.nextCandidates)
	point.CurrentReward = *copyReward(&c.arbitrators.CurrentReward)
	point.NextReward = *copyReward(&c.arbitrators.NextReward)
	return point
}

func (c *CheckPoint) GetHeight() uint32 {
	return c.Height
}

func (c *CheckPoint) SetHeight(height uint32) {
	c.Height = height
}

func (c *CheckPoint) SavePeriod() uint32 {
	return CheckPointInterval
}

func (c *CheckPoint) EffectivePeriod() uint32 {
	return checkpointEffectiveHeight
}

func (c *CheckPoint) DataExtension() string {
	return checkpointExtension
}

func (c *CheckPoint) Priority() checkpoint.Priority {
	return checkpoint.MediumHigh
}

func (c *CheckPoint) Generator() func(buf []byte) checkpoint.ICheckPoint {
	return func(buf []byte) checkpoint.ICheckPoint {
		stream := new(bytes.Buffer)
		stream.Write(buf)

		result := &CheckPoint{}
		if err := result.Deserialize(stream); err != nil {
			c.LogError(err)
			return nil
		}
		return result
	}
}

func (c *CheckPoint) LogError(err error) {
	log.Warn("[CheckPoint] error: ", err.Error())
}

//Write data to writer
func (c *CheckPoint) Serialize(w io.Writer) (err error) {
	if err = common.WriteUint32(w, c.Height); err != nil {
		return
	}

	if err = common.WriteUint32(w, uint32(c.DutyIndex)); err != nil {
		return
	}

	if err = c.writeBytesArray(w, c.CurrentArbitrators); err != nil {
		return
	}

	if err = c.writeBytesArray(w, c.CurrentCandidates); err != nil {
		return
	}

	if err = c.writeBytesArray(w, c.NextArbitrators); err != nil {
		return
	}

	if err = c.writeBytesArray(w, c.NextCandidates); err != nil {
		return
	}

	if err = c.CurrentReward.Serialize(w); err != nil {
		return
	}

	if err = c.NextReward.Serialize(w); err != nil {
		return
	}

	return c.StateKeyFrame.Serialize(w)
}

//read data to reader
func (c *CheckPoint) Deserialize(r io.Reader) (err error) {
	if c.Height, err = common.ReadUint32(r); err != nil {
		return
	}

	var dutyIndex uint32
	if dutyIndex, err = common.ReadUint32(r); err != nil {
		return
	}
	c.DutyIndex = int(dutyIndex)

	if c.CurrentArbitrators, err = c.readBytesArray(r); err != nil {
		return
	}

	if c.CurrentCandidates, err = c.readBytesArray(r); err != nil {
		return
	}

	if c.NextArbitrators, err = c.readBytesArray(r); err != nil {
		return
	}

	if c.NextCandidates, err = c.readBytesArray(r); err != nil {
		return
	}

	if err = c.CurrentReward.Deserialize(r); err != nil {
		return
	}

	if err = c.NextReward.Deserialize(r); err != nil {
		return
	}

	return c.StateKeyFrame.Deserialize(r)
}

func (c *CheckPoint) writeBytesArray(w io.Writer, bytesArray [][]byte) error {
	if err := common.WriteVarUint(w, uint64(len(bytesArray))); err != nil {
		return err
	}

	for _, b := range bytesArray {
		if err := common.WriteVarBytes(w, b); err != nil {
			return err
		}
	}
	return nil
}

func (c *CheckPoint) readBytesArray(r io.Reader) ([][]byte, error) {
	count, err := common.ReadVarUint(r, 0)
	if err != nil {
		return nil, err
	}

	bytesArray := make([][]byte, 0, count)
	for i := uint64(0); i < count; i++ {
		arbiter, err := common.ReadVarBytes(r, crypto.NegativeBigLength, "arbiter")
		if err != nil {
			return nil, err
		}
		bytesArray = append(bytesArray, arbiter)
	}
	return bytesArray, nil
}

func (c *CheckPoint) initFromArbitrators(ar *arbitrators) {
	c.CurrentCandidates = ar.currentCandidates
	c.NextArbitrators = ar.nextArbitrators
	c.NextCandidates = ar.nextCandidates
	c.CurrentReward = ar.CurrentReward
	c.NextReward = ar.NextReward
	c.KeyFrame = KeyFrame{
		CurrentArbitrators: ar.CurrentArbitrators,
	}
	c.StateKeyFrame = *ar.State.StateKeyFrame
}

func NewCheckpoint(ar *arbitrators) *CheckPoint {
	cp := &CheckPoint{
		arbitrators: ar,
	}
	cp.initFromArbitrators(ar)
	return cp
}
