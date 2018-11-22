package core

import (
	"io"
)

type ProposalEvidence struct {
	Proposal    DPosProposal
	BlockHeader Header
}

type DposIllegalProposals struct {
	Evidence        ProposalEvidence
	CompareEvidence ProposalEvidence
}

func (d *ProposalEvidence) IsMatch() bool {
	return d.Proposal.BlockHash.IsEqual(d.BlockHeader.Hash())
}

func (d *ProposalEvidence) Serialize(w io.Writer) error {
	if err := d.Proposal.Serialize(w); err != nil {
		return err
	}

	if err := d.BlockHeader.Serialize(w); err != nil {
		return err
	}

	return nil
}

func (d *ProposalEvidence) Deserialize(r io.Reader) error {
	if err := d.Proposal.Deserialize(r); err != nil {
		return err
	}

	if err := d.BlockHeader.Deserialize(r); err != nil {
		return err
	}

	return nil
}

func (d *DposIllegalProposals) Serialize(w io.Writer) error {
	if err := d.Evidence.Serialize(w); err != nil {
		return err
	}

	if err := d.CompareEvidence.Serialize(w); err != nil {
		return err
	}

	return nil
}

func (d *DposIllegalProposals) Deserialize(r io.Reader) error {
	if err := d.Evidence.Deserialize(r); err != nil {
		return err
	}

	if err := d.CompareEvidence.Deserialize(r); err != nil {
		return err
	}

	return nil
}
