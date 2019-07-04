package state

import (
	"bytes"
	"math/rand"
	"testing"

	"github.com/elastos/Elastos.ELA/common"

	"github.com/stretchr/testify/assert"
)

func TestKeyFrame_Deserialize(t *testing.T) {
	frame := randomKeyFrame(5, rand.Uint32())

	buf := new(bytes.Buffer)
	frame.Serialize(buf)

	frame2 := &KeyFrame{}
	frame2.Deserialize(buf)

	assert.True(t, keyframeEqual(frame, frame2))
}

func TestKeyFrame_Snapshot(t *testing.T) {
	frame := randomKeyFrame(5, rand.Uint32())
	frame2 := frame.Snapshot()
	assert.True(t, keyframeEqual(frame, frame2))
}

func TestStateKeyFrame_Deserialize(t *testing.T) {
	frame := randomStateKeyFrame(5, true)

	buf := new(bytes.Buffer)
	frame.Serialize(buf)

	frame2 := &StateKeyFrame{}
	frame2.Deserialize(buf)

	assert.True(t, stateKeyframeEqual(frame, frame2))
}

func TestStateKeyFrame_Snapshot(t *testing.T) {
	frame := randomStateKeyFrame(5, true)
	frame2 := frame.Snapshot()
	assert.True(t, stateKeyframeEqual(frame, frame2))
}

func stateKeyframeEqual(first *StateKeyFrame, second *StateKeyFrame) bool {
	if len(first.Nicknames) != len(second.Nicknames) ||
		len(first.CodeDIDMap) != len(second.CodeDIDMap) {
		return false
	}

	for k := range first.Nicknames {
		if _, ok := second.Nicknames[k]; !ok {
			return false
		}
	}

	for k, v := range first.CodeDIDMap {
		v2, ok := second.CodeDIDMap[k]
		if !ok {
			return false
		}

		if !v.IsEqual(v2) {
			return false
		}
	}

	return candidatesMapEqual(first.PendingCandidates, second.PendingCandidates) &&
		candidatesMapEqual(first.ActivityCandidates, second.ActivityCandidates) &&
		candidatesMapEqual(first.CanceledCandidates, second.CanceledCandidates)
}

func candidatesMapEqual(first map[common.Uint168]*Candidate,
	second map[common.Uint168]*Candidate) bool {
	if len(first) != len(second) {
		return false
	}
	for k, v := range first {
		v2, ok := second[k]
		if !ok {
			return false
		}

		if !candidateEqual(v, v2) {
			return false
		}
	}
	return true
}

func keyframeEqual(first *KeyFrame, second *KeyFrame) bool {
	if first.LastCommitteeHeight != second.LastCommitteeHeight ||
		len(first.Members) != len(second.Members) {
		return false
	}

	for i := 0; i < len(first.Members); i++ {
		if !crMemberEqual(first.Members[i], second.Members[i]) {
			return false
		}
	}
	return true
}

func randomKeyFrame(size int, commitHeight uint32) *KeyFrame {
	frame := &KeyFrame{
		LastCommitteeHeight: commitHeight,
	}

	frame.Members = make([]*CRMember, 0, size)
	for i := 0; i < size; i++ {
		frame.Members = append(frame.Members, randomCRMember())
	}

	return frame
}

func crMemberEqual(first *CRMember, second *CRMember) bool {
	return crInfoEqual(&first.Info, &second.Info) &&
		first.ImpeachmentVotes == second.ImpeachmentVotes
}

func randomCRMember() *CRMember {
	return &CRMember{
		Info:             *randomCRInfo(),
		ImpeachmentVotes: common.Fixed64(rand.Uint64()),
	}
}

func randomStateKeyFrame(size int, hasPending bool) *StateKeyFrame {
	frame := NewStateKeyFrame()

	for i := 0; i < size; i++ {
		did := *randomUint168()
		candidate := randomCandidate()
		nickname := randomString()
		code := randomBytes(34)
		candidate.info.Code = code
		candidate.info.NickName = nickname
		candidate.info.DID = did
		frame.CodeDIDMap[common.BytesToHexString(code)] = did
		frame.PendingCandidates[did] = candidate
		frame.Nicknames[nickname] = struct{}{}
	}
	if hasPending {
		for i := 0; i < size; i++ {
			did := *randomUint168()
			candidate := randomCandidate()
			nickname := randomString()
			code := randomBytes(34)
			candidate.info.Code = code
			candidate.info.NickName = nickname
			candidate.info.DID = did
			frame.CodeDIDMap[common.BytesToHexString(code)] = did
			frame.ActivityCandidates[did] = candidate
			frame.Nicknames[nickname] = struct{}{}
		}
	}
	for i := 0; i < size; i++ {
		did := *randomUint168()
		candidate := randomCandidate()
		nickname := randomString()
		code := randomBytes(34)
		candidate.info.Code = code
		candidate.info.NickName = nickname
		candidate.info.DID = did
		if i%2 == 0 {
			candidate.state = Canceled
		} else {
			candidate.state = Returned
		}
		frame.CodeDIDMap[common.BytesToHexString(code)] = did
		frame.CanceledCandidates[did] = candidate
		frame.Nicknames[nickname] = struct{}{}
	}
	return frame
}
