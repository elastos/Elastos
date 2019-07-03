package state

import (
	"bytes"
	"crypto/rand"
	rand2 "math/rand"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types/payload"

	"github.com/stretchr/testify/assert"
)

func TestCandidate_Deserialize(t *testing.T) {
	candidate1 := randomCandidate()

	buf := new(bytes.Buffer)
	candidate1.Serialize(buf)

	candidate2 := &Candidate{}
	candidate2.Deserialize(buf)

	assert.True(t, candidateEqual(candidate1, candidate2))
}

func candidateEqual(first *Candidate, second *Candidate) bool {
	if !bytes.Equal(first.info.Code, second.info.Code) ||
		!first.info.DID.IsEqual(second.info.DID) ||
		first.info.NickName != second.info.NickName ||
		first.info.Url != second.info.Url ||
		first.info.Location != second.info.Location {
		return false
	}

	return first.state == second.state && first.votes == second.votes
}

func randomCandidate() *Candidate {
	return &Candidate{
		info: payload.CRInfo{
			Code:     randomBytes(34),
			DID:      *randomUint168(),
			NickName: randomString(),
			Url:      randomString(),
			Location: rand2.Uint64(),
		},
		state: CandidateState(rand2.Uint32()),
		votes: common.Fixed64(rand2.Int63()),
	}
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

func randomUint168() *common.Uint168 {
	randBytes := make([]byte, 21)
	rand.Read(randBytes)
	result, _ := common.Uint168FromBytes(randBytes)

	return result
}
