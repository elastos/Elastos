// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package state

import (
	"bytes"
	"math/rand"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"

	"github.com/stretchr/testify/assert"
)

func TestRewardData_Deserialize(t *testing.T) {
	originData := randomRewardData()

	buf := new(bytes.Buffer)
	assert.NoError(t, originData.Serialize(buf))

	cmpData := NewRewardData()
	assert.NoError(t, cmpData.Deserialize(buf))

	assert.True(t, rewardEqual(originData, cmpData))
}

func TestStateKeyFrame_Deserialize(t *testing.T) {
	originFrame := randomStateKeyFrame()

	buf := new(bytes.Buffer)
	assert.NoError(t, originFrame.Serialize(buf))

	cmpData := &StateKeyFrame{}
	assert.NoError(t, cmpData.Deserialize(buf))

	assert.True(t, stateKeyFrameEqual(originFrame, cmpData))
}

func TestCheckPoint_Deserialize(t *testing.T) {
	originCheckPoint := generateCheckPoint(rand.Uint32())

	buf := new(bytes.Buffer)
	assert.NoError(t, originCheckPoint.Serialize(buf))

	cmpData := &CheckPoint{}
	assert.NoError(t, cmpData.Deserialize(buf))

	assert.True(t, checkPointsEqual(originCheckPoint, cmpData))
}

func checkPointsEqual(first *CheckPoint, second *CheckPoint) bool {
	if first.Height != second.Height || first.DutyIndex != second.DutyIndex ||
		first.CurrentReward.TotalVotesInRound !=
			second.CurrentReward.TotalVotesInRound ||
		second.NextReward.TotalVotesInRound !=
			second.NextReward.TotalVotesInRound {
		return false
	}

	if !arrayEqual(first.CurrentArbitrators, second.CurrentArbitrators) ||
		!arrayEqual(first.CurrentCandidates, second.CurrentCandidates) ||
		!arrayEqual(first.NextArbitrators, second.NextArbitrators) ||
		!arrayEqual(first.NextCandidates, second.NextCandidates) {
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

func generateCheckPoint(height uint32) *CheckPoint {
	result := &CheckPoint{
		Height:             height,
		DutyIndex:          int(rand.Uint32()),
		NextArbitrators:    []ArbiterMember{},
		NextCandidates:     []ArbiterMember{},
		CurrentCandidates:  []ArbiterMember{},
		CurrentArbitrators: []ArbiterMember{},
		CurrentReward:      *NewRewardData(),
		NextReward:         *NewRewardData(),
		StateKeyFrame:      *randomStateKeyFrame(),
	}
	result.CurrentReward.TotalVotesInRound = common.Fixed64(rand.Uint64())
	result.NextReward.TotalVotesInRound = common.Fixed64(rand.Uint64())

	for i := 0; i < 5; i++ {
		ar, _ := NewOriginArbiter(Origin, randomFakePK())
		result.CurrentArbitrators = append(result.CurrentArbitrators, ar)
		ar, _ = NewOriginArbiter(Origin, randomFakePK())
		result.CurrentCandidates = append(result.CurrentCandidates, ar)
		ar, _ = NewOriginArbiter(Origin, randomFakePK())
		result.NextArbitrators = append(result.NextArbitrators, ar)
		ar, _ = NewOriginArbiter(Origin, randomFakePK())
		result.NextCandidates = append(result.NextCandidates, ar)

		result.CurrentReward.OwnerVotesInRound[*randomProgramHash()] =
			common.Fixed64(rand.Uint64())

		result.NextReward.OwnerVotesInRound[*randomProgramHash()] =
			common.Fixed64(rand.Uint64())
	}

	return result
}

func stateKeyFrameEqual(first *StateKeyFrame, second *StateKeyFrame) bool {
	if len(first.NodeOwnerKeys) != len(second.NodeOwnerKeys) ||
		len(first.PendingProducers) != len(second.PendingProducers) ||
		len(first.ActivityProducers) != len(second.ActivityProducers) ||
		len(first.InactiveProducers) != len(second.InactiveProducers) ||
		len(first.CanceledProducers) != len(second.CanceledProducers) ||
		len(first.IllegalProducers) != len(second.IllegalProducers) ||
		len(first.PendingCanceledProducers) != len(second.PendingCanceledProducers) ||
		len(first.Votes) != len(second.Votes) ||
		len(first.DepositOutputs) != len(second.DepositOutputs) ||
		len(first.Nicknames) != len(second.Nicknames) ||
		len(first.SpecialTxHashes) != len(second.SpecialTxHashes) ||
		len(first.PreBlockArbiters) != len(second.PreBlockArbiters) ||
		len(first.ProducerDepositMap) != len(second.ProducerDepositMap) ||
		len(first.EmergencyInactiveArbiters) != len(second.EmergencyInactiveArbiters) {
		return false
	}

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

	for k, vf := range first.PendingCanceledProducers {
		vs, ok := second.PendingCanceledProducers[k]
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

	for k := range first.DepositOutputs {
		_, ok := second.DepositOutputs[k]
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

	for k := range first.ProducerDepositMap {
		_, ok := second.ProducerDepositMap[k]
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

func randomStateKeyFrame() *StateKeyFrame {
	result := &StateKeyFrame{
		NodeOwnerKeys:             make(map[string]string),
		PendingProducers:          make(map[string]*Producer),
		ActivityProducers:         make(map[string]*Producer),
		InactiveProducers:         make(map[string]*Producer),
		CanceledProducers:         make(map[string]*Producer),
		IllegalProducers:          make(map[string]*Producer),
		PendingCanceledProducers:  make(map[string]*Producer),
		Votes:                     make(map[string]struct{}),
		DepositOutputs:            make(map[string]common.Fixed64),
		Nicknames:                 make(map[string]struct{}),
		SpecialTxHashes:           make(map[common.Uint256]struct{}),
		PreBlockArbiters:          make(map[string]struct{}),
		ProducerDepositMap:        make(map[common.Uint168]struct{}),
		EmergencyInactiveArbiters: make(map[string]struct{}),
		VersionStartHeight:        rand.Uint32(),
		VersionEndHeight:          rand.Uint32(),
	}

	for i := 0; i < 5; i++ {
		result.NodeOwnerKeys[randomString()] = randomString()
		result.PendingProducers[randomString()] = randomProducer()
		result.ActivityProducers[randomString()] = randomProducer()
		result.InactiveProducers[randomString()] = randomProducer()
		result.CanceledProducers[randomString()] = randomProducer()
		result.IllegalProducers[randomString()] = randomProducer()
		result.PendingCanceledProducers[randomString()] = randomProducer()
		result.Votes[randomString()] = struct{}{}
		result.DepositOutputs[randomString()] = common.Fixed64(rand.Uint64())
		result.Nicknames[randomString()] = struct{}{}
		result.SpecialTxHashes[*randomHash()] = struct{}{}
		result.PreBlockArbiters[randomString()] = struct{}{}
		result.ProducerDepositMap[*randomProgramHash()] = struct{}{}
		result.EmergencyInactiveArbiters[randomString()] = struct{}{}
	}
	return result
}

func producerEqual(first *Producer, second *Producer) bool {
	if first.state != second.state ||
		first.registerHeight != second.registerHeight ||
		first.cancelHeight != second.cancelHeight ||
		first.inactiveCountingHeight != second.inactiveCountingHeight ||
		first.inactiveSince != second.inactiveSince ||
		first.activateRequestHeight != second.activateRequestHeight ||
		first.illegalHeight != second.illegalHeight ||
		first.penalty != second.penalty ||
		first.votes != second.votes {
		return false
	}

	return producerInfoEqual(&first.info, &second.info)
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

func rewardEqual(first *RewardData, second *RewardData) bool {
	if first.TotalVotesInRound != second.TotalVotesInRound {
		return false
	}

	return votesMapEqual(first.OwnerVotesInRound, second.OwnerVotesInRound)
}

func randomRewardData() *RewardData {
	result := NewRewardData()

	for i := 0; i < 5; i++ {
		result.OwnerVotesInRound[*randomProgramHash()] =
			common.Fixed64(rand.Uint64())
	}

	return result
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
						{randomFakePK(), 0},
					},
				},
			},
		},
	}
}

func randomHash() *common.Uint256 {
	a := make([]byte, 32)
	rand.Read(a)
	hash, _ := common.Uint256FromBytes(a)
	return hash
}

func randomProgramHash() *common.Uint168 {
	a := make([]byte, 21)
	rand.Read(a)
	hash, _ := common.Uint168FromBytes(a)
	return hash
}

func randomString() string {
	a := make([]byte, 20)
	rand.Read(a)
	return common.BytesToHexString(a)
}

func randomBytes(len int) []byte {
	a := make([]byte, len)
	rand.Read(a)
	return a
}

func randomProducer() *Producer {
	return &Producer{
		info: payload.ProducerInfo{
			OwnerPublicKey: randomFakePK(),
			NodePublicKey:  randomFakePK(),
			NickName:       randomString(),
			Url:            randomString(),
			Location:       rand.Uint64(),
			NetAddress:     randomString(),
			Signature:      randomBytes(64),
		},
		state:                  ProducerState(rand.Uint32()),
		registerHeight:         rand.Uint32(),
		cancelHeight:           rand.Uint32(),
		inactiveCountingHeight: rand.Uint32(),
		inactiveSince:          rand.Uint32(),
		activateRequestHeight:  rand.Uint32(),
		illegalHeight:          rand.Uint32(),
		penalty:                common.Fixed64(rand.Uint64()),
		votes:                  common.Fixed64(rand.Uint64()),
	}
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

func arrayEqual(first []ArbiterMember, second []ArbiterMember) bool {
	if len(first) != len(second) {
		return false
	}

	for _, vf := range first {
		found := false
		for _, vs := range second {
			if arbiterMemberEqual(vf, vs) {
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

func arbiterMemberEqual(first ArbiterMember, second ArbiterMember) bool {
	if bytes.Equal(first.GetNodePublicKey(), second.GetNodePublicKey()) &&
		bytes.Equal(first.GetOwnerPublicKey(), second.GetOwnerPublicKey()) &&
		first.GetType() == second.GetType() &&
		first.GetOwnerProgramHash().IsEqual(second.GetOwnerProgramHash()) {
		return true
	}

	return false
}
