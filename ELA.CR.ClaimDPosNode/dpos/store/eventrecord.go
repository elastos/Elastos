package store

import (
	"time"

	"github.com/elastos/Elastos.ELA/dpos/log"
)

type EventRecord struct {
	eventStore EventStore

	currentConsensus   uint64
	startConsensusTime time.Time
}

func (e *EventRecord) Initialize() {
	e.eventStore = EventStore{dbOperator: new(LevelDBOperator)}
	err := e.eventStore.Open()
	if err != nil {
		log.Error("[Initialize] EventRecord open store failed:", err.Error())
	}
}

func (e *EventRecord) OnProposalArrived(prop *log.ProposalEvent) {
	e.eventStore.AddProposalEvent(prop)
}

func (e *EventRecord) OnProposalFinished(prop *log.ProposalEvent) {
	e.eventStore.UpdateProposalEvent(prop)
}

func (e *EventRecord) OnVoteArrived(vote *log.VoteEvent) {
	e.eventStore.AddVoteEvent(vote)
}

func (e *EventRecord) OnViewStarted(view *log.ViewEvent) {
	e.eventStore.AddViewEvent(view)
}

func (e *EventRecord) OnConsensusStarted(cons *log.ConsensusEvent) {
	e.startConsensusTime = cons.StartTime
	e.eventStore.AddConsensusEvent(cons)
}

func (e *EventRecord) OnConsensusFinished(cons *log.ConsensusEvent) {
	log.Info("[OnConsensusFinished] time duration:", cons.EndTime.Sub(e.startConsensusTime).Seconds())
	e.eventStore.UpdateConsensusEvent(cons)
}
