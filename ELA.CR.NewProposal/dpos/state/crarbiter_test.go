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
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/cr/state"

	"github.com/stretchr/testify/assert"
)

func TestCRCArbiter_Deserialize(t *testing.T) {
	a, _ := NewCRCArbiter(randomPublicKey(), randomCRMember())
	ar1 := a.(*crcArbiter)

	buf := new(bytes.Buffer)
	ar1.Serialize(buf)

	ar2 := &crcArbiter{}
	ar2.Deserialize(buf)

	assert.True(t, bytes.Equal(ar1.nodePk, ar2.nodePk))
	assert.True(t, ar1.ownerHash.IsEqual(ar2.ownerHash))
	assert.True(t, crMemberEqual(ar1.crMember, ar2.crMember))
}

func TestCRCArbiter_Clone(t *testing.T) {
	a, _ := NewCRCArbiter(randomPublicKey(), randomCRMember())
	ar1 := a.(*crcArbiter)

	a = a.Clone()
	ar2 := a.(*crcArbiter)

	assert.True(t, bytes.Equal(ar1.nodePk, ar2.nodePk))
	assert.True(t, ar1.ownerHash.IsEqual(ar2.ownerHash))
	assert.True(t, crMemberEqual(ar1.crMember, ar2.crMember))
}

func randomCRMember() *state.CRMember {
	return &state.CRMember{
		Info:             *randomCRInfo(),
		ImpeachmentVotes: common.Fixed64(rand.Uint64()),
	}
}

func randomCRInfo() *payload.CRInfo {
	return &payload.CRInfo{
		Code:     randomBytes(34),
		CID:      *randomUint168(),
		NickName: randomString(),
		Url:      randomString(),
		Location: rand.Uint64(),
	}
}

func randomUint168() *common.Uint168 {
	randBytes := make([]byte, 21)
	rand.Read(randBytes)
	result, _ := common.Uint168FromBytes(randBytes)

	return result
}

func crMemberEqual(first *state.CRMember, second *state.CRMember) bool {
	return crInfoEqual(&first.Info, &second.Info) &&
		first.ImpeachmentVotes == second.ImpeachmentVotes
}

func crInfoEqual(first *payload.CRInfo, second *payload.CRInfo) bool {
	if !bytes.Equal(first.Code, second.Code) ||
		!first.DID.IsEqual(second.DID) ||
		first.NickName != second.NickName ||
		first.Url != second.Url ||
		first.Location != second.Location {
		return false
	}
	return true
}
