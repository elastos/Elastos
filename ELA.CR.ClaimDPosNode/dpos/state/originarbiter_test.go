// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package state

import (
	"bytes"
	"testing"

	"github.com/elastos/Elastos.ELA/crypto"

	"github.com/stretchr/testify/assert"
)

func TestOriginArbiter_Deserialize(t *testing.T) {
	a1, err := NewOriginArbiter(Origin, randomPublicKey())
	assert.NoError(t, err)

	buf := new(bytes.Buffer)
	a1.Serialize(buf)

	a2 := &originArbiter{}
	a2.Deserialize(buf)

	assert.Equal(t, a1.GetType(), a2.GetType())
	assert.True(t, bytes.Equal(a1.GetNodePublicKey(), a2.GetNodePublicKey()))
	assert.True(t, bytes.Equal(a1.GetOwnerPublicKey(), a2.GetOwnerPublicKey()))
	assert.True(t, a1.GetOwnerProgramHash().IsEqual(a2.GetOwnerProgramHash()))
}

func TestOriginArbiter_Clone(t *testing.T) {
	a1 := &originArbiter{key: make([]byte, crypto.NegativeBigLength)}
	for i := 0; i < crypto.NegativeBigLength; i++ {
		a1.key[i] = byte(i)
	}

	a2 := a1.Clone().(*originArbiter)
	assert.True(t, bytes.Equal(a1.key, a2.key))

	a2.key[0] = 10 // should only change data of a2
	assert.False(t, bytes.Equal(a1.key, a2.key))
}
