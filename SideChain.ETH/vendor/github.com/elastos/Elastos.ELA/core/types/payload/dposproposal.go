package payload

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
)

type DPOSProposal struct {
	Sponsor    []byte
	BlockHash  common.Uint256
	ViewOffset uint32
	Sign       []byte

	hash *common.Uint256
}

func (p *DPOSProposal) Data() []byte {
	buf := new(bytes.Buffer)
	if err := p.SerializeUnsigned(buf); err != nil {
		return []byte{0}
	}

	return buf.Bytes()
}

func (p *DPOSProposal) SerializeUnsigned(w io.Writer) error {
	if err := common.WriteVarBytes(w, p.Sponsor); err != nil {
		return err
	}
	if err := p.BlockHash.Serialize(w); err != nil {
		return err
	}
	return common.WriteUint32(w, p.ViewOffset)
}

func (p *DPOSProposal) Serialize(w io.Writer) error {
	if err := p.SerializeUnsigned(w); err != nil {
		return err
	}
	return common.WriteVarBytes(w, p.Sign)
}

func (p *DPOSProposal) DeserializeUnSigned(r io.Reader) error {
	sponsor, err := common.ReadVarBytes(r, crypto.NegativeBigLength, "public key")
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

func (p *DPOSProposal) Deserialize(r io.Reader) error {
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

func (p *DPOSProposal) Hash() common.Uint256 {
	if p.hash == nil {
		buf := new(bytes.Buffer)
		p.SerializeUnsigned(buf)
		hash := common.Uint256(common.Sha256D(buf.Bytes()))
		p.hash = &hash
	}
	return *p.hash
}
