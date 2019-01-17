package blocks

import (
	"errors"
	"sort"

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

	producers := b.cfg.Chain.GetState().GetActiveProducers()
	if uint32(len(producers)) < resultCount {
		return nil, errors.New("producers count less than min arbitrators count")
	}

	sort.Slice(producers, func(i, j int) bool {
		return producers[i].Votes() > producers[j].Votes()
	})

	result := make([][]byte, resultCount)
	for i := uint32(0); i < resultCount; i++ {
		result[i] = producers[i].Info().NodePublicKey
	}
	return result, nil
}

func (b *blockV2) GetCandidatesDesc() ([][]byte, error) {
	startIndex := config.Parameters.ArbiterConfiguration.NormalArbitratorsCount

	producers := b.cfg.Chain.GetState().GetActiveProducers()
	if uint32(len(producers)) < startIndex {
		return nil, errors.New("producers count less than min arbitrators count")
	}

	sort.Slice(producers, func(i, j int) bool {
		return producers[i].Votes() > producers[j].Votes()
	})

	result := make([][]byte, 0)
	for i := startIndex; i < uint32(len(producers)) && i < startIndex+config.Parameters.ArbiterConfiguration.CandidatesCount; i++ {
		result = append(result, producers[i].Info().NodePublicKey)
	}
	return result, nil
}

func NewBlockV2(cfg *verconf.Config) *blockV2 {
	return &blockV2{&blockV1{cfg: cfg}}
}
