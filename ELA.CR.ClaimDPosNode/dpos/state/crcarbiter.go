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
	"github.com/elastos/Elastos.ELA/cr/state"
	"github.com/elastos/Elastos.ELA/crypto"
)

type crcArbiter struct {
	crMember  *state.CRMember
	nodePk    []byte
	ownerHash common.Uint168
}

func (c *crcArbiter) Serialize(w io.Writer) (err error) {
	if err = c.crMember.Serialize(w); err != nil {
		return
	}

	if err = common.WriteVarBytes(w, c.nodePk); err != nil {
		return
	}

	return c.ownerHash.Serialize(w)
}

func (c *crcArbiter) Deserialize(r io.Reader) (err error) {
	c.crMember = &state.CRMember{}
	if err = c.crMember.Deserialize(r); err != nil {
		return
	}

	if c.nodePk, err = common.ReadVarBytes(r, crypto.NegativeBigLength,
		"public key"); err != nil {
		return
	}

	return c.ownerHash.Deserialize(r)
}

func (c *crcArbiter) GetType() ArbiterType {
	return CRC
}

func (c *crcArbiter) GetOwnerPublicKey() []byte {
	return c.getPublicKey()
}

func (c *crcArbiter) GetOwnerProgramHash() common.Uint168 {
	return c.ownerHash
}

func (c *crcArbiter) GetNodePublicKey() []byte {
	return c.nodePk
}

func (c *crcArbiter) Clone() ArbiterMember {
	buf := new(bytes.Buffer)
	c.Serialize(buf)

	result := &crcArbiter{}
	result.Deserialize(buf)
	return result
}

func (c *crcArbiter) getPublicKey() []byte {
	// todo support for multi public key later
	return c.crMember.Info.Code[1 : len(c.crMember.Info.Code)-1]
}

func NewCRCArbiter(nodePK []byte, ownerPK []byte, cr *state.CRMember) (ArbiterMember, error) {
	ar := &crcArbiter{
		crMember: cr,
		nodePk:   nodePK,
	}
	hash, err := contract.PublicKeyToStandardProgramHash(ownerPK)
	if err != nil {
		return nil, err
	}
	ar.ownerHash = *hash
	return ar, nil
}
