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
	// Common indicates that the transaction is used to record common
	// information.
	Common CRCProposalTrackingType = 0x00

	// Progress indicates that the transaction is used to indicate the progress
	// of the CRC proposal.
	Progress CRCProposalTrackingType = 0x01

	// ProgressReject indicates that the transaction is used to indicate current
	// progress of CRC proposal verification failed.
	ProgressReject CRCProposalTrackingType = 0x02

	// Terminated indicates that the transaction is used to indicate that the
	// CRC proposal has been terminated.
	Terminated CRCProposalTrackingType = 0x03

	// ProposalLeader indicates that the transaction is used to change the person in
	// charge of the CRC proposal.
	ProposalLeader CRCProposalTrackingType = 0x04

	// Appropriation indicates that the transaction is used for appropriation.
	Appropriation CRCProposalTrackingType = 0x05
)

type CRCProposalTrackingType byte

func (pt CRCProposalTrackingType) Name() string {
	switch pt {
	case Common:
		return "Common"
	case Progress:
		return "Progress"
	case ProgressReject:
		return "ProgressReject"
	case Terminated:
		return "Terminated"
	case ProposalLeader:
		return "ProposalLeader"
	case Appropriation:
		return "Appropriation"
	default:
		return "Unknown"
	}
}

const CRCProposalTrackingVersion byte = 0x00

type CRCProposalTracking struct {
	// The type of current proposal tracking.
	ProposalTrackingType CRCProposalTrackingType

	// The hash of current tracking proposal.
	ProposalHash common.Uint256

	// The hash of proposal tracking document.
	DocumentHash common.Uint256

	// The stage of proposal.
	Stage uint8

	// The leader public key.
	LeaderPubKey []byte

	// The new leader public key.
	NewLeaderPubKey []byte

	// The signature of proposal leader.
	LeaderSign []byte

	// The signature of new proposal leader.
	NewLeaderSign []byte

	// The hash of proposal tracking opinion.
	SecretaryOpinionHash common.Uint256

	// The signature of secretary general.
	SecretaryGeneralSign []byte
}

func (p *CRCProposalTracking) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := p.SerializeUnsigned(buf, version); err != nil {
		return []byte{0}
	}

	return buf.Bytes()
}

func (p *CRCProposalTracking) SerializeUnsigned(w io.Writer, version byte) error {
	if _, err := w.Write([]byte{byte(p.ProposalTrackingType)}); err != nil {
		return errors.New("failed to serialize ProposalTrackingType")
	}

	if err := p.ProposalHash.Serialize(w); err != nil {
		return errors.New("failed to serialize ProposalHash")
	}

	if err := p.DocumentHash.Serialize(w); err != nil {
		return errors.New("failed to serialize DocumentHash")
	}

	if err := common.WriteUint8(w, p.Stage); err != nil {
		return errors.New("failed to serialize Stage")
	}

	if err := common.WriteVarBytes(w, p.LeaderPubKey); err != nil {
		return errors.New("failed to serialize LeaderPubKey")
	}

	if err := common.WriteVarBytes(w, p.NewLeaderPubKey); err != nil {
		return errors.New("failed to serialize NewLeaderPubKey")
	}

	return nil
}

func (p *CRCProposalTracking) Serialize(w io.Writer, version byte) error {
	if err := p.SerializeUnsigned(w, version); err != nil {
		return err
	}

	if err := common.WriteVarBytes(w, p.LeaderSign); err != nil {
		return errors.New("failed to serialize LeaderSign")
	}

	if err := common.WriteVarBytes(w, p.NewLeaderSign); err != nil {
		return errors.New("failed to serialize NewLeaderSign")
	}

	if err := p.SecretaryOpinionHash.Serialize(w); err != nil {
		return errors.New("failed to serialize SecretaryOpinionHash")
	}

	if err := common.WriteVarBytes(w, p.SecretaryGeneralSign); err != nil {
		return errors.New("failed to serialize SecretaryGeneralSign")
	}

	return nil
}

func (p *CRCProposalTracking) DeserializeUnSigned(r io.Reader, version byte) error {
	pType, err := common.ReadBytes(r, 1)
	if err != nil {
		return errors.New("failed to deserialize ProposalTrackingType")
	}
	p.ProposalTrackingType = CRCProposalTrackingType(pType[0])

	if err = p.ProposalHash.Deserialize(r); err != nil {
		return errors.New("failed to deserialize ProposalHash")
	}

	if err = p.DocumentHash.Deserialize(r); err != nil {
		return errors.New("failed to deserialize DocumentHash")
	}

	if p.Stage, err = common.ReadUint8(r); err != nil {
		return errors.New("failed to deserialize Stage")
	}

	if p.LeaderPubKey, err = common.ReadVarBytes(r, crypto.PublicKeyScriptLength,
		"leader pubkey"); err != nil {
		return errors.New("failed to deserialize LeaderPubKey")
	}

	if p.NewLeaderPubKey, err = common.ReadVarBytes(r, crypto.PublicKeyScriptLength,
		"new leader pubkey"); err != nil {
		return errors.New("failed to deserialize NewLeaderPubKey")
	}

	return nil
}

func (p *CRCProposalTracking) Deserialize(r io.Reader, version byte) error {
	if err := p.DeserializeUnSigned(r, version); err != nil {
		return err
	}

	leaderSign, err := common.ReadVarBytes(r, crypto.SignatureLength,
		"leader signature")
	if err != nil {
		return errors.New("failed to deserialize leaderSign")
	}
	p.LeaderSign = leaderSign

	newLeaderSign, err := common.ReadVarBytes(r, crypto.SignatureLength,
		"new leader signature")
	if err != nil {
		return errors.New("failed to deserialize newLeaderSign")
	}
	p.NewLeaderSign = newLeaderSign

	if err = p.SecretaryOpinionHash.Deserialize(r); err != nil {
		return errors.New("failed to deserialize SecretaryOpinionHash")
	}

	sgSign, err := common.ReadVarBytes(r, crypto.SignatureLength, "secretary general signature")
	if err != nil {
		return errors.New("failed to deserialize SecretaryGeneralSign")
	}
	p.SecretaryGeneralSign = sgSign

	return nil
}
