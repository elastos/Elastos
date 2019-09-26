// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package state

import (
	"github.com/elastos/Elastos.ELA/common"
)

// 0 - H1
func (a *arbitrators) getNormalArbitratorsDescV0() ([]ArbiterMember, error) {
	arbitersByte := make([]ArbiterMember, 0)
	for _, arbiter := range a.State.chainParams.OriginArbiters {
		arbiterByte, err := common.HexStringToBytes(arbiter)
		if err != nil {
			return nil, err
		}
		ar, err := NewOriginArbiter(Origin, arbiterByte)
		if err != nil {
			return nil, err
		}
		arbitersByte = append(arbitersByte, ar)
	}

	return arbitersByte, nil
}

// H1 - H2
func (a *arbitrators) getNormalArbitratorsDescV1() ([]ArbiterMember, error) {
	return nil, nil
}

// 0 - H1
func (a *arbitrators) getNextOnDutyArbitratorV0(height,
	offset uint32) ArbiterMember {
	arbitrators, _ := a.getNormalArbitratorsDescV0()
	index := (height - 1 + offset) % uint32(len(arbitrators))
	arbiter := arbitrators[index]
	return arbiter
}
