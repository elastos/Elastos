// Copyright (c) 2017-2020 The Elastos Foundation
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
	// ChangeProposalOwner indicates the change proposal owner types of proposals.
	ChangeProposalOwner CRCProposalType = 0x0401
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
	//case MainChainUpgradeCode:
	//	return "MainChainUpgradeCode"
	//case SideChainUpgradeCode:
	//	return "SideChainUpgradeCode"
	case ChangeProposalOwner:
		return "ChangeProposalOwner"
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

const (
	Imprest       InstallmentType = 0x00
	NormalPayment InstallmentType = 0x01
	FinalPayment  InstallmentType = 0x02
)

type InstallmentType byte

func (b InstallmentType) Name() string {
	switch b {
	case Imprest:
		return "Imprest"
	case NormalPayment:
		return "NormalPayment"
	case FinalPayment:
		return "FinalPayment"
	default:
		return "Unknown"
	}
}

type Budget struct {
	Type   InstallmentType
	Stage  byte
	Amount common.Fixed64
}

type CRCProposal struct {
	// The type of current CR Council proposal.
	ProposalType CRCProposalType

	// Used to store category data
	// with a length limit not exceeding 4096 characters
	CategoryData string

	// Public key of proposal owner.
	OwnerPublicKey []byte

	NewOwnerPublicKey []byte

	PreviousHash common.Uint256

	// The hash of draft proposal.
	DraftHash common.Uint256

	// The detailed budget and expenditure plan.
	Budgets []Budget

	// The specified ELA address where the funds are to be sent.
	Recipient common.Uint168

	// To be closed proposal hash, this field will be used when the proposal type is CloseProposal
	CloseProposalHash common.Uint256

	// The signature of proposal's owner.
	Signature []byte

	// DID of CR Council Member.
	CRCouncilMemberDID common.Uint168

	// The signature of CR Council Member, check data include signature of
	// proposal owner.
	CRCouncilMemberSignature []byte

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
	switch p.ProposalType {
	case ChangeProposalOwner:
		return p.SerializeUnsignedChangeProposalOwner(w, version)
	case CloseProposal:
		return p.SerializeUnsignedCloseProposal(w, version)
	case SecretaryGeneral:
		return p.SerializeUnsignedChangeSecretaryGeneral(w, version)
	default:
		return p.SerializeUnsignedNormalOrELIP(w, version)
	}
}

func (p *CRCProposal) SerializeUnsignedNormalOrELIP(w io.Writer, version byte) error {

	if err := common.WriteElement(w, p.ProposalType); err != nil {
		return errors.New("failed to serialize ProposalType")
	}

	if err := common.WriteVarString(w, p.CategoryData); err != nil {
		return errors.New("[CRCProposal], Category Data serialize failed")
	}

	if err := common.WriteVarBytes(w, p.OwnerPublicKey); err != nil {
		return errors.New("failed to serialize OwnerPublicKey")
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

func (p *CRCProposal) SerializeUnsignedChangeProposalOwner(w io.Writer, version byte) error {
	if err := common.WriteElement(w, p.ProposalType); err != nil {
		return errors.New("failed to serialize ProposalType")
	}
	if err := common.WriteVarString(w, p.CategoryData); err != nil {
		return errors.New("category Data serialize failed")
	}
	if err := common.WriteVarBytes(w, p.OwnerPublicKey); err != nil {
		return errors.New("failed to serialize OwnerPublicKey")
	}
	if err := common.WriteVarBytes(w, p.NewOwnerPublicKey); err != nil {
		return errors.New("failed to serialize NewOwnerPublicKey")
	}
	if err := p.PreviousHash.Serialize(w); err != nil {
		return errors.New("failed to serialize PreviousHash")
	}
	if err := p.DraftHash.Serialize(w); err != nil {
		return errors.New("failed to serialize DraftHash")
	}
	return nil
}

func (p *CRCProposal) SerializeUnsignedChangeSecretaryGeneral(w io.Writer, version byte) error {
	// todo complete me later
	return nil
}

func (p *CRCProposal) SerializeUnsignedCloseProposal(w io.Writer, version byte) error {

	if err := common.WriteElement(w, p.ProposalType); err != nil {
		return errors.New("failed to serialize ProposalType")
	}

	if err := common.WriteVarString(w, p.CategoryData); err != nil {
		return errors.New("[CRCProposal], Category Data serialize failed")
	}

	if err := common.WriteVarBytes(w, p.OwnerPublicKey); err != nil {
		return errors.New("failed to serialize OwnerPublicKey")
	}

	if err := p.DraftHash.Serialize(w); err != nil {
		return errors.New("failed to serialize DraftHash")
	}

	if err := p.CloseProposalHash.Serialize(w); err != nil {
		return errors.New("failed to serialize CloseProposalHash")
	}

	return nil
}

func (p *CRCProposal) Serialize(w io.Writer, version byte) error {
	switch p.ProposalType {
	case ChangeProposalOwner:
		return p.SerializeChangeProposalOwner(w, version)
	case CloseProposal:
		return p.SerializeCloseProposal(w, version)
	case SecretaryGeneral:
		return p.SerializeChangeSecretaryGeneral(w, version)
	default:
		return p.SerializeNormalOrELIP(w, version)
	}
}

func (p *CRCProposal) SerializeNormalOrELIP(w io.Writer, version byte) error {
	if err := p.SerializeUnsigned(w, version); err != nil {
		return err
	}

	if err := common.WriteVarBytes(w, p.Signature); err != nil {
		return err
	}

	if err := p.CRCouncilMemberDID.Serialize(w); err != nil {
		return errors.New("failed to serialize CRCouncilMemberDID")
	}

	return common.WriteVarBytes(w, p.CRCouncilMemberSignature)
}

func (p *CRCProposal) SerializeChangeProposalOwner(w io.Writer, version byte) error {
	if err := p.SerializeUnsigned(w, version); err != nil {
		return err
	}
	if err := common.WriteVarBytes(w, p.Signature); err != nil {
		return err
	}
	if err := p.CRCouncilMemberDID.Serialize(w); err != nil {
		return errors.New("failed to serialize CRCouncilMemberDID")
	}
	return nil
}

func (p *CRCProposal) SerializeChangeSecretaryGeneral(w io.Writer, version byte) error {
	// todo complete me later
	return nil
}

func (p *CRCProposal) SerializeCloseProposal(w io.Writer, version byte) error {
	if err := p.SerializeUnsigned(w, version); err != nil {
		return err
	}

	if err := common.WriteVarBytes(w, p.Signature); err != nil {
		return err
	}

	if err := p.CRCouncilMemberDID.Serialize(w); err != nil {
		return errors.New("failed to serialize CRCouncilMemberDID")
	}

	return common.WriteVarBytes(w, p.CRCouncilMemberSignature)
}

func (b *Budget) Serialize(w io.Writer) error {
	if err := common.WriteElement(w, b.Type); err != nil {
		return errors.New("failed to serialize Type")
	}
	if err := common.WriteElement(w, b.Stage); err != nil {
		return errors.New("failed to serialize Stage")
	}
	return b.Amount.Serialize(w)
}

func (b *Budget) Deserialize(r io.Reader) error {
	if err := common.ReadElement(r, &b.Type); err != nil {
		return errors.New("[CRCProposal], Type deserialize failed")
	}
	if err := common.ReadElement(r, &b.Stage); err != nil {
		return errors.New("[CRCProposal], Stage deserialize failed")
	}
	return b.Amount.Deserialize(r)

}

func (p *CRCProposal) DeserializeUnSigned(r io.Reader, version byte) error {
	switch p.ProposalType {
	case ChangeProposalOwner:
		return p.DeserializeUnSignedChangeProposalOwner(r, version)
	case CloseProposal:
		return p.DeserializeUnSignedCloseProposal(r, version)
	case SecretaryGeneral:
		return p.DeserializeUnSignedChangeSecretaryGeneral(r, version)
	default:
		return p.DeserializeUnSignedNormalOrELIP(r, version)
	}
}

func (p *CRCProposal) DeserializeUnSignedNormalOrELIP(r io.Reader, version byte) error {
	var err error

	p.CategoryData, err = common.ReadVarString(r)
	if err != nil {
		return errors.New("[CRCProposal], Category data deserialize failed")
	}

	p.OwnerPublicKey, err = common.ReadVarBytes(r, crypto.NegativeBigLength, "owner")
	if err != nil {
		return errors.New("failed to deserialize OwnerPublicKey")
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

func (p *CRCProposal) DeserializeUnSignedChangeProposalOwner(r io.Reader, version byte) error {
	err := common.ReadElement(r, &p.ProposalType)
	if err != nil {
		return errors.New("[CRCProposal], ProposalType deserialize failed")
	}
	p.CategoryData, err = common.ReadVarString(r)
	if err != nil {
		return errors.New("[CRCProposal], Category data deserialize failed")
	}
	p.OwnerPublicKey, err = common.ReadVarBytes(r, crypto.NegativeBigLength, "owner")
	if err != nil {
		return errors.New("failed to deserialize OwnerPublicKey")
	}

	p.NewOwnerPublicKey, err = common.ReadVarBytes(r, crypto.NegativeBigLength, "owner")
	if err != nil {
		return errors.New("failed to deserialize NewOwnerPublicKey")
	}
	if err = p.PreviousHash.Deserialize(r); err != nil {
		return errors.New("failed to deserialize PreviousHash")
	}

	if err = p.DraftHash.Deserialize(r); err != nil {
		return errors.New("failed to deserialize DraftHash")
	}
	return nil
}
func (p *CRCProposal) DeserializeUnSignedCloseProposal(r io.Reader, version byte) error {
	var err error

	p.CategoryData, err = common.ReadVarString(r)
	if err != nil {
		return errors.New("[CRCProposal], Category data deserialize failed")
	}

	p.OwnerPublicKey, err = common.ReadVarBytes(r, crypto.NegativeBigLength, "owner")
	if err != nil {
		return errors.New("failed to deserialize OwnerPublicKey")
	}

	if err = p.DraftHash.Deserialize(r); err != nil {
		return errors.New("failed to deserialize DraftHash")
	}

	if err = p.CloseProposalHash.Deserialize(r); err != nil {
		return errors.New("failed to deserialize CloseProposalHash")
	}

	return nil
}
func (p *CRCProposal) DeserializeUnSignedChangeSecretaryGeneral(r io.Reader, version byte) error {
	// todo complete me later
	return nil
}

func (p *CRCProposal) Deserialize(r io.Reader, version byte) error {
	err := common.ReadElement(r, &p.ProposalType)
	if err != nil {
		return errors.New("[CRCProposal], ProposalType deserialize failed")
	}

	switch p.ProposalType {
	case ChangeProposalOwner:
		return p.DeserializeChangeProposalOwner(r, version)
	case CloseProposal:
		return p.DeserializeCloseProposal(r, version)
	case SecretaryGeneral:
		return p.DeserializeChangeSecretaryGeneral(r, version)
	default:
		return p.DeserializeNormalOrELIP(r, version)
	}
}

func (p *CRCProposal) DeserializeNormalOrELIP(r io.Reader, version byte) error {
	if err := p.DeserializeUnSigned(r, version); err != nil {
		return err
	}

	sign, err := common.ReadVarBytes(r, crypto.SignatureLength, "sign data")
	if err != nil {
		return err
	}
	p.Signature = sign

	if err := p.CRCouncilMemberDID.Deserialize(r); err != nil {
		return errors.New("failed to deserialize CRCouncilMemberDID")
	}

	crSign, err := common.ReadVarBytes(r, crypto.SignatureLength, "CR sign data")
	if err != nil {
		return err
	}
	p.CRCouncilMemberSignature = crSign

	return nil
}

func (p *CRCProposal) DeserializeChangeProposalOwner(r io.Reader, version byte) error {
	if err := p.DeserializeUnSigned(r, version); err != nil {
		return err
	}

	// owner signature
	sign, err := common.ReadVarBytes(r, crypto.SignatureLength, "sign data")
	if err != nil {
		return err
	}
	p.Signature = sign

	if err := p.CRCouncilMemberDID.Deserialize(r); err != nil {
		return errors.New("failed to deserialize CRCouncilMemberDID")
	}
	// cr signature
	crSign, err := common.ReadVarBytes(r, crypto.SignatureLength, "CR sign data")
	if err != nil {
		return err
	}
	p.CRCouncilMemberSignature = crSign
	return nil
}
func (p *CRCProposal) DeserializeCloseProposal(r io.Reader, version byte) error {

	if err := p.DeserializeUnSigned(r, version); err != nil {
		return err
	}

	sign, err := common.ReadVarBytes(r, crypto.SignatureLength, "sign data")
	if err != nil {
		return err
	}
	p.Signature = sign

	if err := p.CRCouncilMemberDID.Deserialize(r); err != nil {
		return errors.New("failed to deserialize CRCouncilMemberDID")
	}

	crSign, err := common.ReadVarBytes(r, crypto.SignatureLength, "CR sign data")
	if err != nil {
		return err
	}
	p.CRCouncilMemberSignature = crSign

	return nil
}
func (p *CRCProposal) DeserializeChangeSecretaryGeneral(r io.Reader, version byte) error {
	// todo complete me later
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
