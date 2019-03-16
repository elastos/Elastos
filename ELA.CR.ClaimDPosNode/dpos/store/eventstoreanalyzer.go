package store

import (
	"sort"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/dpos/log"
	"github.com/elastos/Elastos.ELA/dpos/state"
)

type EventStoreAnalyzerConfig struct {
	InactiveEliminateCount uint32
	Store                  IDposStore
	Arbitrators            state.Arbitrators
}

type EventStoreAnalyzer struct {
	cfg EventStoreAnalyzerConfig
}

func (e *EventStoreAnalyzer) ParseInactiveArbitrators() (
	result []string) {

	viewCount := e.getLastConsensusViewCount()

	arbitratorsVoteCount := map[string]int{}
	totalVotes := e.getLastConsensusVoteEvents()
	for _, v := range totalVotes {
		if _, exists := arbitratorsVoteCount[v.Signer]; exists {
			arbitratorsVoteCount[v.Signer] += 1
		} else {
			arbitratorsVoteCount[v.Signer] = 0
		}
	}

	type sortItem struct {
		Ratio float64
		PK    string
		CRC   bool
	}
	var sortItems []sortItem
	currentArbitrators := e.cfg.Arbitrators.GetArbitrators()
	for _, v := range currentArbitrators {
		hexPk := common.BytesToHexString(v)

		ratio := float64(0)
		if count, exists := arbitratorsVoteCount[hexPk]; exists {
			ratio = float64(count) / float64(viewCount)
		}

		sortItems = append(sortItems, sortItem{
			Ratio: ratio,
			PK:    hexPk,
			CRC:   e.cfg.Arbitrators.IsCRCArbitrator(v),
		})
	}

	sort.Slice(sortItems, func(i, j int) bool {
		return sortItems[i].Ratio > sortItems[j].Ratio
	})
	for i := 0; i < len(sortItems) &&
		i < int(e.cfg.InactiveEliminateCount); i++ {
		if !sortItems[i].CRC {
			result = append(result, sortItems[i].PK)
		}
	}

	sort.Strings(result)
	return result
}

func (e *EventStoreAnalyzer) getLastConsensusViewCount() uint32 {
	//todo complete me
	return 0
}

func (e *EventStoreAnalyzer) getLastConsensusVoteEvents() []log.VoteEvent {
	//todo complete me
	return nil
}

func NewEventStoreAnalyzer(cfg EventStoreAnalyzerConfig) *EventStoreAnalyzer {
	return &EventStoreAnalyzer{cfg: cfg}
}
