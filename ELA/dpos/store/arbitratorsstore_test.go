// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package store

import (
	"bytes"
	"math/rand"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/dpos/log"
	"github.com/elastos/Elastos.ELA/dpos/state"
	"github.com/elastos/Elastos.ELA/utils/test"

	"github.com/stretchr/testify/assert"
)

var arbitratorsStore *DposStore

func TestArbitratorsStore_Open(t *testing.T) {
	log.Init(0, 20, 100)

	store, err := NewDposStore(test.DataPath, &config.DefaultParams)
	if err != nil {
		t.Error("open database failed:", err.Error())
	}

	store.StartEventRecord()
	arbitratorsStore = store
}

func TestArbitratorsStore_GetCheckPoint(t *testing.T) {
	firstPoint := generateCheckPoint(10)
	arbitratorsStore.SaveArbitersState(firstPoint)

	// < 10
	actual, err := arbitratorsStore.GetCheckPoint(10 - 1)
	assert.Error(t, err, "can't find check point")

	// 10
	actual, err = arbitratorsStore.GetCheckPoint(10)
	assert.Error(t, err, "can't find check point")

	// > 10 && < 10 + CheckPointInterval
	actual, err = arbitratorsStore.GetCheckPoint(11)
	assert.Error(t, err, "can't find check point")

	// 10 + CheckPointInterval
	actual, err = arbitratorsStore.GetCheckPoint(10 + state.CheckPointInterval)
	assert.NoError(t, err)
	assert.True(t, checkPointsEqual(firstPoint, actual))

	// > 10 + CheckPointInterval
	actual, err = arbitratorsStore.GetCheckPoint(11 + state.CheckPointInterval)
	assert.NoError(t, err)
	assert.True(t, checkPointsEqual(firstPoint, actual))

	secondPoint := generateCheckPoint(20)
	arbitratorsStore.SaveArbitersState(secondPoint)

	// > 10 + CheckPointInterval && < 20 + CheckPointInterval
	actual, err = arbitratorsStore.GetCheckPoint(11 + state.CheckPointInterval)
	assert.NoError(t, err)
	assert.True(t, checkPointsEqual(firstPoint, actual))

	// > 20 + CheckPointInterval
	actual, err = arbitratorsStore.GetCheckPoint(21 + state.CheckPointInterval)
	assert.NoError(t, err)
	assert.True(t, checkPointsEqual(secondPoint, actual))
}

func TestArbitratorsStore_Close(t *testing.T) {
	arbitratorsStore.deleteTable(ProposalEventTable)
	arbitratorsStore.deleteTable(ConsensusEventTable)
	arbitratorsStore.deleteTable(VoteEventTable)
	arbitratorsStore.deleteTable(ViewEventTable)
	arbitratorsStore.Close()
}

func checkPointsEqual(first *state.CheckPoint, second *state.CheckPoint) bool {
	if first.Height != second.Height || first.DutyIndex != second.DutyIndex ||
		first.CurrentReward.TotalVotesInRound !=
			second.CurrentReward.TotalVotesInRound ||
		second.NextReward.TotalVotesInRound !=
			second.NextReward.TotalVotesInRound {
		return false
	}

	if !arbitersEqual(first.CurrentArbitrators, second.CurrentArbitrators) ||
		!arbitersEqual(first.CurrentCandidates, second.CurrentCandidates) ||
		!arbitersEqual(first.NextArbitrators, second.NextArbitrators) ||
		!arbitersEqual(first.NextCandidates, second.NextCandidates) {
		return false
	}

	if !stateKeyFrameEqual(&first.StateKeyFrame, &second.StateKeyFrame) {
		return false
	}

	return votesMapEqual(first.CurrentReward.OwnerVotesInRound,
		second.CurrentReward.OwnerVotesInRound) &&
		votesMapEqual(first.NextReward.OwnerVotesInRound,
			second.NextReward.OwnerVotesInRound)
}

func generateCheckPoint(height uint32) *state.CheckPoint {
	result := &state.CheckPoint{
		Height:             height,
		DutyIndex:          int(rand.Uint32()),
		NextArbitrators:    []state.ArbiterMember{},
		NextCandidates:     []state.ArbiterMember{},
		CurrentCandidates:  []state.ArbiterMember{},
		CurrentArbitrators: []state.ArbiterMember{},
		CurrentReward:      *state.NewRewardData(),
		NextReward:         *state.NewRewardData(),
		StateKeyFrame:      *randomStateKeyFrame(),
	}
	result.CurrentReward.TotalVotesInRound = common.Fixed64(rand.Uint64())
	result.NextReward.TotalVotesInRound = common.Fixed64(rand.Uint64())

	for i := 0; i < 5; i++ {
		ar, _ := state.NewOriginArbiter(state.Origin, randomPkBytes())
		result.CurrentArbitrators = append(result.CurrentArbitrators, ar)
		ar, _ = state.NewOriginArbiter(state.Origin, randomPkBytes())
		result.CurrentCandidates = append(result.CurrentCandidates, ar)
		ar, _ = state.NewOriginArbiter(state.Origin, randomPkBytes())
		result.NextArbitrators = append(result.NextArbitrators, ar)
		ar, _ = state.NewOriginArbiter(state.Origin, randomPkBytes())
		result.NextCandidates = append(result.NextCandidates, ar)

		result.CurrentReward.OwnerVotesInRound[*randomProgramHash()] =
			common.Fixed64(rand.Uint64())

		result.NextReward.OwnerVotesInRound[*randomProgramHash()] =
			common.Fixed64(rand.Uint64())
	}

	return result
}

func randomStateKeyFrame() *state.StateKeyFrame {
	result := &state.StateKeyFrame{
		NodeOwnerKeys:             make(map[string]string),
		PendingProducers:          make(map[string]*state.Producer),
		ActivityProducers:         make(map[string]*state.Producer),
		InactiveProducers:         make(map[string]*state.Producer),
		CanceledProducers:         make(map[string]*state.Producer),
		IllegalProducers:          make(map[string]*state.Producer),
		PendingCanceledProducers:  make(map[string]*state.Producer),
		Votes:                     make(map[string]struct{}),
		DepositOutputs:            make(map[string]common.Fixed64),
		Nicknames:                 make(map[string]struct{}),
		SpecialTxHashes:           make(map[common.Uint256]struct{}),
		PreBlockArbiters:          make(map[string]struct{}),
		EmergencyInactiveArbiters: make(map[string]struct{}),
		VersionStartHeight:        rand.Uint32(),
		VersionEndHeight:          rand.Uint32(),
	}

	for i := 0; i < 5; i++ {
		result.NodeOwnerKeys[randomString()] = randomString()
		result.PendingProducers[randomString()] = &state.Producer{}
		result.ActivityProducers[randomString()] = &state.Producer{}
		result.InactiveProducers[randomString()] = &state.Producer{}
		result.CanceledProducers[randomString()] = &state.Producer{}
		result.IllegalProducers[randomString()] = &state.Producer{}
		result.PendingCanceledProducers[randomString()] = &state.Producer{}
		result.Votes[randomString()] = struct{}{}
		result.DepositOutputs[randomString()] = common.Fixed64(rand.Uint64())
		result.Nicknames[randomString()] = struct{}{}
		result.SpecialTxHashes[*randomHash()] = struct{}{}
		result.PreBlockArbiters[randomString()] = struct{}{}
		result.EmergencyInactiveArbiters[randomString()] = struct{}{}
	}
	return result
}

func randomString() string {
	a := make([]byte, 20)
	rand.Read(a)
	return common.BytesToHexString(a)
}

func votesMapEqual(first map[common.Uint168]common.Fixed64,
	second map[common.Uint168]common.Fixed64) bool {
	if len(first) != len(second) {
		return false
	}

	for k, vf := range first {
		if vs, ok := second[k]; !ok || vs != vf {
			return false
		}
	}
	return true
}

func arbitersEqual(first []state.ArbiterMember, second []state.ArbiterMember) bool {
	if len(first) != len(second) {
		return false
	}

	for _, vf := range first {
		found := false
		for _, vs := range second {
			if bytes.Equal(vf.GetNodePublicKey(), vs.GetNodePublicKey()) &&
				bytes.Equal(vf.GetOwnerPublicKey(), vs.GetOwnerPublicKey()) &&
				vf.GetType() == vs.GetType() &&
				vf.GetOwnerProgramHash().IsEqual(vs.GetOwnerProgramHash()) {
				found = true
				break
			}
		}
		if !found {
			return false
		}
	}
	return true
}

func hashesEqual(first []*common.Uint168, second []*common.Uint168) bool {
	if len(first) != len(second) {
		return false
	}

	for _, vf := range first {
		found := false
		for _, vs := range second {
			if vs.IsEqual(*vf) {
				found = true
				break
			}
		}
		if !found {
			return false
		}
	}
	return true
}

func randomFakePK() []byte {
	pk := make([]byte, 33)
	rand.Read(pk)
	return pk
}

func randomProgramHash() *common.Uint168 {
	a := make([]byte, 21)
	rand.Read(a)
	hash, _ := common.Uint168FromBytes(a)
	return hash
}

func randomHash() *common.Uint256 {
	a := make([]byte, 32)
	rand.Read(a)
	hash, _ := common.Uint256FromBytes(a)
	return hash
}

func randomVotes() *types.Output {
	return &types.Output{
		AssetID:     *randomHash(),
		Value:       common.Fixed64(rand.Uint64()),
		OutputLock:  rand.Uint32(),
		ProgramHash: *randomProgramHash(),
		Type:        types.OTVote,
		Payload: &outputpayload.VoteOutput{
			Version: byte(rand.Uint32()),
			Contents: []outputpayload.VoteContent{
				{
					VoteType: outputpayload.Delegate,
					CandidateVotes: []outputpayload.CandidateVotes{
						{
							Candidate: randomFakePK(),
							Votes:     common.Fixed64(rand.Uint64()),
						},
					},
				},
			},
		},
	}
}

func producerEqual(first *state.Producer, second *state.Producer) bool {
	if first.State() != second.State() ||
		first.RegisterHeight() != second.RegisterHeight() ||
		first.CancelHeight() != second.CancelHeight() ||
		first.InactiveSince() != second.InactiveSince() ||
		first.ActivateRequestHeight() != second.ActivateRequestHeight() ||
		first.IllegalHeight() != second.IllegalHeight() ||
		first.Penalty() != second.Penalty() ||
		first.Votes() != second.Votes() {
		return false
	}

	firstInfo := first.Info()
	secondInfo := second.Info()
	return producerInfoEqual(&firstInfo, &secondInfo)
}

func producerInfoEqual(first *payload.ProducerInfo,
	second *payload.ProducerInfo) bool {
	if first.NickName != second.NickName ||
		first.Url != second.Url ||
		first.Location != second.Location ||
		first.NetAddress != second.NetAddress {
		return false
	}

	return bytes.Equal(first.OwnerPublicKey, second.OwnerPublicKey) &&
		bytes.Equal(first.NodePublicKey, second.NodePublicKey) &&
		bytes.Equal(first.Signature, second.Signature)
}

func stateKeyFrameEqual(first *state.StateKeyFrame,
	second *state.StateKeyFrame) bool {

	for k, vf := range first.NodeOwnerKeys {
		vs, ok := second.NodeOwnerKeys[k]
		if !ok {
			return false
		}
		if vf != vs {
			return false
		}
	}

	for k, vf := range first.PendingProducers {
		vs, ok := second.PendingProducers[k]
		if !ok {
			return false
		}
		if !producerEqual(vf, vs) {
			return false
		}
	}

	for k, vf := range first.ActivityProducers {
		vs, ok := second.ActivityProducers[k]
		if !ok {
			return false
		}
		if !producerEqual(vf, vs) {
			return false
		}
	}

	for k, vf := range first.InactiveProducers {
		vs, ok := second.InactiveProducers[k]
		if !ok {
			return false
		}
		if !producerEqual(vf, vs) {
			return false
		}
	}

	for k, vf := range first.CanceledProducers {
		vs, ok := second.CanceledProducers[k]
		if !ok {
			return false
		}
		if !producerEqual(vf, vs) {
			return false
		}
	}

	for k, vf := range first.IllegalProducers {
		vs, ok := second.IllegalProducers[k]
		if !ok {
			return false
		}
		if !producerEqual(vf, vs) {
			return false
		}
	}

	for k := range first.Votes {
		_, ok := second.Votes[k]
		if !ok {
			return false
		}
	}

	for k := range first.Nicknames {
		_, ok := second.Nicknames[k]
		if !ok {
			return false
		}
	}

	for k := range first.SpecialTxHashes {
		_, ok := second.SpecialTxHashes[k]
		if !ok {
			return false
		}
	}

	for k := range first.PreBlockArbiters {
		_, ok := second.PreBlockArbiters[k]
		if !ok {
			return false
		}
	}

	for k := range first.EmergencyInactiveArbiters {
		_, ok := second.EmergencyInactiveArbiters[k]
		if !ok {
			return false
		}
	}

	return first.VersionStartHeight == second.VersionStartHeight &&
		first.VersionEndHeight == second.VersionEndHeight
}
