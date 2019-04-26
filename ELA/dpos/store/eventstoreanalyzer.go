package store

import (
	"sort"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/dpos/state"
)

type EventStoreAnalyzerConfig struct {
	Store       IDposStore
	Arbitrators state.Arbitrators
}

type EventStoreAnalyzer struct {
	cfg EventStoreAnalyzerConfig

	viewCount  uint32
	voteEvents map[common.Uint256]string // vote hash as key, signer as value
}

func (e *EventStoreAnalyzer) ParseInactiveArbitrators() (result []string) {

	viewCount := e.GetLastConsensusViewCount()

	arbitratorsVoteCount := map[string]int{}
	totalVotes := e.GetLastConsensusVoteSignerHistory()
	for _, v := range totalVotes {
		if _, exists := arbitratorsVoteCount[v]; exists {
			arbitratorsVoteCount[v] += 1
		} else {
			arbitratorsVoteCount[v] = 0
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

func (e *EventStoreAnalyzer) AppendConsensusVote(vote *payload.DPOSProposalVote) {
	if vote != nil {
		e.voteEvents[vote.Hash()] = common.BytesToHexString(vote.Signer)
	}
}

func (e *EventStoreAnalyzer) Clear() {
	e.viewCount = 0
	e.voteEvents = map[common.Uint256]string{}
}

func (e *EventStoreAnalyzer) GetLastConsensusViewCount() uint32 {
	return e.viewCount
}

func (e *EventStoreAnalyzer) GetLastConsensusVoteSignerHistory() []string {
	result := make([]string, 0, len(e.voteEvents))
	for _, v := range e.voteEvents {
		result = append(result, v)
	}
	return result
}

func NewEventStoreAnalyzer(cfg EventStoreAnalyzerConfig) *EventStoreAnalyzer {
	return &EventStoreAnalyzer{
		cfg:        cfg,
		viewCount:  0,
		voteEvents: map[common.Uint256]string{},
	}
}
