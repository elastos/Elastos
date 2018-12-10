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
	ct, err := CreateStandardContractByPubKey(pub)
	if err != nil {
		t.Errorf("[PublicKeyToStandardProgramHash] failed")
	}
	programHash, err := ct.ToProgramHash()
	if err != nil {
		t.Errorf("[ToProgramHash] failed")
	}
	addr, _ := programHash.ToAddress()
	if !assert.Equal(t, "ENTogr92671PKrMmtWo3RLiYXfBTXUe13Z", addr) {
		t.FailNow()
	}

	// Empty code
	var ct1 = Contract{}
	_, err = ct1.ToProgramHash()
	if !assert.EqualError(t, err, "[ToProgramHash] failed, empty program code") {
		t.FailNow()
	}
}
