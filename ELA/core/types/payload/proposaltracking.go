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

	// Terminated indicates that the transaction is used to indicate that the
	// CRC proposal has been terminated.
	Terminated CRCProposalTrackingType = 0x02

	// ProposalLeader indicates that the transaction is used to change the person in
	// charge of the CRC proposal.
	ProposalLeader CRCProposalTrackingType = 0x03

	// Appropriation indicates that the transaction is used for appropriation.
	Appropriation CRCProposalTrackingType = 0x04
)

type CRCProposalTrackingType byte

func (pt CRCProposalTrackingType) Name() string {
	switch pt {
	case Common:
		return "Common"
	case Progress:
		return "Progress"
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

	// The appropriation of current stage of proposal.
	Appropriation common.Fixed64

	// The leader public key.
	LeaderPubKey []byte

	// The new leader public key.
	NewLeaderPubKey []byte

	// The signature of proposal leader.
	LeaderSign []byte

	// The signature of new proposal leader.
	NewLeaderSign []byte

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
		return errors.New("the ProposalTrackingType serialize failed")
	}

	if err := p.ProposalHash.Serialize(w); err != nil {
		return errors.New("the ProposalHash serialize failed")
	}

	if err := p.DocumentHash.Serialize(w); err != nil {
		return errors.New("the DocumentHash serialize failed")
	}

	if err := common.WriteUint8(w, p.Stage); err != nil {
		return errors.New("the Stage serialize failed")
	}

	if err := p.Appropriation.Serialize(w); err != nil {
		return errors.New("the Appropriation serialize failed")
	}

	if err := common.WriteVarBytes(w, p.LeaderPubKey); err != nil {
		return errors.New("the LeaderPubKey serialize failed")
	}

	if err := common.WriteVarBytes(w, p.NewLeaderPubKey); err != nil {
		return errors.New("the NewLeaderPubKey serialize failed")
	}

	return nil
}

func (p *CRCProposalTracking) Serialize(w io.Writer, version byte) error {
	if err := p.SerializeUnsigned(w, version); err != nil {
		return err
	}

	if err := common.WriteVarBytes(w, p.LeaderSign); err != nil {
		return errors.New("the LeaderSign serialize failed")
	}

	if err := common.WriteVarBytes(w, p.NewLeaderSign); err != nil {
		return errors.New("the NewLeaderSign serialize failed")
	}

	if err := common.WriteVarBytes(w, p.SecretaryGeneralSign); err != nil {
		return errors.New("the SecretaryGeneralSign serialize failed")
	}

	return nil
}

func (p *CRCProposalTracking) DeserializeUnSigned(r io.Reader, version byte) error {
	pType, err := common.ReadBytes(r, 1)
	if err != nil {
		return errors.New("the ProposalTrackingType deserialize failed")
	}
	p.ProposalTrackingType = CRCProposalTrackingType(pType[0])

	if err = p.ProposalHash.Deserialize(r); err != nil {
		return errors.New("the ProposalHash deserialize failed")
	}

	if err = p.DocumentHash.Deserialize(r); err != nil {
		return errors.New("the DocumentHash deserialize failed")
	}

	if p.Stage, err = common.ReadUint8(r); err != nil {
		return errors.New("the Stage deserialize failed")
	}

	if err := p.Appropriation.Deserialize(r); err != nil {
		return errors.New("the Appropriation deserialize failed")
	}

	if p.LeaderPubKey, err = common.ReadVarBytes(r, crypto.PublicKeyScriptLength,
		"leader pubkey"); err != nil {
		return errors.New("the LeaderPubKey deserialize failed")
	}

	if p.NewLeaderPubKey, err = common.ReadVarBytes(r, crypto.PublicKeyScriptLength,
		"new leader pubkey"); err != nil {
		return errors.New("the NewLeaderPubKey deserialize failed")
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
		return errors.New("the leaderSign deserialize failed")
	}
	p.LeaderSign = leaderSign

	newLeaderSign, err := common.ReadVarBytes(r, crypto.SignatureLength,
		"new leader signature")
	if err != nil {
		return errors.New("the newLeaderSign deserialize failed")
	}
	p.NewLeaderSign = newLeaderSign

	sgSign, err := common.ReadVarBytes(r, crypto.SignatureLength,
		"secretary general signature")
	if err != nil {
		return errors.New("the SecretaryGeneralSign deserialize failed")
	}
	p.SecretaryGeneralSign = sgSign

	return nil
}
