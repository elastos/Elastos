// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package state

import (
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

type ArbiterType uint8

const (
	Origin   ArbiterType = 0x00
	DPoS     ArbiterType = 0x01
	CROrigin ArbiterType = 0x02
	CRC      ArbiterType = 0x03
)

type ArbiterMember interface {
	common.Serializable
	GetType() ArbiterType

	GetOwnerPublicKey() []byte
	GetOwnerProgramHash() common.Uint168

	GetNodePublicKey() []byte
	Clone() ArbiterMember
}

func ArbiterMemberFromReader(r io.Reader) (result ArbiterMember, err error) {
	var typeByte uint8
	if typeByte, err = common.ReadUint8(r); err != nil {
		return
	}

	switch ArbiterType(typeByte) {
	case Origin:
		result = &originArbiter{arType: Origin}
	case DPoS:
		result = &dposArbiter{arType: DPoS}
	case CROrigin:
		result = &dposArbiter{arType: CROrigin}
	case CRC:
		return nil, errors.New("CRC arbiter not supported yet")
	default:
		return nil, errors.New("unknown arbiter member type")
	}

	err = result.Deserialize(r)
	return
}

func SerializeArbiterMember(ar ArbiterMember, w io.Writer) (err error) {
	if err = common.WriteUint8(w, uint8(ar.GetType())); err != nil {
		return
	}

	return ar.Serialize(w)
}
