package blockhistory

import (
	"errors"

	"github.com/elastos/Elastos.ELA/blockchain"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

var originalArbitrators = []string{
	"03e333657c788a20577c0288559bd489ee65514748d18cb1dc7560ae4ce3d45613",
	"02dd22722c3b3a284929e4859b07e6a706595066ddd2a0b38e5837403718fb047c",
	"03e4473b918b499e4112d281d805fc8d8ae7ac0a71ff938cba78006bf12dd90a85",
	"03dd66833d28bac530ca80af0efbfc2ec43b4b87504a41ab4946702254e7f48961",
	"02c8a87c076112a1b344633184673cfb0bb6bce1aca28c78986a7b1047d257a448",
}

type BlockVersionV0 struct {
	blockchain.BlockVersionMain
}

func (b *BlockVersionV0) GetVersion() uint32 {
	return 0
}

func (b *BlockVersionV0) GetProducersDesc() ([][]byte, error) {
	if len(originalArbitrators) == 0 {
		return nil, errors.New("arbiters not configured")
	}

	arbitersByte := make([][]byte, 0)
	for _, arbiter := range originalArbitrators {
		arbiterByte, err := common.HexStringToBytes(arbiter)
		if err != nil {
			return nil, err
		}
		arbitersByte = append(arbitersByte, arbiterByte)
	}

	return arbitersByte, nil
}
