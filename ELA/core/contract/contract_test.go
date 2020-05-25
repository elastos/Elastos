// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package contract

import (
	"encoding/hex"
	"testing"

	"github.com/elastos/Elastos.ELA/crypto"

	"github.com/stretchr/testify/assert"
)

func TestToProgramHash(t *testing.T) {
	// Exit address
	publicKeyHex := "022c9652d3ad5cc065aa9147dc2ad022f80001e8ed233de20f352950d351d472b7"
	publicKey, err := hex.DecodeString(publicKeyHex)
	pub, _ := crypto.DecodePoint(publicKey)
	ct, err := CreateStandardContract(pub)
	if err != nil {
		t.Errorf("[PublicKeyToStandardProgramHash] failed")
	}
	programHash := ct.ToProgramHash()
	addr, _ := programHash.ToAddress()
	if !assert.Equal(t, "ENTogr92671PKrMmtWo3RLiYXfBTXUe13Z", addr) {
		t.FailNow()
	}
}
