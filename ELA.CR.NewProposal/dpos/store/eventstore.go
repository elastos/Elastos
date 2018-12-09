package store

import (
	"bytes"
	"fmt"
	"math"

	"github.com/elastos/Elastos.ELA/core/types"
	. "github.com/elastos/Elastos.ELA/dpos/log"
)

type EventStore struct {
	dbOperator DBOperator
}

var ConsensusEventTable = &DposTable{
	Name:       "ConsensusEvent",
	PrimaryKey: 0,
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
	PrimaryKey: 0,
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
	DBDriverName = "sqlite3"
	DBFilePath   = "./vote.db"
)

func (s *EventStore) Open() error {
	err := s.dbOperator.InitConnection(DBDriverName, DBFilePath)
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
	return err
}

func (s *EventStore) Close() error {
	//todo complete me
	err := s.dbOperator.Disconnect()
	return err
}

func (s *EventStore) createConsensusEventTable() error {
	result := s.dbOperator.Create(ConsensusEventTable)
	return result
}

func (s *EventStore) AddConsensusEvent(cons ConsensusEvent) (uint64, error) {
	id, err := s.dbOperator.Insert(ConsensusEventTable, []*Field{
		{"StartTime", cons.StartTime.UnixNano()},
		{"Height", cons.Height},
		{"RawData", cons.RawData},
	})
	if err != nil {
		return 0, err
	}
	return id, nil
}

func (s *EventStore) UpdateConsensusEvent(cons ConsensusEvent) ([]uint64, error) {
	ids, err := s.dbOperator.Update(ConsensusEventTable, []*Field{
		{"Height", cons.Height}}, []*Field{
		{"EndTime", cons.EndTime.UnixNano()}})
	if err != nil {
		return nil, err
	}
	return ids, nil
}

func (s *EventStore) createProposalEventTable() error {
	result := s.dbOperator.Create(ProposalEventTable)
	return result
}

func (s *EventStore) AddProposalEvent(event ProposalEvent) (uint64, error) {
	id, err := s.dbOperator.Insert(ProposalEventTable, []*Field{
		{"Proposal", event.Proposal},
		{"BlockHash", event.BlockHash.Bytes()},
		{"ReceivedTime", event.ReceivedTime.UnixNano()},
		{"Result", event.Result},
		{"RawData", event.RawData},
	})
	if err != nil {
		return 0, err
	}
	return id, nil
}

func (s *EventStore) UpdateProposalEvent(event ProposalEvent) ([]uint64, error) {
	ids, err := s.dbOperator.Update(ProposalEventTable, []*Field{
		{"Proposal", event.Proposal},
		{"BlockHash", event.BlockHash.Bytes()},
	}, []*Field{
		{"EndTime", event.EndTime.UnixNano()},
		{"Result", event.Result},
	})
	if err != nil {
		return nil, err
	}
	return ids, nil
}

func (s *EventStore) createVoteEventTable() error {
	result := s.dbOperator.Create(VoteEventTable)
	return result
}

func (s *EventStore) AddVoteEvent(event VoteEvent) (uint64, error) {
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
	id, err := s.dbOperator.Insert(VoteEventTable, []*Field{
		&Field{"ProposalID", proposalId},
		&Field{"Signer", event.Signer},
		&Field{"ReceivedTime", event.ReceivedTime.UnixNano()},
		&Field{"Result", event.Result},
		&Field{"RawData", event.RawData},
	})
	if err != nil {
		return 0, err
	}
	return id, nil
}

func (s *EventStore) createViewEventTable() error {
	result := s.dbOperator.Create(ViewEventTable)
	return result
}

func (s *EventStore) AddViewEvent(event ViewEvent) (uint64, error) {
	var consensusId uint64
	rowIDs, err := s.dbOperator.SelectID(ConsensusEventTable, []*Field{
		&Field{"Height", event.Height},
	})
	if err != nil || len(rowIDs) != 1 {
		consensusId = math.MaxInt64
	} else {
		consensusId = rowIDs[0]
	}

	id, err := s.dbOperator.Insert(ViewEventTable, []*Field{
		&Field{"ConsensusID", consensusId},
		&Field{"OnDutyArbitrator", event.OnDutyArbitrator},
		&Field{"StartTime", event.StartTime.UnixNano()},
		&Field{"Offset", event.Offset},
	})
	if err != nil {
		return 0, err
	}
	return id, nil
}
