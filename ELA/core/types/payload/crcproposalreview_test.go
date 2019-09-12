// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package payload

import (
	"bytes"
	"crypto/rand"
	mathRand "math/rand"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/stretchr/testify/assert"
)

func TestCRCProposalReview_Deserialize(t *testing.T) {
	proposalReview1 := randomCrcProposalReviewPayload()

	buf := new(bytes.Buffer)
	proposalReview1.Serialize(buf, CRCProposalReviewVersion)

	proposalReview2 := &CRCProposalReview{}
	proposalReview2.Deserialize(buf, CRCProposalReviewVersion)

	assert.True(t, crcProposalReviewPayloadEqual(proposalReview1, proposalReview2))
}

func crcProposalReviewPayloadEqual(payload1 *CRCProposalReview,
	payload2 *CRCProposalReview) bool {
	if !payload1.ProposalHash.IsEqual(payload2.ProposalHash) ||
		payload1.VoteResult != payload2.VoteResult ||
		!bytes.Equal(payload1.Code, payload2.Code) ||
		!bytes.Equal(payload1.Sign, payload2.Sign) {
		return false
	}

	return true
}

func randomCrcProposalReviewPayload() *CRCProposalReview {
	return &CRCProposalReview{
		ProposalHash: *randomUint256(),
		VoteResult:   VoteResult(mathRand.Int() % (int(Abstain) + 1)),
		Code:         randomBytes(34),
		Sign:         randomBytes(65),
	}
}

func randomUint256() *common.Uint256 {
	randBytes := make([]byte, 32)
	rand.Read(randBytes)

	result, _ := common.Uint256FromBytes(randBytes)
	return result
}

func randomBytes(len int) []byte {
	a := make([]byte, len)
	rand.Read(a)
	return a
}
