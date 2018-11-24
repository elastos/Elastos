package blockhistory

import (
	"errors"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/config"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type BlockVersionV0 struct {
	blockchain.BlockVersionMain
}

func (b *BlockVersionV0) GetVersion() uint32 {
	return 0
}

func (b *BlockVersionV0) GetProducersDesc() ([][]byte, error) {
	if len(config.Parameters.Arbiters) == 0 {
		return nil, errors.New("arbiters not configured")
	}

	arbitersByte := make([][]byte, 0)
	for _, arbiter := range config.Parameters.Arbiters {
		arbiterByte, err := common.HexStringToBytes(arbiter)
		if err != nil {
			return nil, err
		}
		arbitersByte = append(arbitersByte, arbiterByte)
	}

	return arbitersByte, nil
}
