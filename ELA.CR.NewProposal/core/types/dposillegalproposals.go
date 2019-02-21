package types

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

type ProposalEvidence struct {
	Proposal    DPosProposal
	BlockHeader Header
}

type DposIllegalProposals struct {
	Evidence        ProposalEvidence
	CompareEvidence ProposalEvidence

	hash *common.Uint256
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

func (d *DposIllegalProposals) Hash() common.Uint256 {
	if d.hash == nil {
		buf := new(bytes.Buffer)
		d.Serialize(buf)
		hash := common.Uint256(common.Sha256D(buf.Bytes()))
		d.hash = &hash
	}
	return *d.hash
}

func (d *DposIllegalProposals) GetBlockHeight() uint32 {
	return d.Evidence.BlockHeader.Height
}

func (d *DposIllegalProposals) Type() IllegalDataType {
	return IllegalProposal
}
