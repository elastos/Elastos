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

func TestArbiterMemberInterfaceDeserialize(t *testing.T) {
	// origin
	ar1, _ := NewOriginArbiter(Origin, randomPublicKey())
	buf := new(bytes.Buffer)
	SerializeArbiterMember(ar1, buf)
	ar2, _ := ArbiterMemberFromReader(buf)

	origin1 := ar1.(*originArbiter)
	origin2 := ar2.(*originArbiter)
	assert.Equal(t, ar1.GetType(), ar2.GetType())
	assert.True(t, bytes.Equal(origin1.key, origin2.key))
	assert.True(t, origin1.ownerHash.IsEqual(origin2.ownerHash))

	// DPoS
	ar1, _ = NewDPoSArbiter(DPoS, randomProducer())
	buf = new(bytes.Buffer)
	SerializeArbiterMember(ar1, buf)
	ar2, _ = ArbiterMemberFromReader(buf)

	dpos1 := ar1.(*dposArbiter)
	dpos2 := ar2.(*dposArbiter)
	assert.Equal(t, ar1.GetType(), ar2.GetType())
	assert.True(t, producerEqual(&dpos1.producer, &dpos2.producer))
	assert.True(t, dpos1.ownerHash.IsEqual(dpos2.ownerHash))

	// CROrigin
	ar1, _ = NewDPoSArbiter(CROrigin, randomProducer())
	buf = new(bytes.Buffer)
	SerializeArbiterMember(ar1, buf)
	ar2, _ = ArbiterMemberFromReader(buf)

	crOrigin1 := ar1.(*dposArbiter)
	crOrigin2 := ar2.(*dposArbiter)
	assert.Equal(t, ar1.GetType(), ar2.GetType())
	assert.True(t, producerEqual(&crOrigin1.producer, &crOrigin2.producer))
	assert.True(t, crOrigin1.ownerHash.IsEqual(crOrigin2.ownerHash))
}
