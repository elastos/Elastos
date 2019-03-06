package state

import (
	"github.com/elastos/Elastos.ELA/common"
)

// 0 - H1
func (a *Arbitrators) GetNormalArbitratorsDescV0() ([][]byte, error) {
	arbitersByte := make([][]byte, 0)
	for _, arbiter := range a.State.chainParams.OriginArbiters {
		arbiterByte, err := common.HexStringToBytes(arbiter)
		if err != nil {
			return nil, err
		}
		arbitersByte = append(arbitersByte, arbiterByte)
	}

	return arbitersByte, nil
}

// H1 - H2
func (a *Arbitrators) GetNormalArbitratorsDescV1() ([][]byte, error) {
	return [][]byte{}, nil
}

// 0 - H1
func (a *Arbitrators) GetNextOnDutyArbitratorV0(height, offset uint32) []byte {
	arbitrators, _ := a.GetNormalArbitratorsDescV0()
	index := (height + offset) % uint32(len(arbitrators))
	arbiter := arbitrators[index]

	return arbiter
}
