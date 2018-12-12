package types

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

type VoteEvidence struct {
	Vote        DPosProposalVote
	Proposal    DPosProposal
	BlockHeader Header
}

type DposIllegalVotes struct {
	Evidence        VoteEvidence
	CompareEvidence VoteEvidence

	hash *common.Uint256
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

func (d *DposIllegalVotes) Hash() common.Uint256 {
	if d.hash == nil {
		buf := new(bytes.Buffer)
		d.Serialize(buf)
		hash := common.Uint256(common.Sha256D(buf.Bytes()))
		d.hash = &hash
	}
	return *d.hash
}

func (d *DposIllegalVotes) GetBlockHeight() uint32 {
	return d.Evidence.BlockHeader.Height
}

func (d *DposIllegalVotes) Type() IllegalDataType {
	return IllegalVote
}
