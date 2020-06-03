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
	// Common indicates that the transaction is used to record common
	// information.
	Common CRCProposalTrackingType = 0x00

	// Progress indicates that the transaction is used to indicate the progress
	// of the CRC proposal.
	Progress CRCProposalTrackingType = 0x01

	// Rejected indicates that the transaction is used to indicate current
	// progress of CRC proposal verification failed.
	Rejected CRCProposalTrackingType = 0x02

	// Terminated indicates that the transaction is used to indicate that the
	// CRC proposal has been terminated.
	Terminated CRCProposalTrackingType = 0x03

	// ChangeOwner indicates that the transaction is used to change the person in
	// charge of the CRC proposal.
	ChangeOwner CRCProposalTrackingType = 0x04

	// Finalized for summary proposal execution, as well as the application of final payment
	Finalized CRCProposalTrackingType = 0x05
)

type CRCProposalTrackingType byte

func (pt CRCProposalTrackingType) Name() string {
	switch pt {
	case Common:
		return "Common"
	case Progress:
		return "Progress"
	case Rejected:
		return "Rejected"
	case Terminated:
		return "Terminated"
	case ChangeOwner:
		return "ChangeOwner"
	case Finalized:
		return "Finalized"
	default:
		return "Unknown"
	}
}

const CRCProposalTrackingVersion byte = 0x00

type CRCProposalTracking struct {
	// The hash of current tracking proposal.
	ProposalHash common.Uint256

	// The hash of proposal tracking message.
	MessageHash common.Uint256

	// The stage of proposal.
	Stage uint8

	// The proposal owner public key.
	OwnerPublicKey []byte

	// The new proposal owner public key.
	NewOwnerPublicKey []byte

	// The signature of proposal owner.
	OwnerSignature []byte

	// The signature of new proposal owner.
	NewOwnerSignature []byte

	// The type of current proposal tracking.
	ProposalTrackingType CRCProposalTrackingType

	// The hash of secretary general's opinion.
	SecretaryGeneralOpinionHash common.Uint256

	// The signature of secretary general.
	SecretaryGeneralSignature []byte
}

func (p *CRCProposalTracking) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := p.SerializeUnsigned(buf, version); err != nil {
		return []byte{0}
	}

	return buf.Bytes()
}

func (p *CRCProposalTracking) SerializeUnsigned(w io.Writer, version byte) error {
	if err := p.ProposalHash.Serialize(w); err != nil {
		return errors.New("failed to serialize ProposalHash")
	}

	if err := p.MessageHash.Serialize(w); err != nil {
		return errors.New("failed to serialize MessageHash")
	}

	if err := common.WriteUint8(w, p.Stage); err != nil {
		return errors.New("failed to serialize Stage")
	}

	if err := common.WriteVarBytes(w, p.OwnerPublicKey); err != nil {
		return errors.New("failed to serialize OwnerPublicKey")
	}

	if err := common.WriteVarBytes(w, p.NewOwnerPublicKey); err != nil {
		return errors.New("failed to serialize NewOwnerPublicKey")
	}

	return nil
}

func (p *CRCProposalTracking) Serialize(w io.Writer, version byte) error {
	if err := p.SerializeUnsigned(w, version); err != nil {
		return err
	}

	if err := common.WriteVarBytes(w, p.OwnerSignature); err != nil {
		return errors.New("failed to serialize OwnerSignature")
	}

	if err := common.WriteVarBytes(w, p.NewOwnerSignature); err != nil {
		return errors.New("failed to serialize NewOwnerSignature")
	}

	if _, err := w.Write([]byte{byte(p.ProposalTrackingType)}); err != nil {
		return errors.New("failed to serialize ProposalTrackingType")
	}

	if err := p.SecretaryGeneralOpinionHash.Serialize(w); err != nil {
		return errors.New("failed to serialize SecretaryGeneralOpinionHash")
	}

	if err := common.WriteVarBytes(w, p.SecretaryGeneralSignature); err != nil {
		return errors.New("failed to serialize SecretaryGeneralSignature")
	}

	return nil
}

func (p *CRCProposalTracking) DeserializeUnSigned(r io.Reader, version byte) error {
	var err error
	if err = p.ProposalHash.Deserialize(r); err != nil {
		return errors.New("failed to deserialize ProposalHash")
	}

	if err = p.MessageHash.Deserialize(r); err != nil {
		return errors.New("failed to deserialize MessageHash")
	}

	if p.Stage, err = common.ReadUint8(r); err != nil {
		return errors.New("failed to deserialize Stage")
	}

	if p.OwnerPublicKey, err = common.ReadVarBytes(r, crypto.PublicKeyScriptLength,
		"owner pubkey"); err != nil {
		return errors.New("failed to deserialize OwnerPublicKey")
	}

	if p.NewOwnerPublicKey, err = common.ReadVarBytes(r, crypto.PublicKeyScriptLength,
		"new owner pubkey"); err != nil {
		return errors.New("failed to deserialize NewOwnerPublicKey")
	}

	return nil
}

func (p *CRCProposalTracking) Deserialize(r io.Reader, version byte) error {
	if err := p.DeserializeUnSigned(r, version); err != nil {
		return err
	}

	ownerSign, err := common.ReadVarBytes(r, crypto.SignatureLength,
		"owner signature")
	if err != nil {
		return errors.New("failed to deserialize ownerSign")
	}
	p.OwnerSignature = ownerSign

	NewOwnerSignature, err := common.ReadVarBytes(r, crypto.SignatureLength,
		"new owner signature")
	if err != nil {
		return errors.New("failed to deserialize NewOwnerSignature")
	}
	p.NewOwnerSignature = NewOwnerSignature

	pType, err := common.ReadBytes(r, 1)
	if err != nil {
		return errors.New("failed to deserialize ProposalTrackingType")
	}
	p.ProposalTrackingType = CRCProposalTrackingType(pType[0])

	if err = p.SecretaryGeneralOpinionHash.Deserialize(r); err != nil {
		return errors.New("failed to deserialize SecretaryGeneralOpinionHash")
	}

	sgSign, err := common.ReadVarBytes(r, crypto.SignatureLength, "secretary general signature")
	if err != nil {
		return errors.New("failed to deserialize SecretaryGeneralSignature")
	}
	p.SecretaryGeneralSignature = sgSign

	return nil
}
