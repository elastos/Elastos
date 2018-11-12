package common

import (
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type DPosProposalVote struct {
	Proposal DPosProposal

	Signer string
	Accept bool
	Sign   []byte
}

func (v *DPosProposalVote) SignVote() []byte {
	return []byte{2}
}

func (v *DPosProposalVote) IsValid() bool {
	//todo [merge] verify signature
	return true
}

func (v *DPosProposalVote) SerializeUnsigned(w io.Writer) error {
	if err := v.Proposal.Serialize(w); err != nil {
		return err
	}

	if err := common.WriteVarString(w, v.Signer); err != nil {
		return err
	}

	var acceptValue uint8
	if v.Accept {
		acceptValue = 1
	} else {
		acceptValue = 0
	}
	if err := common.WriteUint8(w, acceptValue); err != nil {
		return err
	}

	return nil
}

func (v *DPosProposalVote) Serialize(w io.Writer) error {
	if err := v.SerializeUnsigned(w); err != nil {
		return err
	}

	return common.WriteVarBytes(w, v.Sign)
}

func (v *DPosProposalVote) DeserializeUnsigned(r io.Reader) error {
	if err := v.Proposal.Deserialize(r); err != nil {
		return err
	}

	signer, err := common.ReadVarString(r)
	if err != nil {
		return err
	}
	v.Signer = signer

	acceptValue, err := common.ReadUint8(r)
	if err != nil {
		return err
	}
	if acceptValue == 1 {
		v.Accept = true
	} else {
		v.Accept = false
	}

	return nil
}

func (v *DPosProposalVote) Deserialize(r io.Reader) error {
	if err := v.DeserializeUnsigned(r); err != nil {
		return err
	}

	sign, err := common.ReadVarBytes(r, DefaultDPosSignDataLength, "sign data")
	if err != nil {
		return err
	}

	v.Sign = sign
	return nil
}
