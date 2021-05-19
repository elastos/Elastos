// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package manager

import (
	"sort"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/dpos/state"
)

type EventAnalyzerConfig struct {
	Arbitrators state.Arbitrators
}

type eventAnalyzer struct {
	cfg EventAnalyzerConfig

	viewCount  uint32
	voteEvents map[common.Uint256]string // vote hash as key, signer as value
}

func (e *eventAnalyzer) ParseInactiveArbitrators() (result []string) {

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
	}
	var sortItems []sortItem
	currentArbitrators := e.cfg.Arbitrators.GetArbitrators()
	for _, v := range currentArbitrators {
		if e.cfg.Arbitrators.IsCRCArbitrator(v) {
			continue
		}
		hexPk := common.BytesToHexString(v)

		ratio := float64(0)
		if count, exists := arbitratorsVoteCount[hexPk]; exists {
			ratio = float64(count) / float64(viewCount)
		}

		sortItems = append(sortItems, sortItem{
			Ratio: ratio,
			PK:    hexPk,
		})
	}

	sort.Slice(sortItems, func(i, j int) bool {
		return sortItems[i].Ratio < sortItems[j].Ratio
	})
	// set eliminate count to 1/3 of current arbitrators
	inactiveEliminateCount := len(currentArbitrators) / 3
	for i := 0; i < len(sortItems) &&
		i < int(inactiveEliminateCount); i++ {
		result = append(result, sortItems[i].PK)
	}

	sort.Strings(result)
	return result
}

func (e *eventAnalyzer) IncreaseLastConsensusViewCount() {
	e.viewCount++
}

func (e *eventAnalyzer) AppendConsensusVote(vote *payload.DPOSProposalVote) {
	if vote != nil {
		e.voteEvents[vote.Hash()] = common.BytesToHexString(vote.Signer)
	}
}

func (e *eventAnalyzer) Clear() {
	e.viewCount = 0
	e.voteEvents = map[common.Uint256]string{}
}

func (e *eventAnalyzer) GetLastConsensusViewCount() uint32 {
	return e.viewCount
}

func (e *eventAnalyzer) GetLastConsensusVoteSignerHistory() []string {
	result := make([]string, 0, len(e.voteEvents))
	for _, v := range e.voteEvents {
		result = append(result, v)
	}
	return result
}

func newEventStoreAnalyzer(cfg EventAnalyzerConfig) *eventAnalyzer {
	return &eventAnalyzer{
		cfg:        cfg,
		viewCount:  0,
		voteEvents: map[common.Uint256]string{},
	}
}
