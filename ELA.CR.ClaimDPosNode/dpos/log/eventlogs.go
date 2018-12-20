package log

type EventLogs struct {
}

func (e *EventLogs) OnProposalArrived(prop *ProposalEvent) {
	Info("[OnProposalArrived], Proposal:", prop.Proposal,
		"BlockHash:", prop.BlockHash, "ReceivedTime:", prop.ReceivedTime, "Result:", prop.Result)
}

func (e *EventLogs) OnProposalFinished(prop *ProposalEvent) {
	Info("[OnProposalFinished], Proposal:", prop.Proposal,
		"BlockHash:", prop.BlockHash, "EndTime:", prop.EndTime, "Result:", prop.Result)
}

func (e *EventLogs) OnVoteArrived(vote *VoteEvent) {
	Info("[OnVoteArrived], Signer:", vote.Signer, "ReceivedTime:", vote.ReceivedTime, "Result:", vote.Result)
}

func (e *EventLogs) OnViewStarted(view *ViewEvent) {
	Info("[OnViewStarted], OnDutyArbitrator:", view.OnDutyArbitrator,
		"StartTime:", view.StartTime, "Offset:", view.Offset, "Height", view.Height)
}

func (e *EventLogs) OnConsensusStarted(cons *ConsensusEvent) {
	Info("[OnConsensusStarted], StartTime:", cons.StartTime, "Height:", cons.Height)
}

func (e *EventLogs) OnConsensusFinished(cons *ConsensusEvent) {
	Info("[OnConsensusFinished], EndTime:", cons.EndTime, "Height:", cons.Height)
}
