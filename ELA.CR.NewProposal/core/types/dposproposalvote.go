package types

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
)

type DPosProposalVote struct {
	ProposalHash common.Uint256

	Signer string
	Accept bool
	Sign   []byte

	hash *common.Uint256
}

func (v *DPosProposalVote) Data() []byte {
	buf := new(bytes.Buffer)
	if err := v.SerializeUnsigned(buf); err != nil {
		return []byte{0}
	}

	return buf.Bytes()
}

func (v *DPosProposalVote) SerializeUnsigned(w io.Writer) error {
	if err := v.ProposalHash.Serialize(w); err != nil {
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
	if err := v.ProposalHash.Deserialize(r); err != nil {
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

	sign, err := common.ReadVarBytes(r, crypto.SignatureLength, "sign data")
	if err != nil {
		return err
	}

	v.Sign = sign
	return nil
}

func (v *DPosProposalVote) Hash() common.Uint256 {
	if v.hash == nil {
		buf := new(bytes.Buffer)
		v.SerializeUnsigned(buf)
		hash := common.Uint256(common.Sha256D(buf.Bytes()))
		v.hash = &hash
	}
	return *v.hash
}
