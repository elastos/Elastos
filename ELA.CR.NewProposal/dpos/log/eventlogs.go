package log

import (
	"fmt"
)

type EventLogs struct {
}

func (e *EventLogs) OnProposalArrived(prop *ProposalEvent) {
	Info(fmt.Sprintf("[OnProposalArrived] "+
		"Proposal: %s, "+
		"BlockHash: %s, "+
		"ReceivedTime: %s, "+
		"Result: %t, "+
		"ProposalHash: %s",
		prop.Proposal,
		prop.BlockHash,
		prop.ReceivedTime,
		prop.Result,
		prop.ProposalHash))
}

func (e *EventLogs) OnProposalFinished(prop *ProposalEvent) {
	Info(fmt.Sprintf("[OnProposalFinished] "+
		"Proposal: %s, "+
		"BlockHash: %s, "+
		"EndTime: %s, "+
		"Result: %t, "+
		"ProposalHash: %s",
		prop.Proposal,
		prop.BlockHash,
		prop.EndTime,
		prop.Result,
		prop.ProposalHash))
}

func (e *EventLogs) OnVoteArrived(vote *VoteEvent) {
	Info(fmt.Sprintf("[OnVoteArrived] "+
		"Signer: %s, "+
		"ReceivedTime: %s, "+
		"Result: %t",
		vote.Signer,
		vote.ReceivedTime,
		vote.Result))
}

func (e *EventLogs) OnViewStarted(view *ViewEvent) {
	Info(fmt.Sprintf("[OnViewStarted] "+
		"OnDutyArbitrator: %s, "+
		"StartTime: %s, "+
		"Offset: %d, "+
		"Height: %d",
		view.OnDutyArbitrator,
		view.StartTime,
		view.Offset,
		view.Height))
}

func (e *EventLogs) OnConsensusStarted(cons *ConsensusEvent) {
	Info(fmt.Sprintf("[OnConsensusStarted] "+
		"StartTime: %s, "+
		"Height: %d",
		cons.StartTime,
		cons.Height))
}

func (e *EventLogs) OnConsensusFinished(cons *ConsensusEvent) {
	Info(fmt.Sprintf("[OnConsensusFinished] "+
		"EndTime: %s, "+
		"Height: %d",
		cons.EndTime,
		cons.Height))
}
