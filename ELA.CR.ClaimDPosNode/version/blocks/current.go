package blocks

import (
	"errors"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/version/verconf"
)

// Ensure blockCurrent implement the BlockVersion interface.
var _ BlockVersion = (*blockCurrent)(nil)

// blockCurrent represent the current block version.
type blockCurrent struct {
	*blockV1
}

func (b *blockCurrent) GetVersion() uint32 {
	return 2
}

func (b *blockCurrent) GetNormalArbitratorsDesc() ([][]byte, error) {
	resultCount := config.Parameters.ArbiterConfiguration.NormalArbitratorsCount

	producersInfo := b.cfg.ChainStore.GetRegisteredProducers()
	if uint32(len(producersInfo)) < resultCount {
		return nil, errors.New("producers count less than min arbitrators count")
	}

	result := make([][]byte, resultCount)
	for i := uint32(0); i < resultCount; i++ {
		result[i] = producersInfo[i].PublicKey
	}
	return result, nil
}

func (b *blockCurrent) GetCandidatesDesc() ([][]byte, error) {
	startIndex := config.Parameters.ArbiterConfiguration.NormalArbitratorsCount

	producersInfo := b.cfg.ChainStore.GetRegisteredProducers()
	if uint32(len(producersInfo)) < startIndex {
		return nil, errors.New("producers count less than min arbitrators count")
	}

	result := make([][]byte, 0)
	for i := startIndex; i < uint32(len(producersInfo)) && i < startIndex+config.Parameters.ArbiterConfiguration.CandidatesCount; i++ {
		result = append(result, producersInfo[i].PublicKey)
	}
	return result, nil
}

func NewBlockCurrent(cfg *verconf.Config) *blockCurrent {
	return &blockCurrent{&blockV1{cfg: cfg}}
}
