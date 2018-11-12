package common

import (
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/crypto"
)

const (
	DefaultDPosSignDataLength = crypto.SignatureLength
)

type DPosProposal struct {
	Sponsor   string //todo [merge] replace with public key
	BlockHash common.Uint256
	Sign      []byte
}

func (p *DPosProposal) IsValid() bool {
	//todo [merge] verify signature
	return true
}

func (p *DPosProposal) SignProposal() []byte {
	return []byte{1}
}

func (p *DPosProposal) SerializeUnsigned(w io.Writer) error {
	if err := common.WriteVarString(w, p.Sponsor); err != nil {
		return err
	}

	return p.BlockHash.Serialize(w)
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

	return p.BlockHash.Deserialize(r)
}

func (p *DPosProposal) Deserialize(r io.Reader) error {
	if err := p.DeserializeUnSigned(r); err != nil {
		return err
	}

	sign, err := common.ReadVarBytes(r, DefaultDPosSignDataLength, "sign data")
	if err != nil {
		return err
	}

	p.Sign = sign
	return nil
}
