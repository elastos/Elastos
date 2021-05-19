// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package state

import (
	"bytes"
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestDposArbiter_Deserialize(t *testing.T) {
	a, _ := NewDPoSArbiter(DPoS, randomProducer())
	ar1 := a.(*dposArbiter)

	buf := new(bytes.Buffer)
	ar1.Serialize(buf)

	ar2 := &dposArbiter{}
	ar2.Deserialize(buf)

	assert.True(t, producerEqual(&ar1.producer, &ar2.producer))
	assert.True(t, ar1.ownerHash.IsEqual(ar2.ownerHash))
}

func TestDposArbiter_Clone(t *testing.T) {
	a, _ := NewDPoSArbiter(DPoS, randomProducer())
	ar1 := a.(*dposArbiter)

	ar2 := ar1.Clone().(*dposArbiter)

	assert.True(t, producerEqual(&ar1.producer, &ar2.producer))
	assert.True(t, ar1.ownerHash.IsEqual(ar2.ownerHash))

	ar1.producer.info.NodePublicKey[0] = ar1.producer.info.NodePublicKey[0] + 1
	assert.False(t, producerEqual(&ar1.producer, &ar2.producer))
}
