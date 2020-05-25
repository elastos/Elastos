// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package crypto

import (
	"sort"
	"testing"

	"github.com/stretchr/testify/assert"
)

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

func TestEncryptDecrypt(t *testing.T) {
	priKey, pubKey, _ := GenerateKeyPair()

	message := []byte("Hello World!")

	cipher, err := Encrypt(pubKey, message)
	assert.NoError(t, err)

	m, err := Decrypt(priKey, cipher)
	assert.NoError(t, err)

	assert.Equal(t, message, m)
}
