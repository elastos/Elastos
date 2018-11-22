package store

import (
	"bytes"
	"fmt"
	"math"

	"github.com/elastos/Elastos.ELA/core"
	. "github.com/elastos/Elastos.ELA/dpos/log"
)

type EventStore struct {
	dbOperator DBOperator
}

const (
	DBDriverName = "sqlite3"
	DBFilePath   = "./vote.db"

	CreateConsensusEventTable = `CREATE TABLE IF NOT EXISTS ConsensusEvent (
				ID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
				StartTime INTEGER,
				EndTime INTEGER,
				Height INTEGER ,
				RawData BLOB
			);`
	CreateProposalEventTable = `CREATE TABLE IF NOT EXISTS ProposalEvent (
				ID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
				Proposal VARCHAR(20),
				BlockHash  BLOB,
				ReceivedTime INTEGER,
				EndTime INTEGER,
				Result INTEGER,
				RawData BLOB
			);`
	CreateVoteEventTable = `CREATE TABLE IF NOT EXISTS VoteEvent (
				ID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
				ProposalID INTEGER,
				Signer VARCHAR(20),
				ReceivedTime INTEGER,
				Result INTEGER,
				RawData BLOB
			);`
	CreateConfirmEventTable = `CREATE TABLE IF NOT EXISTS ConfirmEvent (
				ID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
				BlockHash  BLOB,
				ProposalID INTEGER,
				ReceivedTime INTEGER,
				Result INTEGER,
				RawData BLOB
			);`
	CreateViewEventTable = `CREATE TABLE IF NOT EXISTS ViewEvent (
				ID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
				ConsensusID INTEGER,
				OnDutyArbitrator VARCHAR(20) ,
				StartTime INTEGER ,
				Offset INTEGER
			);`
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
	_, result := s.dbOperator.Execute(CreateConsensusEventTable)
	return result
}

func (s *EventStore) AddConsensusEvent(cons ConsensusEvent) (uint64, error) {
	sql := "INSERT INTO ConsensusEvent(StartTime, Height, RawData) values(?,?,?)"
	id, err := s.dbOperator.Execute(sql, cons.StartTime.UnixNano(), cons.Height, cons.RawData)
	if err != nil {
		return 0, err
	}
	return id, nil
}

func (s *EventStore) UpdateConsensusEvent(cons ConsensusEvent) (uint64, error) {
	sql := "UPDATE ConsensusEvent SET EndTime=? WHERE Height=?"
	id, err := s.dbOperator.Execute(sql, cons.EndTime.UnixNano(), cons.Height)

	if err != nil {
		return 0, err
	}
	return id, nil
}

func (s *EventStore) createProposalEventTable() error {
	_, result := s.dbOperator.Execute(CreateProposalEventTable)
	return result
}

func (s *EventStore) AddProposalEvent(event ProposalEvent) (uint64, error) {
	sqlInsert := "INSERT INTO ProposalEvent(Proposal, BlockHash, ReceivedTime, Result, RawData) values(?,?,?,?,?)"
	id, err := s.dbOperator.Execute(sqlInsert, event.Proposal, event.BlockHash.Bytes(), event.ReceivedTime.UnixNano(), event.Result, event.RawData)
	if err != nil {
		return 0, err
	}
	return id, nil
}

func (s *EventStore) UpdateProposalEvent(event ProposalEvent) (uint64, error) {
	sqlUpdate := "UPDATE ProposalEvent SET EndTime=?,Result=? WHERE Proposal=? AND BlockHash=?"
	id, err := s.dbOperator.Execute(sqlUpdate, event.EndTime.UnixNano(), event.Result, event.Proposal, event.BlockHash.Bytes())
	if err != nil {
		return 0, err
	}
	return id, nil
}

func (s *EventStore) createVoteEventTable() error {
	_, result := s.dbOperator.Execute(CreateVoteEventTable)
	return result
}

func (s *EventStore) AddVoteEvent(event VoteEvent) (uint64, error) {
	vote := &core.DPosProposalVote{}
	err := vote.Deserialize(bytes.NewReader(event.RawData))
	if err != nil {
		return 0, err
	}
	w := bytes.NewBuffer(nil)
	err = vote.ProposalHash.Serialize(w)
	if err != nil {
		return 0, err
	}

	sqlSelect := "SELECT ID FROM ProposalEvent WHERE RawData=?"
	proposalId, err := s.dbOperator.Query(sqlSelect, w.Bytes())
	if err != nil {
		proposalId = math.MaxInt64
	}
	fmt.Println("[AddVoteEvent] proposalId = ", proposalId)
	sql := "INSERT INTO VoteEvent (ProposalID, Signer, ReceivedTime, Result, RawData) values(?,?,?,?,?)"
	id, err := s.dbOperator.Execute(sql, proposalId, event.Signer, event.ReceivedTime.UnixNano(), event.Result, event.RawData)
	if err != nil {
		return 0, err
	}
	return id, nil
}

func (s *EventStore) createViewEventTable() error {
	_, result := s.dbOperator.Execute(CreateViewEventTable)
	return result
}

func (s *EventStore) AddViewEvent(event ViewEvent) (uint64, error) {
	sqlSelect := "SELECT ID FROM ConsensusEvent WHERE Height=?"
	consensusId, err := s.dbOperator.Query(sqlSelect, event.Height)
	if err != nil {
		consensusId = math.MaxInt64
	}

	sqlInsert := "INSERT INTO ViewEvent(ConsensusID, OnDutyArbitrator, StartTime, Offset) values(?,?,?,?)"
	id, err := s.dbOperator.Execute(sqlInsert, consensusId, event.OnDutyArbitrator, event.StartTime.UnixNano(), event.Offset)
	if err != nil {
		return 0, err
	}
	return id, nil
}
