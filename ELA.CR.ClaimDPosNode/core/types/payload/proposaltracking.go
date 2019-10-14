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

	// SideChain indicates that the transaction is used to change the person in
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

	// The stage of proposal.
	Stage uint32

	// The appropriation of current stage of proposal.
	Appropriation common.Fixed64

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
		return err
	}

	if err := p.ProposalHash.Serialize(w); err != nil {
		return errors.New("the ProposalHash serialize failed")
	}

	if err := common.WriteUint32(w, p.Stage); err != nil {
		return errors.New("the Stage serialize failed")
	}

	if err := p.Appropriation.Serialize(w); err != nil {
		return errors.New("the Appropriation serialize failed")
	}

	return nil
}

func (p *CRCProposalTracking) serializeProgressType(w io.Writer, version byte) error {
	if _, err := w.Write([]byte{byte(p.ProposalTrackingType)}); err != nil {
		return err
	}

	return nil
}

func (p *CRCProposalTracking) Serialize(w io.Writer, version byte) error {
	if err := p.SerializeUnsigned(w, version); err != nil {
		return err
	}

	if err := common.WriteVarBytes(w, p.LeaderSign); err != nil {
		return err
	}

	if err := common.WriteVarBytes(w, p.NewLeaderSign); err != nil {
		return err
	}

	return common.WriteVarBytes(w, p.SecretaryGeneralSign)
}

func (p *CRCProposalTracking) DeserializeUnSigned(r io.Reader, version byte) error {
	pType, err := common.ReadBytes(r, 1)
	if err != nil {
		return err
	}
	p.ProposalTrackingType = CRCProposalTrackingType(pType[0])

	if err := p.ProposalHash.Deserialize(r); err != nil {
		return errors.New("the ProposalHash deserialize failed")
	}
	if p.Stage, err = common.ReadUint32(r); err != nil {
		return errors.New("the Stage deserialize failed")
	}
	if err := p.Appropriation.Deserialize(r); err != nil {
		return errors.New("the Appropriation deserialize failed")
	}
	return nil
}

func (p *CRCProposalTracking) Deserialize(r io.Reader, version byte) error {
	if err := p.DeserializeUnSigned(r, version); err != nil {
		return err
	}
	crSign, err := common.ReadVarBytes(r, crypto.SignatureLength,
		"leader signature")
	if err != nil {
		return err
	}
	p.LeaderSign = crSign
	newCRSign, err := common.ReadVarBytes(r, crypto.SignatureLength,
		"new leader signature")
	if err != nil {
		return err
	}
	p.NewLeaderSign = newCRSign
	sgSign, err := common.ReadVarBytes(r, crypto.SignatureLength,
		"secretary general signature")
	if err != nil {
		return err
	}
	p.SecretaryGeneralSign = sgSign
	return nil
}
