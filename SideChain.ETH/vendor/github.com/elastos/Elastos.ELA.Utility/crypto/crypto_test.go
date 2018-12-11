package crypto

import (
	"crypto/rand"
	"github.com/stretchr/testify/assert"
	math "math/rand"
	"sort"
	"testing"

	"encoding/hex"
	"github.com/elastos/Elastos.ELA.Utility/common"
)

func TestToProgramHash(t *testing.T) {
	// Exit address
	publicKeyHex := "022c9652d3ad5cc065aa9147dc2ad022f80001e8ed233de20f352950d351d472b7"
	publicKey, err := hex.DecodeString(publicKeyHex)
	pub, _ := DecodePoint(publicKey)
	code, _ := CreateStandardRedeemScript(pub)
	hash, _ := ToProgramHash(code)
	addr, _ := hash.ToAddress()
	if !assert.Equal(t, "ENTogr92671PKrMmtWo3RLiYXfBTXUe13Z", addr) {
		t.FailNow()
	}

	// Empty code
	hash, err = ToProgramHash(nil)
	if !assert.EqualError(t, err, "[ToProgramHash] failed, empty program code") {
		t.FailNow()
	}

	// CHECKSIG
	code = make([]byte, PublicKeyScriptLength)
	rand.Read(code)
	code[len(code)-1] = common.STANDARD
	hash, err = ToProgramHash(code)
	if !assert.NoError(t, err) {
		t.FailNow()
	}
	_, err = hash.ToAddress()
	if !assert.NoError(t, err) {
		t.FailNow()
	}

	// Invalid CHECKSIG
	code = make([]byte, PublicKeyScriptLength-1)
	rand.Read(code)
	code[len(code)-1] = common.STANDARD
	hash, err = ToProgramHash(code)
	if !assert.EqualError(t, err, "[ToProgramHash] error, not a valid checksig script") {
		t.FailNow()
	}

	// MULTISIG
	num := math.Intn(5) + 3
	code = make([]byte, (PublicKeyScriptLength-1)*num+3)
	rand.Read(code)
	code[len(code)-1] = common.MULTISIG
	hash, err = ToProgramHash(code)
	if !assert.NoError(t, err) {
		t.FailNow()
	}
	_, err = hash.ToAddress()
	if !assert.NoError(t, err) {
		t.FailNow()
	}

	// Invalid MULTISIG
	code = make([]byte, PublicKeyScriptLength*num+3)
	rand.Read(code)
	code[len(code)-1] = common.MULTISIG
	hash, err = ToProgramHash(code)
	if !assert.EqualError(t, err, "[ToProgramHash] error, not a valid multisig script") {
		t.FailNow()
	}

	// CROSSCHAIN
	code = make([]byte, (PublicKeyScriptLength-1)*num+3)
	rand.Read(code)
	code[len(code)-1] = common.CROSSCHAIN
	hash, err = ToProgramHash(code)
	if !assert.NoError(t, err) {
		t.FailNow()
	}
	_, err = hash.ToAddress()
	if !assert.NoError(t, err) {
		t.FailNow()
	}
}

func TestSortPublicKeys(t *testing.T) {
	count := 10
	publicKeys := make([]*PublicKey, 0, count)
	dupPubKeys := make([]*PublicKey, 0, count)
	for i := 0; i < count; i++ {
		_, public, err := GenerateKeyPair()
		if err != nil {
			t.Errorf("Generate key pair failed, error %s", err.Error())
		}
		publicKeys = append(publicKeys, public)
		dupPubKeys = append(dupPubKeys, public)
	}

	SortPublicKeys(publicKeys)
	sort.Sort(pubKeySlice(dupPubKeys))

	assert.Equal(t, dupPubKeys, publicKeys)
}

type pubKeySlice []*PublicKey

func (p pubKeySlice) Len() int { return len(p) }
func (p pubKeySlice) Less(i, j int) bool {
	r := p[i].X.Cmp(p[j].X)
	if r <= 0 {
		return true
	}
	return false
}
func (p pubKeySlice) Swap(i, j int) {
	p[i], p[j] = p[j], p[i]
}
