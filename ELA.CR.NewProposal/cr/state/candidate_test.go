// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

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
	return crInfoEqual(&first.info, &second.info) &&
		first.state == second.state && first.votes == second.votes &&
		first.registerHeight == second.registerHeight &&
		first.cancelHeight == second.cancelHeight &&
		first.depositHash.IsEqual(second.depositHash)
}

func crInfoEqual(first *payload.CRInfo, second *payload.CRInfo) bool {
	if !bytes.Equal(first.Code, second.Code) ||
		!first.CID.IsEqual(second.CID) ||
		first.NickName != second.NickName ||
		first.Url != second.Url ||
		first.Location != second.Location {
		return false
	}
	return true
}

func randomCRInfo() *payload.CRInfo {
	code := randomBytes(34)
	return &payload.CRInfo{
		Code:     code,
		CID:      *getCID(code),
		DID:      *getDID(code),
		NickName: randomString(),
		Url:      randomString(),
		Location: rand2.Uint64(),
	}
}

func randomCandidate() *Candidate {
	return &Candidate{
		info:           *randomCRInfo(),
		state:          CandidateState(rand2.Uint32()),
		votes:          common.Fixed64(rand2.Int63()),
		registerHeight: rand2.Uint32(),
		cancelHeight:   rand2.Uint32(),
		depositHash:    *randomUint168(),
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

func randomUint256() *common.Uint256 {
	randBytes := make([]byte, 32)
	rand.Read(randBytes)
	result, _ := common.Uint256FromBytes(randBytes)

	return result
}
