package core

import "io"

type VoteEvidence struct {
	Vote        DPosProposalVote
	Proposal    DPosProposal
	BlockHeader Header
}

type DposIllegalVotes struct {
	Evidence        VoteEvidence
	CompareEvidence VoteEvidence
}

func (d *VoteEvidence) IsMatch() bool {
	return d.Proposal.BlockHash.IsEqual(d.BlockHeader.Hash()) &&
		d.Vote.ProposalHash.IsEqual(d.Proposal.Hash())
}

func (d *VoteEvidence) Serialize(w io.Writer) error {
	if err := d.Vote.Serialize(w); err != nil {
		return err
	}

	if err := d.Proposal.Serialize(w); err != nil {
		return err
	}

	if err := d.BlockHeader.Serialize(w); err != nil {
		return err
	}

	return nil
}

func (d *VoteEvidence) Deserialize(r io.Reader) error {
	if err := d.Vote.Deserialize(r); err != nil {
		return err
	}

	if err := d.Proposal.Deserialize(r); err != nil {
		return err
	}

	if err := d.BlockHeader.Deserialize(r); err != nil {
		return err
	}

	return nil
}

func (d *DposIllegalVotes) Serialize(w io.Writer) error {
	if err := d.Evidence.Serialize(w); err != nil {
		return err
	}

	if err := d.CompareEvidence.Serialize(w); err != nil {
		return err
	}

	return nil
}

func (d *DposIllegalVotes) Deserialize(r io.Reader) error {
	if err := d.Evidence.Deserialize(r); err != nil {
		return err
	}

	if err := d.CompareEvidence.Deserialize(r); err != nil {
		return err
	}

	return nil
}
