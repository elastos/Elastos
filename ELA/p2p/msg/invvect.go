// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package msg

import (
	"fmt"
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

const (
	// MaxInvPerMsg is the maximum number of inventory vectors that can be in a
	// single inv message.
	MaxInvPerMsg = 50000

	// Maximum payload size for an inventory vector.
	maxInvVectPayload = 4 + common.UINT256SIZE
)

type InvType uint32

const (
	InvTypeError InvType = iota
	InvTypeTx
	InvTypeBlock
	InvTypeFilteredBlock
	InvTypeConfirmedBlock
	InvTypeAddress
)

func (i InvType) String() string {
	switch i {
	case InvTypeError:
		return "ERROR"
	case InvTypeTx:
		return "MSG_TX"
	case InvTypeBlock:
		return "MSG_BLOCK"
	case InvTypeFilteredBlock:
		return "MSG_FILTERED_BLOCK"
	case InvTypeConfirmedBlock:
		return "MSG_CONFIRMED_BLOCK"
	case InvTypeAddress:
		return "MSG_ADDRESS"
	default:
		return fmt.Sprintf("Unknown InvType (%d)", i)
	}
}

type InvVect struct {
	Type InvType
	Hash common.Uint256
}

// NewInvVect returns a new InvVect using the provided type and hash.
func NewInvVect(typ InvType, hash *common.Uint256) *InvVect {
	return &InvVect{
		Type: typ,
		Hash: *hash,
	}
}

func (vect *InvVect) Serialize(w io.Writer) error {
	return common.WriteElements(w, vect.Type, &vect.Hash)
}

func (vect *InvVect) Deserialize(r io.Reader) error {
	return common.ReadElements(r, &vect.Type, &vect.Hash)
}
