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
	e.eventStore = EventStore{
		dbOperator: new(SqlDBOperator),
	}
	err := e.eventStore.Open()
	if err != nil {
		log.Error("[Initialize] EventRecord open store failed:", err.Error())
	}
}

func (e *EventRecord) OnProposalArrived(prop log.ProposalEvent) {
	_, err := e.eventStore.AddProposalEvent(prop)
	if err != nil {
		log.Error("[OnProposalArrived], Add message failed:", err.Error())
		return
	}
}

func (e *EventRecord) OnProposalFinished(prop log.ProposalEvent) {
	_, err := e.eventStore.UpdateProposalEvent(prop)
	if err != nil {
		log.Error("[OnProposalFinished], Add message failed:", err.Error())
		return
	}
}

func (e *EventRecord) OnVoteArrived(vote log.VoteEvent) {
	_, err := e.eventStore.AddVoteEvent(vote)
	if err != nil {
		log.Error("[OnVoteArrived], Add message failed:", err.Error())
		return
	}
}

func (e *EventRecord) OnViewStarted(view log.ViewEvent) {
	_, err := e.eventStore.AddViewEvent(view)
	if err != nil {
		log.Error("[OnViewStarted], Add message failed:", err.Error())
		return
	}
}

func (e *EventRecord) OnConsensusStarted(cons log.ConsensusEvent) {
	e.startConsensusTime = cons.StartTime
	id, err := e.eventStore.AddConsensusEvent(cons)
	if err != nil {
		log.Error("[OnConsensusStarted], Add message failed:", err.Error())
		return
	}
	e.currentConsensus = id
}

func (e *EventRecord) OnConsensusFinished(cons log.ConsensusEvent) {
	log.Info("[OnConsensusFinished] time duration:", cons.EndTime.Sub(e.startConsensusTime).Seconds())
	_, err := e.eventStore.UpdateConsensusEvent(cons)
	if err != nil {
		log.Error("[OnConsensusFinished], Add message failed:", err.Error())
		return
	}
}
