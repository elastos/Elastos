package store

import (
	"sort"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/dpos/log"
	"github.com/elastos/Elastos.ELA/dpos/state"
)

type EventStoreAnalyzerConfig struct {
	Store                  IDposStore
	Arbitrators            state.Arbitrators
}

type EventStoreAnalyzer struct {
	cfg EventStoreAnalyzerConfig

	viewCount  uint32
	voteEvents map[common.Uint256]*log.VoteEvent
}

func (e *EventStoreAnalyzer) ParseInactiveArbitrators() (
	result []string) {

	viewCount := e.GetLastConsensusViewCount()

	arbitratorsVoteCount := map[string]int{}
	totalVotes := e.GetLastConsensusVoteEvents()
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
		return sortItems[i].Ratio < sortItems[j].Ratio
	})
	// set eliminate count to 1/3 of current arbitrators
	inactiveEliminateCount := len(currentArbitrators) / 3
	for i := 0; i < len(sortItems) &&
		i < int(inactiveEliminateCount); i++ {
		if !sortItems[i].CRC {
			result = append(result, sortItems[i].PK)
		}
	}

	sort.Strings(result)
	return result
}

func (e *EventStoreAnalyzer) IncreaseLastConsensusViewCount() {
	e.viewCount++
}

func (e *EventStoreAnalyzer) AppendConsensusVoteEvent(event *log.VoteEvent) {
	if event.RawData != nil {
		e.voteEvents[event.RawData.Hash()] = event
	}
}

func (e *EventStoreAnalyzer) Clear() {
	e.viewCount = 0
	e.voteEvents = map[common.Uint256]*log.VoteEvent{}
}

func (e *EventStoreAnalyzer) GetLastConsensusViewCount() uint32 {
	return e.viewCount
}

func (e *EventStoreAnalyzer) GetLastConsensusVoteEvents() []*log.VoteEvent {
	result := make([]*log.VoteEvent, 0)
	for _, v := range e.voteEvents {
		result = append(result, v)
	}
	return result
}

func NewEventStoreAnalyzer(cfg EventStoreAnalyzerConfig) *EventStoreAnalyzer {
	return &EventStoreAnalyzer{
		cfg:        cfg,
		viewCount:  0,
		voteEvents: map[common.Uint256]*log.VoteEvent{},
	}
}
