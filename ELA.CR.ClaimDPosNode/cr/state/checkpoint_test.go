package state

import (
	"bytes"
	"testing"

	"github.com/elastos/Elastos.ELA/common"

	"github.com/stretchr/testify/assert"
)

func TestKeyFrame_Deserialize(t *testing.T) {
	frame := randomKeyFrame(5)

	buf := new(bytes.Buffer)
	frame.Serialize(buf)

	frame2 := &KeyFrame{}
	frame2.Deserialize(buf)

	assert.True(t, keyFrameEqual(frame, frame2))
}

func TestKeyFrame_Snapshot(t *testing.T) {
	frame := randomKeyFrame(5)
	frame2 := frame.Snapshot()
	assert.True(t, keyFrameEqual(frame, frame2))
}

func keyFrameEqual(first *KeyFrame, second *KeyFrame) bool {
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

func randomKeyFrame(size int) *KeyFrame {
	frame := NewKeyFrame()

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
