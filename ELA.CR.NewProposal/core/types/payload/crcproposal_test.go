// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package payload

import (
	"bytes"
	"crypto/rand"
	"encoding/binary"
	"math/big"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/stretchr/testify/assert"
)

func TestCRCProposal_Deserialize(t *testing.T) {

	crpPayload1 := randomCRCProposalPayload()

	buf := new(bytes.Buffer)
	crpPayload1.Serialize(buf, CRCProposalVersion)

	crpPayload2 := &CRCProposal{}
	crpPayload2.Deserialize(buf, CRCProposalVersion)

	assert.True(t, crpPayloadEqual(crpPayload1, crpPayload2))
}

func crpPayloadEqual(payload1 *CRCProposal, payload2 *CRCProposal) bool {
	return payload1.proposalType == payload2.proposalType &&
		bytes.Equal(payload1.Sponsor, payload2.Sponsor) &&
		bytes.Equal(payload1.CRSponsor, payload2.CRSponsor) &&
		payload1.OriginHash.IsEqual(payload2.OriginHash) &&
		budgetsEqual(payload1.Budgets, payload2.Budgets) &&
		bytes.Equal(payload1.Sign, payload2.Sign) &&
		bytes.Equal(payload1.CRSign, payload2.CRSign)
}

func budgetsEqual(budgets1 []common.Fixed64, budgets2 []common.Fixed64) bool {
	if len(budgets1) != len(budgets2) {
		return false
	}
	for i, v := range budgets1 {
		if v != budgets2[i] {
			return false
		}
	}
	return true
}

func randomCRCProposalPayload() *CRCProposal {
	return &CRCProposal{
		proposalType: CRCProposalType(randomBytes(1)[0]),
		Sponsor:      randomBytes(33),
		CRSponsor:    randomBytes(34),
		OriginHash:   *randomUint256(),
		Budgets:      randomBudgets(),
		Sign:         randomBytes(64),
		CRSign:       randomBytes(64),
	}
}

func randomUint256() *common.Uint256 {
	randBytes := make([]byte, 32)
	rand.Read(randBytes)
	result, _ := common.Uint256FromBytes(randBytes)

	return result
}

func randomFix64() common.Fixed64 {
	var randNum int64
	binary.Read(rand.Reader, binary.BigEndian, &randNum)
	return common.Fixed64(randNum)
}

func randomBudgets() []common.Fixed64 {
	max := big.NewInt(100)
	count, _ := rand.Int(rand.Reader, max)
	budgets := make([]common.Fixed64, count.Uint64())
	for i := uint64(0); i < count.Uint64(); i++ {
		budgets[i] = randomFix64()
	}
	return budgets
}
