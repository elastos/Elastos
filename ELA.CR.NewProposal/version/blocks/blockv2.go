package blocks

import (
	"errors"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/version/verconf"
)

// Ensure blockV2 implement the BlockVersion interface.
var _ BlockVersion = (*blockV2)(nil)

// blockV2 represent the current block version.
type blockV2 struct {
	*blockV1
}

func (b *blockV2) GetVersion() uint32 {
	return 2
}

func (b *blockV2) GetNormalArbitratorsDesc() ([][]byte, error) {
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

func (b *blockV2) GetCandidatesDesc() ([][]byte, error) {
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

func NewBlockV2(cfg *verconf.Config) *blockV2 {
	return &blockV2{&blockV1{cfg: cfg}}
}
