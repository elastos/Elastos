package types

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/crypto"
)

type DPosProposal struct {
	Sponsor    string //todo [merge] replace with public key
	BlockHash  common.Uint256
	ViewOffset uint32
	Sign       []byte

	hash *common.Uint256
}

func (p *DPosProposal) Data() []byte {
	buf := new(bytes.Buffer)
	if err := p.SerializeUnsigned(buf); err != nil {
		return []byte{0}
	}

	return buf.Bytes()
}

func (p *DPosProposal) SerializeUnsigned(w io.Writer) error {
	if err := common.WriteVarString(w, p.Sponsor); err != nil {
		return err
	}
	if err := p.BlockHash.Serialize(w); err != nil {
		return err
	}
	return common.WriteUint32(w, p.ViewOffset)
}

func (p *DPosProposal) Serialize(w io.Writer) error {
	if err := p.SerializeUnsigned(w); err != nil {
		return err
	}
	return common.WriteVarBytes(w, p.Sign)
}

func (p *DPosProposal) DeserializeUnSigned(r io.Reader) error {
	sponsor, err := common.ReadVarString(r)
	if err != nil {
		return err
	}
	p.Sponsor = sponsor
	if err = p.BlockHash.Deserialize(r); err != nil {
		return err
	}
	p.ViewOffset, err = common.ReadUint32(r)
	return err
}

func (p *DPosProposal) Deserialize(r io.Reader) error {
	if err := p.DeserializeUnSigned(r); err != nil {
		return err
	}
	sign, err := common.ReadVarBytes(r, crypto.SignatureLength, "sign data")
	if err != nil {
		return err
	}
	p.Sign = sign
	return nil
}

func (p *DPosProposal) Hash() common.Uint256 {
	if p.hash == nil {
		buf := new(bytes.Buffer)
		p.SerializeUnsigned(buf)
		hash := common.Uint256(common.Sha256D(buf.Bytes()))
		p.hash = &hash
	}
	return *p.hash
}
