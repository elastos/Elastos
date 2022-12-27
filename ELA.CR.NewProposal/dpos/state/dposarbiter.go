// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package state

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
)

type dposArbiter struct {
	arType    ArbiterType
	producer  Producer
	ownerHash common.Uint168
}

func (d *dposArbiter) Serialize(w io.Writer) (err error) {
	if err = d.producer.Serialize(w); err != nil {
		return
	}

	return d.ownerHash.Serialize(w)
}

func (d *dposArbiter) Deserialize(r io.Reader) (err error) {
	if err = d.producer.Deserialize(r); err != nil {
		return
	}

	return d.ownerHash.Deserialize(r)
}

func (d *dposArbiter) Clone() ArbiterMember {
	buf := new(bytes.Buffer)
	d.Serialize(buf)

	result := &dposArbiter{}
	result.Deserialize(buf)
	return result
}

func (d *dposArbiter) GetType() ArbiterType {
	return d.arType
}

func (d *dposArbiter) GetOwnerPublicKey() []byte {
	return d.producer.OwnerPublicKey()
}

func (d *dposArbiter) GetOwnerProgramHash() common.Uint168 {
	return d.ownerHash
}

func (d *dposArbiter) GetNodePublicKey() []byte {
	return d.producer.NodePublicKey()
}

func NewDPoSArbiter(t ArbiterType, producer *Producer) (ArbiterMember, error) {
	ar := &dposArbiter{
		arType:   t,
		producer: *producer,
	}
	hash, err := contract.PublicKeyToStandardProgramHash(
		producer.OwnerPublicKey())
	if err != nil {
		return nil, err
	}
	ar.ownerHash = *hash
	return ar, nil
}
