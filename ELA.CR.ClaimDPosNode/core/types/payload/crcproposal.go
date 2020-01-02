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
	Normal CRCProposalType = 0x0000

	// ELIP indicates elastos improvement type of proposal.
	ELIP CRCProposalType = 0x0100
	// Used to identify process-related elips
	FLOWELIP CRCProposalType = 0x0101
	// Used to flag Elastos design issues
	INFOELIP CRCProposalType = 0x0102

	// MainChainUpgradeCode indicates the code upgrade types of proposals.
	MainChainUpgradeCode CRCProposalType = 0x0200

	// SideChainUpgradeCode indicates the side chain related types of proposals.
	SideChainUpgradeCode CRCProposalType = 0x0300
	// Registration of side chain
	RegisterSideChain CRCProposalType = 0x0301

	// SecretaryGeneral indicates the vote secretary general types of proposals.
	SecretaryGeneral CRCProposalType = 0x0400
	// ChangeSponsor indicates the change proposal sponsor types of proposals.
	ChangeSponsor CRCProposalType = 0x0401
	// CloseProposal indicates the close proposal types of proposals.
	CloseProposal CRCProposalType = 0x0402

	// Common information used to define consensus governance
	DappConsensus CRCProposalType = 0x0500
)

type CRCProposalType uint16

func (pt CRCProposalType) Name() string {
	switch pt {
	case Normal:
		return "Normal"
	case ELIP:
		return "ELIP"
	case MainChainUpgradeCode:
		return "MainChainUpgradeCode"
	case SideChainUpgradeCode:
		return "SideChainUpgradeCode"
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

type Budget struct {
	// 0x00 regular payment
	// 0x01 final payment
	BudgetType byte
	Stage      byte
	Amount     common.Fixed64
}

type CRCProposal struct {
	// The type of current proposal.
	ProposalType CRCProposalType

	// Used to store category data
	// with a length limit not exceeding 4096 characters
	CategoryData string

	// Public key of sponsor.
	SponsorPublicKey []byte
	// The hash of draft proposal.
	DraftHash common.Uint256
	// The budget of different stages.
	Budgets []Budget
	// The address of budget.
	Recipient common.Uint168

	// The signature of sponsor.
	Sign []byte

	// DID of CR sponsor.
	CRSponsorDID common.Uint168

	// The hash of proposal opinion.
	CROpinionHash common.Uint256

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

	if err := common.WriteElement(w, p.ProposalType); err != nil {
		return errors.New("failed to serialize ProposalType")
	}

	if err := common.WriteVarString(w, p.CategoryData); err != nil {
		return errors.New("[CRCProposal], Category Data serialize failed")
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

func (b *Budget) Serialize(w io.Writer) error {
	if err := common.WriteElement(w, b.BudgetType); err != nil {
		return errors.New("failed to serialize BudgetType")
	}
	if err := common.WriteElement(w, b.Stage); err != nil {
		return errors.New("failed to serialize BudgetType")
	}
	return b.Amount.Serialize(w)
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

	if err := p.CROpinionHash.Serialize(w); err != nil {
		return errors.New("failed to serialize CROpinionHash")
	}

	return common.WriteVarBytes(w, p.CRSign)
}

func (b *Budget) Deserialize(r io.Reader) error {
	if err := common.ReadElement(r, &b.BudgetType); err != nil {
		return errors.New("[CRCProposal], BudgetType deserialize failed")
	}
	if err := common.ReadElement(r, &b.Stage); err != nil {
		return errors.New("[CRCProposal], Stage deserialize failed")
	}
	return b.Amount.Deserialize(r)

}

func (p *CRCProposal) DeserializeUnSigned(r io.Reader, version byte) error {
	err := common.ReadElement(r, &p.ProposalType)
	if err != nil {
		return errors.New("[CRCProposal], ProposalType deserialize failed")
	}

	p.CategoryData, err = common.ReadVarString(r)
	if err != nil {
		return errors.New("[CRCProposal], Category data deserialize failed")
	}

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
	p.Budgets = make([]Budget, 0)
	for i := 0; i < int(count); i++ {
		var budget Budget
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

	if err = p.CROpinionHash.Deserialize(r); err != nil {
		return errors.New("failed to deserialize CROpinionHash")
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
