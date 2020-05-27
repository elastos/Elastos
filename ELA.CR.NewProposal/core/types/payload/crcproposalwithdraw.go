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
	CRCProposalWithdrawDefault   byte = 0x00
	CRCProposalWithdrawVersion01 byte = 0x01
)

type CRCProposalWithdraw struct {

	// Hash of the proposal to withdrawal ela.
	ProposalHash common.Uint256

	// Public key of proposal owner.
	OwnerPublicKey []byte

	// The specified ELA address where the funds will be sent.
	Recipient common.Uint168

	// The amount of sela to send
	Amount common.Fixed64

	// Signature of proposal owner.
	Signature []byte
}

func (p *CRCProposalWithdraw) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := p.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (p *CRCProposalWithdraw) Serialize(w io.Writer, version byte) error {
	err := p.SerializeUnsigned(w, version)
	if err != nil {
		return err
	}

	err = common.WriteVarBytes(w, p.Signature)
	if err != nil {
		return errors.New("[CRCProposalWithdraw], Signature serialize failed")
	}

	return nil
}

func (p *CRCProposalWithdraw) SerializeUnsigned(w io.Writer, version byte) error {
	if err := p.ProposalHash.Serialize(w); err != nil {
		return err
	}

	if err := common.WriteVarBytes(w, p.OwnerPublicKey); err != nil {
		return err
	}

	if version == CRCProposalWithdrawVersion01 {
		if err := p.Recipient.Serialize(w); err != nil {
			return err
		}

		if err := p.Amount.Serialize(w); err != nil {
			return err
		}
	}

	return nil
}

func (p *CRCProposalWithdraw) Deserialize(r io.Reader, version byte) error {
	err := p.DeserializeUnsigned(r, version)
	if err != nil {
		return err
	}
	p.Signature, err = common.ReadVarBytes(r, crypto.MaxSignatureScriptLength, "sign")
	if err != nil {
		return errors.New("[CRCProposalWithdraw], Signature deserialize failed")
	}

	return nil
}

func (p *CRCProposalWithdraw) DeserializeUnsigned(r io.Reader,
	version byte) error {
	var err error
	if err = p.ProposalHash.Deserialize(r); err != nil {
		return err
	}
	ownerPublicKey, err := common.ReadVarBytes(r, crypto.NegativeBigLength, "owner")
	if err != nil {
		return err
	}
	p.OwnerPublicKey = ownerPublicKey

	if version == CRCProposalWithdrawVersion01 {
		if err := p.Recipient.Deserialize(r); err != nil {
			return err
		}
		if err := p.Amount.Deserialize(r); err != nil {
			return err
		}
	}
	return nil
}
