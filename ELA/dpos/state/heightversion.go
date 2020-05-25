// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package state

import (
	"errors"
	"math"

	"github.com/elastos/Elastos.ELA/common"
)

// 0 - H1
func (a *arbitrators) getNormalArbitratorsDescV0() ([]ArbiterMember, error) {
	arbitersByte := make([]ArbiterMember, 0)
	for _, arbiter := range a.State.chainParams.OriginArbiters {
		arbiterByte, err := common.HexStringToBytes(arbiter)
		if err != nil {
			return nil, err
		}
		ar, err := NewOriginArbiter(Origin, arbiterByte)
		if err != nil {
			return nil, err
		}
		arbitersByte = append(arbitersByte, ar)
	}

	return arbitersByte, nil
}

// H1 - H2
func (a *arbitrators) getNormalArbitratorsDescV1() ([]ArbiterMember, error) {
	return nil, nil
}

// 0 - H1
func (a *arbitrators) getNextOnDutyArbitratorV0(height,
	offset uint32) ArbiterMember {
	arbitrators, _ := a.getNormalArbitratorsDescV0()
	index := (height - 1 + offset) % uint32(len(arbitrators))
	arbiter := arbitrators[index]
	return arbiter
}

func (a *arbitrators) distributeWithNormalArbitratorsV0(
	reward common.Fixed64) (map[common.Uint168]common.Fixed64, common.Fixed64, error) {
	if len(a.currentArbitrators) == 0 {
		return nil, 0, errors.New("not found arbiters when distributeWithNormalArbitratorsV0")
	}

	roundReward := map[common.Uint168]common.Fixed64{}
	totalBlockConfirmReward := float64(reward) * 0.25
	totalTopProducersReward := float64(reward) - totalBlockConfirmReward
	individualBlockConfirmReward := common.Fixed64(
		math.Floor(totalBlockConfirmReward / float64(len(a.currentArbitrators))))
	totalVotesInRound := a.CurrentReward.TotalVotesInRound
	if len(a.chainParams.CRCArbiters) == len(a.currentArbitrators) {
		roundReward[a.chainParams.CRCAddress] = reward
		return roundReward, reward, nil
	}
	rewardPerVote := totalTopProducersReward / float64(totalVotesInRound)

	roundReward[a.chainParams.CRCAddress] = 0
	realDPOSReward := common.Fixed64(0)
	for _, arbiter := range a.currentArbitrators {
		ownerHash := arbiter.GetOwnerProgramHash()
		votes := a.CurrentReward.OwnerVotesInRound[ownerHash]
		individualProducerReward := common.Fixed64(math.Floor(float64(
			votes) * rewardPerVote))
		r := individualBlockConfirmReward + individualProducerReward
		if _, ok := a.crcArbiters[ownerHash]; ok {
			r = individualBlockConfirmReward
			roundReward[a.chainParams.CRCAddress] += r
		} else {
			roundReward[ownerHash] = r
		}

		realDPOSReward += r
	}
	for _, candiate := range a.currentCandidates {
		ownerHash := candiate.GetOwnerProgramHash()
		votes := a.CurrentReward.OwnerVotesInRound[ownerHash]
		individualProducerReward := common.Fixed64(math.Floor(float64(
			votes) * rewardPerVote))
		roundReward[ownerHash] = individualProducerReward

		realDPOSReward += individualProducerReward
	}
	return roundReward, realDPOSReward, nil
}
