package cs

import (
	"io"
	"time"

	"github.com/elastos/Elastos.ELA/core"
	. "github.com/elastos/Elastos.ELA/dpos/arbitration/common"
	"github.com/elastos/Elastos.ELA/dpos/chain"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type ConsensusStatus struct {
	ProcessingBlock core.Block
	ConsensusStatus uint32
	ViewOffset      uint32
	ViewStartTime   time.Time

	AcceptVotes      []DPosProposalVote
	RejectedVotes    []DPosProposalVote
	PendingProposals []DPosProposal

	MissingBlocks        []*core.Block
	MissingBlockConfirms []*chain.ProposalVoteSlot
}

func (s *ConsensusStatus) Serialize(w io.Writer) error {
	if err := s.ProcessingBlock.Serialize(w); err != nil {
		return err
	}

	if err := common.WriteUint32(w, s.ConsensusStatus); err != nil {
		return err
	}

	if err := common.WriteUint32(w, s.ViewOffset); err != nil {
		return err
	}

	if err := common.WriteUint64(w, uint64(s.ViewStartTime.UnixNano())); err != nil {
		return err
	}

	if err := common.WriteVarUint(w, uint64(len(s.AcceptVotes))); err != nil {
		return nil
	}
	for _, v := range s.AcceptVotes {
		if err := v.Serialize(w); err != nil {
			return err
		}
	}

	if err := common.WriteVarUint(w, uint64(len(s.RejectedVotes))); err != nil {
		return err
	}
	for _, v := range s.RejectedVotes {
		if err := v.Serialize(w); err != nil {
			return err
		}
	}

	if err := common.WriteVarUint(w, uint64(len(s.PendingProposals))); err != nil {
		return err
	}
	for _, v := range s.PendingProposals {
		if err := v.Serialize(w); err != nil {
			return err
		}
	}

	if err := common.WriteVarUint(w, uint64(len(s.MissingBlocks))); err != nil {
		return err
	}
	for _, v := range s.MissingBlocks {
		if err := v.Serialize(w); err != nil {
			return err
		}
	}

	if err := common.WriteVarUint(w, uint64(len(s.MissingBlockConfirms))); err != nil {
		return err
	}
	for _, v := range s.MissingBlockConfirms {
		if err := v.Serialize(w); err != nil {
			return err
		}
	}

	return nil
}

func (s *ConsensusStatus) Deserialize(r io.Reader) error {
	var err error
	if err = s.ProcessingBlock.Deserialize(r); err != nil {
		return err
	}

	if s.ConsensusStatus, err = common.ReadUint32(r); err != nil {
		return err
	}

	if s.ViewOffset, err = common.ReadUint32(r); err != nil {
		return err
	}

	var startTimeUnix uint64
	if startTimeUnix, err = common.ReadUint64(r); err != nil {
		return err
	}
	s.ViewStartTime = time.Unix(0, int64(startTimeUnix))

	var arrayLength uint64
	if arrayLength, err = common.ReadVarUint(r, 0); err != nil {
		return nil
	}
	s.AcceptVotes = make([]DPosProposalVote, arrayLength)
	for i := uint64(0); i < arrayLength; i++ {
		if err = s.AcceptVotes[i].Deserialize(r); err != nil {
			return err
		}
	}

	if arrayLength, err = common.ReadVarUint(r, 0); err != nil {
		return err
	}
	s.RejectedVotes = make([]DPosProposalVote, arrayLength)
	for i := uint64(0); i < arrayLength; i++ {
		if err = s.RejectedVotes[i].Deserialize(r); err != nil {
			return err
		}
	}

	if arrayLength, err = common.ReadVarUint(r, 0); err != nil {
		return err
	}
	s.PendingProposals = make([]DPosProposal, arrayLength)
	for i := uint64(0); i < arrayLength; i++ {
		if err = s.PendingProposals[i].Deserialize(r); err != nil {
			return err
		}
	}

	if arrayLength, err = common.ReadVarUint(r, 0); err != nil {
		return err
	}
	s.MissingBlocks = make([]*core.Block, arrayLength)
	for i := uint64(0); i < arrayLength; i++ {
		s.MissingBlocks[i] = &core.Block{}
		if err = s.MissingBlocks[i].Deserialize(r); err != nil {
			return err
		}
	}

	if arrayLength, err = common.ReadVarUint(r, 0); err != nil {
		return err
	}
	s.MissingBlockConfirms = make([]*chain.ProposalVoteSlot, arrayLength)
	for i := uint64(0); i < arrayLength; i++ {
		s.MissingBlockConfirms[i] = &chain.ProposalVoteSlot{}
		if err = s.MissingBlockConfirms[i].Deserialize(r); err != nil {
			return err
		}
	}

	return nil
}
