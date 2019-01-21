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

func (e *EventStoreAnalyzer) ParseInactiveArbitrators() (result []string) {

	startTime := e.cfg.Arbitrators.GetLastConfirmedBlockTimeStamp()
	viewCount := e.getViewCountUtilNow(startTime)

	arbitratorsVoteCount := map[string]int{}
	totalVotes := e.getEventsUtilNow(startTime)
	for _, v := range totalVotes {
		if _, exists := arbitratorsVoteCount[v.Signer]; exists {
			arbitratorsVoteCount[v.Signer] += 1
		} else {
			arbitratorsVoteCount[v.Signer] = 0
		}
	}

	crcArbitrators := e.cfg.Arbitrators.GetCRCArbitrators()
	crcArbitratorsMap := map[string]interface{}{}
	for _, v := range crcArbitrators {
		crcArbitratorsMap[common.BytesToHexString(v.PublicKey)] = nil
	}

	currentArbitrators := e.cfg.Arbitrators.GetArbitrators()
	for _, v := range currentArbitrators {
		hexPk := common.BytesToHexString(v)
		if _, exists := crcArbitratorsMap[hexPk]; exists {
			continue
		}

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

func (e *EventStoreAnalyzer) getViewCountUtilNow(startTime uint32) uint32 {
	//todo complete me
	//now := time.Now()
	return 0
}

func (e *EventStoreAnalyzer) getEventsUtilNow(startTime uint32) []log.VoteEvent {
	//todo complete me
	return nil
}

func NewEventStoreAnalyzer(cfg EventStoreAnalyzerConfig) *EventStoreAnalyzer {
	return &EventStoreAnalyzer{cfg: cfg}
}
