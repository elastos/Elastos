package payload

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

const (
	IllegalVoteVersion byte = 0x00
)

type VoteEvidence struct {
	ProposalEvidence
	Vote DPOSProposalVote
}

type DPOSIllegalVotes struct {
	Evidence        VoteEvidence
	CompareEvidence VoteEvidence

	hash *common.Uint256
}

func (d *VoteEvidence) Serialize(w io.Writer) error {
	if err := d.Vote.Serialize(w); err != nil {
		return err
	}

	if err := d.ProposalEvidence.Serialize(w); err != nil {
		return err
	}

	return nil
}

func (d *VoteEvidence) Deserialize(r io.Reader) (err error) {
	if err := d.Vote.Deserialize(r); err != nil {
		return err
	}

	if err := d.ProposalEvidence.Deserialize(r); err != nil {
		return err
	}

	return nil
}

func (d *DPOSIllegalVotes) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := d.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (d *DPOSIllegalVotes) Serialize(w io.Writer, version byte) error {
	if err := d.Evidence.Serialize(w); err != nil {
		return err
	}

	if err := d.CompareEvidence.Serialize(w); err != nil {
		return err
	}

	return nil
}

func (d *DPOSIllegalVotes) Deserialize(r io.Reader, version byte) error {
	if err := d.Evidence.Deserialize(r); err != nil {
		return err
	}

	if err := d.CompareEvidence.Deserialize(r); err != nil {
		return err
	}

	return nil
}

func (d *DPOSIllegalVotes) Hash() common.Uint256 {
	if d.hash == nil {
		buf := new(bytes.Buffer)
		d.Serialize(buf, IllegalVoteVersion)
		hash := common.Uint256(common.Sha256D(buf.Bytes()))
		d.hash = &hash
	}
	return *d.hash
}

func (d *DPOSIllegalVotes) GetBlockHeight() uint32 {
	return d.Evidence.BlockHeight
}

func (d *DPOSIllegalVotes) Type() IllegalDataType {
	return IllegalVote
}
