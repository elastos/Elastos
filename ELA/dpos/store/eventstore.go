package store

import (
	"bytes"
	"fmt"
	"math"

	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/types"
	. "github.com/elastos/Elastos.ELA/dpos/log"
	"time"
)

var ConsensusEventTable = &DposTable{
	Name:       "ConsensusEvent",
	PrimaryKey: 4,
	Indexes:    nil,
	Fields: []string{
		"StartTime",
		"EndTime",
		"Height",
		"RawData",
	},
}

var ProposalEventTable = &DposTable{
	Name:       "ProposalEvent",
	PrimaryKey: 6,
	Indexes:    nil,
	Fields: []string{
		"Proposal",
		"BlockHash",
		"ReceivedTime",
		"EndTime",
		"Result",
		"RawData",
	},
}

var VoteEventTable = &DposTable{
	Name:       "VoteEvent",
	PrimaryKey: 0,
	Indexes:    nil,
	Fields: []string{
		"ProposalID",
		"Signer",
		"ReceivedTime",
		"Result",
		"RawData",
	},
}

var ViewEventTable = &DposTable{
	Name:       "ViewEvent",
	PrimaryKey: 0,
	Indexes:    nil,
	Fields: []string{
		"ConsensusID",
		"OnDutyArbitrator",
		"StartTime",
		"Offset",
	},
}

const (
	DBFilePath         = "DposEvent"
	MaxEvnetTaskNumber = 10000
)

type persistTask interface{}

type addConsensusEventTask struct {
	event *ConsensusEvent
	reply chan bool
}

type updateConsensusEventTask struct {
	event *ConsensusEvent
	reply chan bool
}

type addProposalEventTask struct {
	event *ProposalEvent
	reply chan bool
}

type updateProposalEventTask struct {
	event *ProposalEvent
	reply chan bool
}

type addVoteEventTask struct {
	event *VoteEvent
	reply chan bool
}

type addViewEventTask struct {
	event *ViewEvent
	reply chan bool
}

type EventStore struct {
	dbOperator DBOperator

	taskCh chan persistTask
	quit   chan chan bool
}

func (s *EventStore) loop() {
	for {
		select {
		case t := <-s.taskCh:
			now := time.Now()
			switch task := t.(type) {
			case *addConsensusEventTask:
				s.handleAddConsensusEvent(task.event)
				task.reply <- true
				tcall := float64(time.Now().Sub(now)) / float64(time.Second)
				Debugf("handle add consensus event task exetime: %g", tcall)
			case *updateConsensusEventTask:
				s.handleUpdateConsensusEvent(task.event)
				task.reply <- true
				tcall := float64(time.Now().Sub(now)) / float64(time.Second)
				Debugf("handle update consensus event task exetime: %g", tcall)
			case *addProposalEventTask:
				s.handleAddProposalEvent(task.event)
				task.reply <- true
				tcall := float64(time.Now().Sub(now)) / float64(time.Second)
				Debugf("handle add proposal event task exetime: %g", tcall)
			case *updateProposalEventTask:
				s.handleUpdateProposalEvent(task.event)
				task.reply <- true
				tcall := float64(time.Now().Sub(now)) / float64(time.Second)
				Debugf("handle update proposal event task exetime: %g", tcall)
			case *addVoteEventTask:
				s.handleVoteProposalEvent(task.event)
				task.reply <- true
				tcall := float64(time.Now().Sub(now)) / float64(time.Second)
				Debugf("handle add vote event task exetime: %g", tcall)
			case *addViewEventTask:
				s.handleViewProposalEvent(task.event)
				task.reply <- true
				tcall := float64(time.Now().Sub(now)) / float64(time.Second)
				Debugf("handle add view event task exetime: %g", tcall)
			}

		case closed := <-s.quit:
			closed <- true
			return
		}
	}
}

func (s *EventStore) handleAddConsensusEvent(cons *ConsensusEvent) {
	rowID, err := s.addConsensusEvent(cons)
	if err != nil {
		log.Error("add consensus event failed:", err.Error())
	}
	log.Info("add consensus event succeed row id:", rowID)
}

func (s *EventStore) handleUpdateConsensusEvent(cons *ConsensusEvent) {
	_, err := s.updateConsensusEvent(cons)
	if err != nil {
		log.Error("update consensus event failed:", err.Error())
	}
}

func (s *EventStore) handleAddProposalEvent(prop *ProposalEvent) {
	rowID, err := s.addProposalEvent(prop)
	if err != nil {
		log.Error("add proposal event failed:", err.Error())
	}
	log.Info("add proposal event succeed at row id:", rowID)
}

func (s *EventStore) handleUpdateProposalEvent(prop *ProposalEvent) {
	_, err := s.updateProposalEvent(prop)
	if err != nil {
		log.Error("update proposal event failed:", err.Error())
	}
}

func (s *EventStore) handleVoteProposalEvent(vote *VoteEvent) {
	rowID, err := s.addVoteEvent(vote)
	if err != nil {
		log.Error("add vote event failed:", err.Error())
	}
	log.Info("add vote event succeed at row id:", rowID)
}

func (s *EventStore) handleViewProposalEvent(view *ViewEvent) {
	rowID, err := s.addViewEvent(view)
	if err != nil {
		log.Error("add view event failed:", err.Error())
	}
	log.Info("add view event succeed at row id:", rowID)
}

func (s *EventStore) Open() error {
	err := s.dbOperator.InitConnection(DBFilePath)
	if err != nil {
		return err
	}
	err = s.dbOperator.Connect()
	if err != nil {
		return fmt.Errorf("database connect failed:", err.Error())
	}
	err = s.createConsensusEventTable()
	if err != nil {
		return fmt.Errorf("create ConsensusEvent table Connect failed:", err.Error())
	}
	err = s.createProposalEventTable()
	if err != nil {
		return fmt.Errorf("create ProposalEvent table failed:", err.Error())
	}
	err = s.createVoteEventTable()
	if err != nil {
		return fmt.Errorf("create VoteEvent table failed:", err.Error())
	}
	err = s.createViewEventTable()
	if err != nil {
		return fmt.Errorf("create ViewEvent table failed:", err.Error())
	}

	s.taskCh = make(chan persistTask, MaxEvnetTaskNumber)
	s.quit = make(chan chan bool, 1)

	go s.loop()

	return nil
}

func (s *EventStore) Close() error {
	closed := make(chan bool)
	s.quit <- closed
	<-closed
	return s.dbOperator.Disconnect()
}

func (s *EventStore) createConsensusEventTable() error {
	result := s.dbOperator.Create(ConsensusEventTable)
	return result
}

func (s *EventStore) AddConsensusEvent(cons *ConsensusEvent) {
	reply := make(chan bool)
	s.taskCh <- &addConsensusEventTask{event: cons, reply: reply}
	<-reply
}

func (s *EventStore) addConsensusEvent(cons *ConsensusEvent) (uint64, error) {
	return s.dbOperator.Insert(ConsensusEventTable, []*Field{
		{"StartTime", cons.StartTime.UnixNano()},
		{"Height", cons.Height},
		{"RawData", cons.RawData},
	})
}

func (s *EventStore) UpdateConsensusEvent(cons *ConsensusEvent) {
	reply := make(chan bool)
	s.taskCh <- &updateConsensusEventTask{event: cons, reply: reply}
	<-reply
}

func (s *EventStore) updateConsensusEvent(cons *ConsensusEvent) ([]uint64, error) {
	return s.dbOperator.Update(ConsensusEventTable, []*Field{
		{"Height", cons.Height}}, []*Field{
		{"EndTime", cons.EndTime.UnixNano()}})
}

func (s *EventStore) createProposalEventTable() error {
	return s.dbOperator.Create(ProposalEventTable)
}

func (s *EventStore) AddProposalEvent(event *ProposalEvent) {
	reply := make(chan bool)
	s.taskCh <- &addProposalEventTask{event: event, reply: reply}
	<-reply
}

func (s *EventStore) addProposalEvent(event *ProposalEvent) (uint64, error) {
	return s.dbOperator.Insert(ProposalEventTable, []*Field{
		{"Proposal", event.Proposal},
		{"BlockHash", event.BlockHash.Bytes()},
		{"ReceivedTime", event.ReceivedTime.UnixNano()},
		{"Result", event.Result},
		{"RawData", event.RawData},
	})
}
func (s *EventStore) UpdateProposalEvent(event *ProposalEvent) {
	reply := make(chan bool)
	s.taskCh <- &updateProposalEventTask{event: event, reply: reply}
	<-reply
}

func (s *EventStore) updateProposalEvent(event *ProposalEvent) ([]uint64, error) {
	return s.dbOperator.Update(ProposalEventTable, []*Field{
		{"Proposal", event.Proposal},
		{"BlockHash", event.BlockHash.Bytes()},
	}, []*Field{
		{"EndTime", event.EndTime.UnixNano()},
		{"Result", event.Result},
	})
}

func (s *EventStore) createVoteEventTable() error {
	result := s.dbOperator.Create(VoteEventTable)
	return result
}

func (s *EventStore) AddVoteEvent(event *VoteEvent) {
	reply := make(chan bool)
	s.taskCh <- &addVoteEventTask{event: event, reply: reply}
	<-reply
}

func (s *EventStore) addVoteEvent(event *VoteEvent) (uint64, error) {
	vote := &types.DPosProposalVote{}
	err := vote.Deserialize(bytes.NewReader(event.RawData))
	if err != nil {
		return 0, err
	}
	w := bytes.NewBuffer(nil)
	err = vote.ProposalHash.Serialize(w)
	if err != nil {
		return 0, err
	}

	var proposalId uint64
	rowIDs, err := s.dbOperator.SelectID(ProposalEventTable, []*Field{
		&Field{"RawData", w.Bytes()},
	})
	if err != nil || len(rowIDs) != 1 {
		proposalId = math.MaxInt64
	} else {
		proposalId = rowIDs[0]
	}

	fmt.Println("[AddVoteEvent] proposalId = ", proposalId)
	return s.dbOperator.Insert(VoteEventTable, []*Field{
		&Field{"ProposalID", proposalId},
		&Field{"Signer", event.Signer},
		&Field{"ReceivedTime", event.ReceivedTime.UnixNano()},
		&Field{"Result", event.Result},
		&Field{"RawData", event.RawData},
	})
}

func (s *EventStore) createViewEventTable() error {
	result := s.dbOperator.Create(ViewEventTable)
	return result
}

func (s *EventStore) AddViewEvent(event *ViewEvent) {
	reply := make(chan bool)
	s.taskCh <- &addViewEventTask{event: event, reply: reply}
	<-reply
}

func (s *EventStore) addViewEvent(event *ViewEvent) (uint64, error) {
	var consensusId uint64
	rowIDs, err := s.dbOperator.SelectID(ConsensusEventTable, []*Field{
		&Field{"Height", event.Height},
	})
	if err != nil || len(rowIDs) != 1 {
		consensusId = math.MaxInt64
	} else {
		consensusId = rowIDs[0]
	}

	return s.dbOperator.Insert(ViewEventTable, []*Field{
		&Field{"ConsensusID", consensusId},
		&Field{"OnDutyArbitrator", event.OnDutyArbitrator},
		&Field{"StartTime", event.StartTime.UnixNano()},
		&Field{"Offset", event.Offset},
	})
}
