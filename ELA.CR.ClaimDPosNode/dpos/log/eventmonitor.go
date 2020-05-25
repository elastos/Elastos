// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package log

import (
	"time"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
)

type ProposalEvent struct {
	Sponsor      string
	BlockHash    common.Uint256
	ReceivedTime time.Time
	EndTime      time.Time
	Result       bool
	ProposalHash common.Uint256
	RawData      *payload.DPOSProposal
}

type VoteEvent struct {
	Signer       string
	ReceivedTime time.Time
	Result       bool
	RawData      *payload.DPOSProposalVote
}

type ViewEvent struct {
	OnDutyArbitrator string
	StartTime        time.Time
	Offset           uint32
	Height           uint32
}

type ConsensusEvent struct {
	StartTime time.Time
	EndTime   time.Time
	Height    uint32
	RawData   *types.Header
}

type EventListener interface {
	OnProposalArrived(prop *ProposalEvent)
	OnProposalFinished(prop *ProposalEvent)
	OnVoteArrived(vote *VoteEvent)
	OnViewStarted(view *ViewEvent)
	OnConsensusStarted(cons *ConsensusEvent)
	OnConsensusFinished(cons *ConsensusEvent)
}

type EventMonitor struct {
	listeners []EventListener
}

func NewEventMonitor() *EventMonitor {
	e := &EventMonitor{
		listeners: make([]EventListener, 0),
	}

	return e
}

func (e *EventMonitor) RegisterListener(l EventListener) {
	e.listeners = append(e.listeners, l)
}

func (e *EventMonitor) UnregisterListener(l EventListener) {
	e.listeners = e.listeners[0 : len(e.listeners)-2]
}

func (e *EventMonitor) OnProposalArrived(prop *ProposalEvent) {
	for _, l := range e.listeners {
		l.OnProposalArrived(prop)
	}
}

func (e *EventMonitor) OnProposalFinished(prop *ProposalEvent) {
	for _, l := range e.listeners {
		l.OnProposalFinished(prop)
	}
}

func (e *EventMonitor) OnVoteArrived(vote *VoteEvent) {
	for _, l := range e.listeners {
		l.OnVoteArrived(vote)
	}
}

func (e *EventMonitor) OnViewStarted(view *ViewEvent) {
	for _, l := range e.listeners {
		l.OnViewStarted(view)
	}
}

func (e *EventMonitor) OnConsensusStarted(cons *ConsensusEvent) {
	for _, l := range e.listeners {
		l.OnConsensusStarted(cons)
	}
}

func (e *EventMonitor) OnConsensusFinished(cons *ConsensusEvent) {
	for _, l := range e.listeners {
		l.OnConsensusFinished(cons)
	}
}
