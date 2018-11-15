package core

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA/config"

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
	var isArbiter bool
	for _, a := range config.Parameters.Arbiters {
		if a == p.Sponsor {
			isArbiter = true
		}
	}
	if !isArbiter {
		return false
	}

	publicKey, err := common.HexStringToBytes(p.Sponsor)
	if err != nil {
		return false
	}
	pubKey, err := crypto.DecodePoint(publicKey[1:])
	if err != nil {
		return false
	}
	err = crypto.Verify(*pubKey, p.Data(), p.Sign)
	if err != nil {
		return false
	}

	return true
}

func (p *DPosProposal) Data() []byte {
	buf := new(bytes.Buffer)
	if err := p.SerializeUnsigned(buf); err != nil {
		return []byte{0}
	}

	return buf.Bytes()
}

func (p *DPosProposal) SignProposal() ([]byte, error) {
	privateKey, err := common.HexStringToBytes(config.Parameters.ArbiterConfiguration.PrivateKey)
	if err != nil {
		return []byte{0}, err
	}

	signature, err := crypto.Sign(privateKey, p.Data())
	if err != nil {
		return []byte{0}, err
	}

	return signature, nil
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
	if err == nil {
		return err
	}
	p.Sign = sign
	return nil
}
