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

	// ELIP indicates elastos improvement type of proposal.
	ELIP CRCProposalType = 0x01

	// Code indicates the code upgrade types of proposals.
	Code CRCProposalType = 0x02

	// SideChain indicates the side chain related types of proposals.
	SideChain CRCProposalType = 0x03

	// ChangeSponsor indicates the change proposal sponsor types of proposals.
	ChangeSponsor CRCProposalType = 0x04

	// CloseProposal indicates the close proposal types of proposals.
	CloseProposal CRCProposalType = 0x05

	// SecretaryGeneral indicates the vote secretary general types of proposals.
	SecretaryGeneral CRCProposalType = 0x06
)

type CRCProposalType byte

func (pt CRCProposalType) Name() string {
	switch pt {
	case Normal:
		return "Normal"
	case ELIP:
		return "ELIP"
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

const (
	// CRCProposalVersion indicates the version of CRC proposal payload
	CRCProposalVersion byte = 0x00

	// MaxProposalDataSize the max size of proposal draft data or proposal
	// tracking document data.
	MaxProposalDataSize = 2 * 1024 * 1024
)

type CRCProposal struct {
	// The type of current proposal.
	ProposalType CRCProposalType
	// Public key of sponsor.
	SponsorPublicKey []byte
	// The hash of draft proposal.
	DraftHash common.Uint256
	// The budget of different stages.
	Budgets []common.Fixed64
	// The address of budget.
	Recipient common.Uint168

	// The signature of sponsor.
	Sign []byte

	// DID of CR sponsor.
	CRSponsorDID common.Uint168
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
		return errors.New("failed to serialize ProposalType")
	}

	if err := common.WriteVarBytes(w, p.SponsorPublicKey); err != nil {
		return errors.New("failed to serialize SponsorPublicKey")
	}

	if err := p.DraftHash.Serialize(w); err != nil {
		return errors.New("failed to serialize DraftHash")
	}

	if err := common.WriteVarUint(w, uint64(len(p.Budgets))); err != nil {
		return errors.New("failed to serialize Budgets")
	}

	for _, v := range p.Budgets {
		if err := v.Serialize(w); err != nil {
			return errors.New("failed to serialize Budgets")
		}
	}

	if err := p.Recipient.Serialize(w); err != nil {
		return errors.New("failed to serialize Recipient")
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

	if err := p.CRSponsorDID.Serialize(w); err != nil {
		return errors.New("failed to serialize CRSponsorDID")
	}

	return common.WriteVarBytes(w, p.CRSign)
}

func (p *CRCProposal) DeserializeUnSigned(r io.Reader, version byte) error {
	pType, err := common.ReadBytes(r, 1)
	if err != nil {
		return err
	}
	p.ProposalType = CRCProposalType(pType[0])

	p.SponsorPublicKey, err = common.ReadVarBytes(r, crypto.NegativeBigLength, "sponsor")
	if err != nil {
		return errors.New("failed to deserialize SponsorPublicKey")
	}

	if err = p.DraftHash.Deserialize(r); err != nil {
		return errors.New("failed to deserialize DraftHash")
	}
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return errors.New("failed to deserialize Budgets")
	}
	p.Budgets = make([]common.Fixed64, 0)
	for i := 0; i < int(count); i++ {
		var budget common.Fixed64
		if err := budget.Deserialize(r); err != nil {
			return errors.New("failed to deserialize Budgets")
		}
		p.Budgets = append(p.Budgets, budget)
	}

	if err = p.Recipient.Deserialize(r); err != nil {
		return errors.New("failed to deserialize Recipient")
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

	if err := p.CRSponsorDID.Deserialize(r); err != nil {
		return errors.New("failed to deserialize CRSponsorDID")
	}
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
		hash := common.Hash(buf.Bytes())
		p.hash = &hash
	}
	return *p.hash
}
