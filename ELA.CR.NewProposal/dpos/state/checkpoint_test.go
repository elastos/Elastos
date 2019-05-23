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

func stateKeyFrameEqual(first *StateKeyFrame, second *StateKeyFrame) bool {

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

func randomStateKeyFrame() *StateKeyFrame {
	result := &StateKeyFrame{
		NodeOwnerKeys:             make(map[string]string),
		PendingProducers:          make(map[string]*Producer),
		ActivityProducers:         make(map[string]*Producer),
		InactiveProducers:         make(map[string]*Producer),
		CanceledProducers:         make(map[string]*Producer),
		IllegalProducers:          make(map[string]*Producer),
		PendingCanceledProducers:  make(map[string]*Producer),
		Votes:                     make(map[string]*types.Output),
		Nicknames:                 make(map[string]struct{}),
		SpecialTxHashes:           make(map[common.Uint256]struct{}),
		PreBlockArbiters:          make(map[string]struct{}),
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
		result.Votes[randomString()] = randomVotes()
		result.Nicknames[randomString()] = struct{}{}
		result.SpecialTxHashes[*randomHash()] = struct{}{}
		result.PreBlockArbiters[randomString()] = struct{}{}
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

	if !hashesEqual(first.OwnerProgramHashes, second.OwnerProgramHashes) ||
		!hashesEqual(first.CandidateOwnerProgramHashes,
			second.CandidateOwnerProgramHashes) {
		return false
	}

	return votesMapEqual(first.OwnerVotesInRound, second.OwnerVotesInRound)
}

func randomRewardData() *RewardData {
	result := NewRewardData()

	for i := 0; i < 5; i++ {
		result.OwnerProgramHashes = append(result.OwnerProgramHashes,
			randomProgramHash())
		result.CandidateOwnerProgramHashes = append(
			result.CandidateOwnerProgramHashes, randomProgramHash())
		result.OwnerVotesInRound[*randomProgramHash()] = common.Fixed64(rand.Uint64())
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
					Candidates: [][]byte{
						randomFakePK(),
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
