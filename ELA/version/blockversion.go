package version

import (
	"errors"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/config"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type BlockVersion interface {
	GetVersion() uint32
	GetProducersDesc() ([][]byte, error)
}

type BlockVersionMain struct {
}

func (b *BlockVersionMain) GetVersion() uint32 {
	return 1
}

func (b *BlockVersionMain) GetProducersDesc() ([][]byte, error) {
	producersInfo := blockchain.DefaultLedger.Store.GetRegisteredProducers()
	if uint32(len(producersInfo)) < config.Parameters.ArbiterConfiguration.ArbitratorsCount {
		return nil, errors.New("producers count less than min arbitrators count.")
	}

	result := make([][]byte, 0)
	for i := uint32(0); i < uint32(len(producersInfo)); i++ {
		arbiterByte, err := common.HexStringToBytes(producersInfo[i].PublicKey)
		if err != nil {
			return nil, err
		}
		result = append(result, arbiterByte)
	}
	return result, nil
}
