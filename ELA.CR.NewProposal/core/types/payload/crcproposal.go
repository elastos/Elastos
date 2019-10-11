// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package payload

import (
	"bytes"
	"errors"
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

	// CloseProposal indicates the close proposal types of proposals.
	CloseProposal CRCProposalType = 0x04

	// SecretaryGeneral indicates the vote secretary general types of proposals.
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
	case CloseProposal:
		return "CloseProposal"
	case SecretaryGeneral:
		return "SecretaryGeneral"
	default:
		return "Unknown"
	}
}

const CRCProposalVersion byte = 0x00

type CRCProposal struct {
	// The type of current proposal.
	ProposalType CRCProposalType
	// Public key of sponsor.
	SponsorPublicKey []byte
	// DID of CR sponsor.
	CRSponsorDID common.Uint168
	// The hash of draft proposal.
	DraftHash common.Uint256
	// The budget of different stages.
	Budgets []common.Fixed64
	// The signature of sponsor.
	Sign []byte
	// The signature of CR sponsor, check data include signature of sponsor.
	CRSign []byte

	hash *common.Uint256
}

func (p *CRCProposal) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := p.SerializeUnsigned(buf, version); err != nil {
		return []byte{0}
	}

	return buf.Bytes()
}

func (p *CRCProposal) SerializeUnsigned(w io.Writer, version byte) error {
	if _, err := w.Write([]byte{byte(p.ProposalType)}); err != nil {
		return err
	}
	if err := common.WriteVarBytes(w, p.SponsorPublicKey); err != nil {
		return err
	}

	if err := p.CRSponsorDID.Serialize(w); err != nil {
		return errors.New("[UnregisterCR], DID serialize failed")
	}
	if err := p.DraftHash.Serialize(w); err != nil {
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
	p.ProposalType = CRCProposalType(pType[0])
	sponsor, err := common.ReadVarBytes(r, crypto.MaxSignatureScriptLength, "sponsor")
	if err != nil {
		return err
	}
	p.SponsorPublicKey = sponsor
	if err := p.CRSponsorDID.Deserialize(r); err != nil {
		return errors.New("[UnregisterCR], DID deserialize failed")
	}

	if err := p.DraftHash.Deserialize(r); err != nil {
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

func (p *CRCProposal) Hash() common.Uint256 {
	if p.hash == nil {
		buf := new(bytes.Buffer)
		p.Serialize(buf, CRCProposalVersion)
		hash := common.Uint256(common.Sha256D(buf.Bytes()))
		p.hash = &hash
	}
	return *p.hash
}
