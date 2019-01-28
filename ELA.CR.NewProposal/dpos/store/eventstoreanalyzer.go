package store

import (
	"github.com/elastos/Elastos.ELA/blockchain/interfaces"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/dpos/log"
)

type EventStoreAnalyzerConfig struct {
	InactivePercentage float64
	Store              interfaces.IDposStore
	Arbitrators        interfaces.Arbitrators
}

type EventStoreAnalyzer struct {
	cfg EventStoreAnalyzerConfig
}

func (e *EventStoreAnalyzer) ParseInactiveArbitrators(blockHeight uint32) (
	result []string) {

	viewCount := e.getViewCountByHeight(blockHeight)

	arbitratorsVoteCount := map[string]int{}
	totalVotes := e.getVoteEventsByHeight(blockHeight)
	for _, v := range totalVotes {
		if _, exists := arbitratorsVoteCount[v.Signer]; exists {
			arbitratorsVoteCount[v.Signer] += 1
		} else {
			arbitratorsVoteCount[v.Signer] = 0
		}
	}

	currentArbitrators := e.cfg.Arbitrators.GetArbitrators()
	for _, v := range currentArbitrators {
		hexPk := common.BytesToHexString(v)

		if count, exists := arbitratorsVoteCount[hexPk]; exists {
			voteRatio := float64(count) / float64(viewCount)
			if voteRatio < e.cfg.InactivePercentage {
				result = append(result, hexPk)
			}
		} else {
			result = append(result, hexPk)
		}
	}

	return result
}

func (e *EventStoreAnalyzer) getViewCountByHeight(startTime uint32) uint32 {
	//todo complete me
	//now := time.Now()
	return 0
}

func (e *EventStoreAnalyzer) getVoteEventsByHeight(startTime uint32) []log.VoteEvent {
	//todo complete me
	return nil
}

func NewEventStoreAnalyzer(cfg EventStoreAnalyzerConfig) *EventStoreAnalyzer {
	return &EventStoreAnalyzer{cfg: cfg}
}
