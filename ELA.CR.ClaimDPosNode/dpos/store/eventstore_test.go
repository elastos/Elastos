package store

import (
	"crypto/rand"
	"testing"
	"time"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/dpos/log"
	"github.com/elastos/Elastos.ELA/utils/test"
)

var eventStore *DposStore

//fixme clean event store for next unit test
func TestEventStore_Open(t *testing.T) {
	log.Init(0, 20, 100)

	store, err := NewDposStore(test.DataPath)
	if err != nil {
		t.Error("open database failed:", err.Error())
	}

	store.StartEventRecord()
	eventStore = store
}

func TestEventStore_AddProposalEvent(t *testing.T) {
	proposal := &payload.DPOSProposal{
		Sponsor:    randomPkBytes(),
		BlockHash:  common.Uint256{2},
		Sign:       []byte{1, 2, 3},
		ViewOffset: 0,
	}

	proposalEvent := &log.ProposalEvent{
		Sponsor:      "A",
		BlockHash:    common.Uint256{},
		ReceivedTime: time.Time{},
		EndTime:      time.Time{},
		Result:       false,
		ProposalHash: common.Uint256{1, 2, 3},
		RawData:      proposal,
	}
	id, err := eventStore.addProposalEvent(proposalEvent)
	if id != 1 {
		t.Errorf("add proposal event failed! got %d, expected 1", id)
	}

	if err != nil {
		t.Error("add proposal event data failed!")
	}
}

func TestEventStore_UpdateProposalEvent(t *testing.T) {
	proposalEvent := &log.ProposalEvent{
		Sponsor:      "A",
		BlockHash:    common.Uint256{},
		ReceivedTime: time.Time{},
		EndTime:      time.Now(),
		Result:       true,
		RawData:      nil,
	}
	_, err := eventStore.updateProposalEvent(proposalEvent)
	if err != nil {
		t.Error("update proposal event data failed, err:", err.Error())
	}
}

func TestEventStore_AddConsensusEvent(t *testing.T) {
	cons := &log.ConsensusEvent{
		StartTime: time.Time{},
		Height:    0,
		RawData:   &types.Header{},
	}
	id, err := eventStore.addConsensusEvent(cons)

	if id != 1 {
		t.Errorf("add consensus event failed! got %d, expected 1", id)
	}

	if err != nil {
		t.Error("add  event data failed!")
	}

}

func TestEventStore_UpdateConsensusEvent(t *testing.T) {
	cons := &log.ConsensusEvent{
		StartTime: time.Time{},
		Height:    0,
		RawData:   &types.Header{},
	}
	_, err := eventStore.updateConsensusEvent(cons)
	if err != nil {
		t.Error("update consensus event data failed, err:", err.Error())
	}
}

func TestEventStore_AddViewEvent(t *testing.T) {
	viewEvent := &log.ViewEvent{
		OnDutyArbitrator: "A",
		StartTime:        time.Time{},
		Offset:           0,
		Height:           0,
	}

	id, err := eventStore.addViewEvent(viewEvent)
	if id != 1 {
		t.Errorf("add view event failed! got %d, expected 1", id)
	}

	if err != nil {
		t.Error("ad view event failed!")
	}
}

func TestEventStore_AddVoteEvent(t *testing.T) {
	vote := &payload.DPOSProposalVote{
		ProposalHash: common.Uint256{1, 2, 3},
		Signer:       randomPkBytes(),
		Accept:       false,
		Sign:         []byte{1, 2, 3},
	}

	voteEvent := &log.VoteEvent{
		Signer:       "A",
		ReceivedTime: time.Time{},
		Result:       false,
		RawData:      vote,
	}

	id, err := eventStore.addVoteEvent(voteEvent)

	if id != 1 {
		t.Errorf("add vote event failed, got %d, expected 1", id)
	}

	if err != nil {
		t.Error("add vote event failed, got error: ", err)

	}
}

func TestEventStore_Close(t *testing.T) {
	eventStore.deleteTable(ProposalEventTable)
	eventStore.deleteTable(ConsensusEventTable)
	eventStore.deleteTable(VoteEventTable)
	eventStore.deleteTable(ViewEventTable)
	eventStore.Close()
}

func randomPkBytes() []byte {
	pk := make([]byte, 33)
	rand.Read(pk)

	return pk
}
