// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package state

import (
	"bytes"
	"errors"
	"io"
	"math"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/checkpoint"
	"github.com/elastos/Elastos.ELA/core/types"
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
	StateKeyFrame
	Height                     uint32
	DutyIndex                  int
	CurrentArbitrators         []ArbiterMember
	NextArbitrators            []ArbiterMember
	NextCandidates             []ArbiterMember
	CurrentCandidates          []ArbiterMember
	CurrentReward              RewardData
	NextReward                 RewardData
	crcArbiters                map[common.Uint168]ArbiterMember
	crcChangedHeight           uint32
	accumulativeReward         common.Fixed64
	finalRoundChange           common.Fixed64
	clearingHeight             uint32
	arbitersRoundReward        map[common.Uint168]common.Fixed64
	illegalBlocksPayloadHashes map[common.Uint256]interface{}

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
		Height:             c.Height,
		DutyIndex:          c.arbitrators.dutyIndex,
		CurrentCandidates:  make([]ArbiterMember, 0),
		NextArbitrators:    make([]ArbiterMember, 0),
		NextCandidates:     make([]ArbiterMember, 0),
		CurrentReward:      *NewRewardData(),
		NextReward:         *NewRewardData(),
		CurrentArbitrators: c.arbitrators.currentArbitrators,
		StateKeyFrame:      *c.arbitrators.StateKeyFrame.snapshot(),
	}
	point.CurrentArbitrators = copyByteList(c.arbitrators.currentArbitrators)
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
	return checkpoint.Medium
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

// Serialize write data to writer
func (c *CheckPoint) Serialize(w io.Writer) (err error) {
	if err = common.WriteUint32(w, c.Height); err != nil {
		return
	}

	if err = common.WriteUint32(w, uint32(c.DutyIndex)); err != nil {
		return
	}

	if err = c.writeArbiters(w, c.CurrentArbitrators); err != nil {
		return
	}

	if err = c.writeArbiters(w, c.CurrentCandidates); err != nil {
		return
	}

	if err = c.writeArbiters(w, c.NextArbitrators); err != nil {
		return
	}

	if err = c.writeArbiters(w, c.NextCandidates); err != nil {
		return
	}

	if err = c.CurrentReward.Serialize(w); err != nil {
		return
	}

	if err = c.NextReward.Serialize(w); err != nil {
		return
	}

	if err = c.serializeCRCArbitersMap(w, c.crcArbiters); err != nil {
		return
	}

	if err = common.WriteUint32(w, c.crcChangedHeight); err != nil {
		return
	}

	if err = c.accumulativeReward.Serialize(w); err != nil {
		return
	}

	if err = c.finalRoundChange.Serialize(w); err != nil {
		return
	}

	if err = common.WriteUint32(w, c.clearingHeight); err != nil {
		return
	}

	if err = c.serializeRoundRewardMap(w, c.arbitersRoundReward); err != nil {
		return
	}

	if err = c.serializeIllegalPayloadHashesMap(w, c.illegalBlocksPayloadHashes); err != nil {
		return
	}

	return c.StateKeyFrame.Serialize(w)
}

func (c *CheckPoint) serializeCRCArbitersMap(w io.Writer,
	rmap map[common.Uint168]ArbiterMember) (err error) {
	if err = common.WriteVarUint(w, uint64(len(rmap))); err != nil {
		return
	}
	for k, v := range rmap {
		if err = k.Serialize(w); err != nil {
			return
		}

		if err = common.WriteUint8(w, uint8(v.GetType())); err != nil {
			return
		}

		if err = v.Serialize(w); err != nil {
			return
		}
	}
	return
}

func (c *CheckPoint) serializeRoundRewardMap(w io.Writer,
	rmap map[common.Uint168]common.Fixed64) (err error) {
	if err = common.WriteVarUint(w, uint64(len(rmap))); err != nil {
		return
	}
	for k, v := range rmap {
		if err = k.Serialize(w); err != nil {
			return
		}

		if err = v.Serialize(w); err != nil {
			return
		}
	}
	return
}

func (c *CheckPoint) serializeIllegalPayloadHashesMap(w io.Writer,
	rmap map[common.Uint256]interface{}) (err error) {
	if err = common.WriteVarUint(w, uint64(len(rmap))); err != nil {
		return
	}
	for k, _ := range rmap {
		if err = k.Serialize(w); err != nil {
			return
		}
	}
	return
}

// Deserialize read data to reader
func (c *CheckPoint) Deserialize(r io.Reader) (err error) {
	if c.Height, err = common.ReadUint32(r); err != nil {
		return
	}

	var dutyIndex uint32
	if dutyIndex, err = common.ReadUint32(r); err != nil {
		return
	}
	c.DutyIndex = int(dutyIndex)

	if c.CurrentArbitrators, err = c.readArbiters(r); err != nil {
		return
	}

	if c.CurrentCandidates, err = c.readArbiters(r); err != nil {
		return
	}

	if c.NextArbitrators, err = c.readArbiters(r); err != nil {
		return
	}

	if c.NextCandidates, err = c.readArbiters(r); err != nil {
		return
	}

	if err = c.CurrentReward.Deserialize(r); err != nil {
		return
	}

	if err = c.NextReward.Deserialize(r); err != nil {
		return
	}

	if c.crcArbiters, err = c.deserializeCRCArbitersMap(r); err != nil {
		return
	}

	if c.crcChangedHeight, err = common.ReadUint32(r); err != nil {
		return
	}

	if err = c.accumulativeReward.Deserialize(r); err != nil {
		return
	}

	if err = c.finalRoundChange.Deserialize(r); err != nil {
		return
	}

	if c.clearingHeight, err = common.ReadUint32(r); err != nil {
		return
	}

	if c.arbitersRoundReward, err = c.deserializeRoundRewardMap(r); err != nil {
		return
	}

	c.illegalBlocksPayloadHashes, err = c.deserializeIllegalPayloadHashes(r)
	if err != nil {
		return
	}

	return c.StateKeyFrame.Deserialize(r)
}


func (c *CheckPoint) deserializeCRCArbitersMap(r io.Reader) (
	rmap map[common.Uint168]ArbiterMember, err error) {

	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	rmap = make(map[common.Uint168]ArbiterMember)
	for i := uint64(0); i < count; i++ {
		var k common.Uint168
		if err = k.Deserialize(r); err != nil {
			return
		}

		var arbiterType uint8
		if arbiterType, err = common.ReadUint8(r); err != nil {
			return
		}
		var am ArbiterMember
		switch ArbiterType(arbiterType) {
		case Origin:
			am = &originArbiter{}
		case DPoS:
			am = &dposArbiter{
				arType: DPoS,
			}
		case CROrigin:
			am = &dposArbiter{
				arType: CROrigin,
			}
		case CRC:
			am = &crcArbiter{}
		default:
			err = errors.New("invalid arbiter type")
			return
		}
		if err = am.Deserialize(r); err != nil {
			return
		}

		rmap[k] = am
	}
	return
}


func (c *CheckPoint) deserializeIllegalPayloadHashes(
	r io.Reader) (hmap map[common.Uint256]interface{}, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	hmap = make(map[common.Uint256]interface{})
	for i := uint64(0); i < count; i++ {
		var k common.Uint256
		if err = k.Deserialize(r); err != nil {
			return
		}
		hmap[k] = nil
	}
	return
}

func (c *CheckPoint) deserializeRoundRewardMap(
	r io.Reader) (rmap map[common.Uint168]common.Fixed64, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	rmap = make(map[common.Uint168]common.Fixed64)
	for i := uint64(0); i < count; i++ {
		var k common.Uint168
		if err = k.Deserialize(r); err != nil {
			return
		}
		reward := common.Fixed64(0)
		if err = reward.Deserialize(r); err != nil {
			return
		}
		rmap[k] = reward
	}
	return
}

func (c *CheckPoint) writeArbiters(w io.Writer,
	arbiters []ArbiterMember) error {
	if err := common.WriteVarUint(w, uint64(len(arbiters))); err != nil {
		return err
	}

	for _, ar := range arbiters {
		if err := SerializeArbiterMember(ar, w); err != nil {
			return err
		}
	}
	return nil
}

func (c *CheckPoint) readArbiters(r io.Reader) ([]ArbiterMember, error) {
	count, err := common.ReadVarUint(r, 0)
	if err != nil {
		return nil, err
	}

	arbiters := make([]ArbiterMember, 0, count)
	for i := uint64(0); i < count; i++ {
		arbiter, err := ArbiterMemberFromReader(r)
		if err != nil {
			return nil, err
		}
		arbiters = append(arbiters, arbiter)
	}
	return arbiters, nil
}

func (c *CheckPoint) initFromArbitrators(ar *arbitrators) {
	c.CurrentCandidates = ar.currentCandidates
	c.NextArbitrators = ar.nextArbitrators
	c.NextCandidates = ar.nextCandidates
	c.CurrentReward = ar.CurrentReward
	c.NextReward = ar.NextReward
	c.CurrentArbitrators = ar.currentArbitrators
	c.StateKeyFrame = *ar.State.StateKeyFrame
}

func NewCheckpoint(ar *arbitrators) *CheckPoint {
	cp := &CheckPoint{
		arbitrators: ar,
	}
	cp.initFromArbitrators(ar)
	return cp
}
