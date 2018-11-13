package store

import (
	"bytes"
	"database/sql"
	"testing"
	"time"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA/dpos/log"
)

func InitEventStore() *EventStore {
	eventStore := &EventStore{
		dbOperator: &SqlDBOperator{
			db: new(sql.DB),
		},
	}
	return eventStore
}

func TestEventStore_Open(t *testing.T) {
	eventStore := InitEventStore()
	err := eventStore.Open()
	defer eventStore.Close()
	if err != nil {
		t.Error("open database failed!")
	}

}

func TestEventStore_Close(t *testing.T) {
	eventStore := InitEventStore()
	err := eventStore.Open()
	if err != nil {
		t.Error("open database failed!")
	}

	err = eventStore.Close()
	if err != nil {
		t.Error("close database failed!")
	}
}

func TestEventStore_AddProposalEvent(t *testing.T) {
	eventStore := InitEventStore()
	eventStore.Open()
	defer eventStore.Close()

	err := eventStore.createProposalEventTable()
	if err != nil {
		t.Error("create proposal event table failed!")
	}

	proposal := &msg.DPosProposal{
		Sponsor:   "B",
		BlockHash: common.Uint256{2},
		Sign:      []byte{1, 2, 3},
	}

	buf := new(bytes.Buffer)
	proposal.Serialize(buf)

	proposalEvent := log.ProposalEvent{
		Proposal:     "A",
		BlockHash:    common.Uint256{},
		ReceivedTime: time.Time{},
		EndTime:      time.Time{},
		Result:       false,
		RawData:      buf.Bytes(),
	}
	id, err := eventStore.AddProposalEvent(proposalEvent)

	if id != 1 {
		t.Errorf("add proposal event failed! got %d, expected 1", id)
	}

	if err != nil {
		t.Error("add proposal event data failed!")
	}
}

func TestEventStore_UpdateProposalEvent(t *testing.T) {
	eventStore := InitEventStore()
	eventStore.Open()
	defer eventStore.Close()

	proposalEvent := log.ProposalEvent{
		Proposal:     "A",
		BlockHash:    common.Uint256{},
		ReceivedTime: time.Time{},
		EndTime:      time.Now(),
		Result:       true,
		RawData:      nil,
	}
	_, err := eventStore.UpdateProposalEvent(proposalEvent)
	if err != nil {
		t.Error("update proposal event data failed!")
	}
}

func TestEventStore_AddConsensusEvent(t *testing.T) {
	eventStore := InitEventStore()
	eventStore.Open()
	defer eventStore.Close()

	err := eventStore.createConsensusEventTable()
	if err != nil {
		t.Error("create consensus store table failed!")
	}

	cons := log.ConsensusEvent{
		StartTime: time.Time{},
		Height:    0,
		RawData:   []byte{1},
	}
	id, err := eventStore.AddConsensusEvent(cons)

	if id != 1 {
		t.Errorf("add consensus event failed! got %d, expected 1", id)
	}

	if err != nil {
		t.Error("add  event data failed!")
	}

}

func TestEventStore_UpdateConsensusEvent(t *testing.T) {
	eventStore := InitEventStore()
	eventStore.Open()
	defer eventStore.Close()
	cons := log.ConsensusEvent{
		StartTime: time.Time{},
		Height:    0,
		RawData:   []byte{1},
	}
	_, err := eventStore.UpdateConsensusEvent(cons)
	if err != nil {
		t.Error("update consensus event data failed!")
	}
}

func TestEventStore_AddViewEvent(t *testing.T) {
	eventStore := InitEventStore()
	eventStore.Open()
	defer eventStore.Close()

	err := eventStore.createViewEventTable()
	if err != nil {
		t.Error("create view event table failed!")
	}

	viewEvent := log.ViewEvent{
		OnDutyArbitrator: "A",
		StartTime:        time.Time{},
		Offset:           0,
		Height:           0,
	}

	id, err := eventStore.AddViewEvent(viewEvent)
	if id != 1 {
		t.Errorf("add view event failed! got %d, expected 1", id)
	}

	if err != nil {
		t.Error("ad view event failed!")
	}
}

func TestEventStore_AddVoteEvent(t *testing.T) {
	eventStore := InitEventStore()
	eventStore.Open()
	defer eventStore.Close()

	err := eventStore.createVoteEventTable()
	if err != nil {
		t.Error("create vote event table failed!")
	}

	vote := &msg.DPosProposalVote{
		Proposal: msg.DPosProposal{
			Sponsor:   "B",
			BlockHash: common.Uint256{2},
			Sign:      []byte{1, 2, 3},
		},
		Signer: "A",
		Accept: false,
		Sign:   []byte{1, 2, 3},
	}

	buf := new(bytes.Buffer)
	vote.Serialize(buf)

	voteEvent := log.VoteEvent{
		Signer:       "A",
		ReceivedTime: time.Time{},
		Result:       false,
		RawData:      buf.Bytes(),
	}

	id, err := eventStore.AddVoteEvent(voteEvent)

	if id != 1 {
		t.Errorf("add vote event failed, got %d, expected 1", id)
	}

	if err != nil {
		t.Error("add vote event failed, got error: ", err)

	}
}
