package store

import (
	"time"

	"github.com/elastos/Elastos.ELA/dpos/log"
)

type EventRecord struct {
	eventStore IEventRecord

	currentConsensus   uint64
	startConsensusTime time.Time
}

func (e *EventRecord) Initialize(store IEventRecord) {
	e.eventStore = store
}

func (e *EventRecord) OnProposalArrived(prop *log.ProposalEvent) {
	if err := e.eventStore.AddProposalEvent(prop); err != nil {
		log.Error("[OnProposalArrived] err:", err.Error())
	}
}

func (e *EventRecord) OnProposalFinished(prop *log.ProposalEvent) {
	if err := e.eventStore.UpdateProposalEvent(prop); err != nil {
		log.Error("[OnProposalFinished] err:", err.Error())
	}
}

func (e *EventRecord) OnVoteArrived(vote *log.VoteEvent) {
	if err := e.eventStore.AddVoteEvent(vote); err != nil {
		log.Error("[OnVoteArrived] err:", err.Error())
	}
}

func (e *EventRecord) OnViewStarted(view *log.ViewEvent) {
	if err := e.eventStore.AddViewEvent(view); err != nil {
		log.Error("[OnViewStarted] err:", err.Error())
	}
}

func (e *EventRecord) OnConsensusStarted(cons *log.ConsensusEvent) {
	e.startConsensusTime = cons.StartTime
	if err := e.eventStore.AddConsensusEvent(cons); err != nil {
		log.Error("[OnConsensusStarted] err:", err.Error())
	}
}

func (e *EventRecord) OnConsensusFinished(cons *log.ConsensusEvent) {
	log.Info("[OnConsensusFinished] time duration:", cons.EndTime.Sub(e.startConsensusTime).Seconds())
	if err := e.eventStore.UpdateConsensusEvent(cons); err != nil {
		log.Error("[OnConsensusFinished] err:", err.Error())
	}
}
