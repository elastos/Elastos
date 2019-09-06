// Copyright (c) 2017-2019 Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package payload

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
)

const (
	// Normal indicates the normal types of proposal.
	Normal CRCProposalType = 0x00

	// Code indicates the code upgrade types of proposals.
	Code CRCProposalType = 0x01

	// SideChain indicates the side chain related types of proposals.
	SideChain CRCProposalType = 0x02

	// ChangeSponsor indicates the change proposal sponsor types of proposals.
	ChangeSponsor CRCProposalType = 0x03

	// Close indicates the close proposal types of proposals.
	Close CRCProposalType = 0x04

	// SecretaryGeneral indicates the
	SecretaryGeneral CRCProposalType = 0x05
)

type CRCProposalType byte

func (pt CRCProposalType) Name() string {
	switch pt {
	case Normal:
		return "Normal"
	case Code:
		return "Code"
	case SideChain:
		return "SideChain"
	case ChangeSponsor:
		return "ChangeSponsor"
	case Close:
		return "Close"
	case SecretaryGeneral:
		return "SecretaryGeneral"
	default:
		return "Unknown"
	}
}

const CRCProposalVersion byte = 0x00

type CRCProposal struct {
	proposalType CRCProposalType
	// Public key of sponsor.
	Sponsor []byte
	// Code of CR sponsor.
	CRSponsor  []byte
	OriginHash common.Uint256
	Budgets    []common.Fixed64

	Sign   []byte
	CRSign []byte
	hash   *common.Uint256
}

func (p *CRCProposal) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := p.SerializeUnsigned(buf, version); err != nil {
		return []byte{0}
	}

	return buf.Bytes()
}

func (p *CRCProposal) SerializeUnsigned(w io.Writer, version byte) error {
	if _, err := w.Write([]byte{byte(p.proposalType)}); err != nil {
		return err
	}
	if err := common.WriteVarBytes(w, p.Sponsor); err != nil {
		return err
	}
	if err := common.WriteVarBytes(w, p.CRSponsor); err != nil {
		return err
	}
	if err := p.OriginHash.Serialize(w); err != nil {
		return err
	}
	if err := common.WriteVarUint(w, uint64(len(p.Budgets))); err != nil {
		return err
	}
	for _, v := range p.Budgets {
		if err := v.Serialize(w); err != nil {
			return err
		}
	}
	return nil
}

func (p *CRCProposal) Serialize(w io.Writer, version byte) error {
	if err := p.SerializeUnsigned(w, version); err != nil {
		return err
	}

	if err := common.WriteVarBytes(w, p.Sign); err != nil {
		return err
	}
	return common.WriteVarBytes(w, p.CRSign)
}

func (p *CRCProposal) DeserializeUnSigned(r io.Reader, version byte) error {
	pType, err := common.ReadBytes(r, 1)
	if err != nil {
		return err
	}
	p.proposalType = CRCProposalType(pType[0])
	sponsor, err := common.ReadVarBytes(r, crypto.MaxSignatureScriptLength, "sponsor")
	if err != nil {
		return err
	}
	p.Sponsor = sponsor
	crSponsor, err := common.ReadVarBytes(r, crypto.MaxSignatureScriptLength, "CR sponsor")
	if err != nil {
		return err
	}
	p.CRSponsor = crSponsor
	if err := p.OriginHash.Deserialize(r); err != nil {
		return err
	}
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return err
	}
	p.Budgets = make([]common.Fixed64, 0)
	for i := 0; i < int(count); i++ {
		var budget common.Fixed64
		if err := budget.Deserialize(r); err != nil {
			return err
		}
		p.Budgets = append(p.Budgets, budget)
	}
	return nil
}

func (p *CRCProposal) Deserialize(r io.Reader, version byte) error {
	if err := p.DeserializeUnSigned(r, version); err != nil {
		return err
	}
	sign, err := common.ReadVarBytes(r, crypto.SignatureLength, "sign data")
	if err != nil {
		return err
	}
	p.Sign = sign
	crSign, err := common.ReadVarBytes(r, crypto.SignatureLength, "CR sign data")
	if err != nil {
		return err
	}
	p.CRSign = crSign
	return nil
}
