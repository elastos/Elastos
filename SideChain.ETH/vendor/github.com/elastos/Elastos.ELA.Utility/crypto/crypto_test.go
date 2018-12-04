package crypto

import (
	"crypto/rand"
	"fmt"
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
	if addr != "ENTogr92671PKrMmtWo3RLiYXfBTXUe13Z" {
		t.Errorf("Exit code test, expect %s got %s", "ENTogr92671PKrMmtWo3RLiYXfBTXUe13Z", addr)
	}
	t.Logf("Exit address match ENTogr92671PKrMmtWo3RLiYXfBTXUe13Z")

	// Empty code
	hash, err = ToProgramHash(nil)
	if err == nil {
		t.Errorf("Expect error %s got nil", "[ToProgramHash] failed, empty program code")
	}
	if err.Error() != "[ToProgramHash] failed, empty program code" {
		t.Errorf("Expect error %s got %s", "[ToProgramHash] failed, empty program code", err.Error())
	}
	t.Logf("[Passed] with empty hash")

	// CHECKSIG
	code = make([]byte, PublicKeyScriptLength)
	rand.Read(code)
	code[len(code)-1] = common.STANDARD
	hash, err = ToProgramHash(code)
	if err != nil {
		t.Error(err.Error())
	}
	addr, _ = hash.ToAddress()
	t.Logf("[Passed] with STANDARD hash %s addr %s", hash.String(), addr)

	// Invalid CHECKSIG
	code = make([]byte, PublicKeyScriptLength-1)
	rand.Read(code)
	code[len(code)-1] = common.STANDARD
	hash, err = ToProgramHash(code)
	if err == nil {
		t.Errorf("Expect error %s got nil", "[ToProgramHash] error, not a valid checksig script")
	}
	if err.Error() != "[ToProgramHash] error, not a valid checksig script" {
		t.Errorf("Expect error %s got %s", "[ToProgramHash] error, not a valid checksig script", err.Error())
	}
	t.Logf("[Passed] with invalied STANDARD code")

	// MULTISIG
	num := math.Intn(5) + 3
	code = make([]byte, (PublicKeyScriptLength-1)*num+3)
	rand.Read(code)
	code[len(code)-1] = common.MULTISIG
	hash, err = ToProgramHash(code)
	if err != nil {
		t.Error(err.Error())
	}
	addr, _ = hash.ToAddress()
	t.Logf("[Passed] with MULTISIG hash %s addr %s", hash.String(), addr)

	// Invalid MULTISIG
	code = make([]byte, PublicKeyScriptLength*num+3)
	rand.Read(code)
	code[len(code)-1] = common.MULTISIG
	hash, err = ToProgramHash(code)
	if err == nil {
		t.Errorf("Expect error %s got nil", "[ToProgramHash] error, not a valid multisig script")
	}
	if err.Error() != "[ToProgramHash] error, not a valid multisig script" {
		t.Errorf("Expect error %s got %s", "[ToProgramHash] error, not a valid multisig script", err.Error())
	}
	t.Logf("[Passed] with invalied MULTISIG code")

	// CROSSCHAIN
	code = make([]byte, (PublicKeyScriptLength-1)*num+3)
	rand.Read(code)
	code[len(code)-1] = common.CROSSCHAIN
	hash, err = ToProgramHash(code)
	if err != nil {
		t.Error(err.Error())
	}
	addr, _ = hash.ToAddress()
	t.Logf("[Passed] with CROSSCHAIN hash %s addr %s", hash.String(), addr)

	// Invalid CROSSCHAIN
	code = make([]byte, PublicKeyScriptLength*num+3)
	rand.Read(code)
	code[len(code)-1] = common.CROSSCHAIN
	hash, err = ToProgramHash(code)
	if err == nil {
		t.Errorf("Expect error %s got nil", "[ToProgramHash] error, not a valid crosschain script")
	}
	if err.Error() != "[ToProgramHash] error, not a valid crosschain script" {
		t.Errorf("Expect error %s got %s", "[ToProgramHash] error, not a valid crosschain script", err.Error())
	}
	t.Logf("[Passed] with invalied CROSSCHAIN code")
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

	for i, pubKey := range publicKeys {
		if !Equal(pubKey, dupPubKeys[i]) {
			t.Errorf("Sorted public keys not the same")
		}
	}

	fmt.Println(publicKeys)
	fmt.Println(dupPubKeys)
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
