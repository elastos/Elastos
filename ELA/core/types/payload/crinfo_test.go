// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package payload

import (
	"bytes"
	"crypto/rand"
	rand2 "math/rand"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/stretchr/testify/assert"
)

func TestCRInfo_Deserialize(t *testing.T) {

	crPayload1 := randomCRInfoPayload()

	buf := new(bytes.Buffer)
	crPayload1.Serialize(buf, CRInfoVersion)

	crPayload2 := &CRInfo{}
	crPayload2.Deserialize(buf, CRInfoVersion)

	assert.True(t, payloadEqual(crPayload1, crPayload2))
}

func payloadEqual(crPayload1 *CRInfo, crPayload2 *CRInfo) bool {
	if !bytes.Equal(crPayload1.Code, crPayload2.Code) ||
		!crPayload1.CID.IsEqual(crPayload2.CID) ||
		crPayload1.NickName != crPayload2.NickName ||
		crPayload1.Url != crPayload2.Url ||
		crPayload1.Location != crPayload2.Location ||
		!bytes.Equal(crPayload1.Signature, crPayload2.Signature) {
		return false
	}

	return true
}

func randomCRInfoPayload() *CRInfo {
	return &CRInfo{
		Code:      randomBytes(34),
		CID:       *randomUint168(),
		NickName:  randomString(),
		Url:       randomString(),
		Location:  rand2.Uint64(),
		Signature: randomBytes(65),
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
