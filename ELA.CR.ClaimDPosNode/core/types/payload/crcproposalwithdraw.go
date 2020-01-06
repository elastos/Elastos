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

const CRCProposalWithdrawVersion byte = 0x00

type CRCProposalWithdraw struct {
	// Hash of the proposal to withdrawal ela
	ProposalHash common.Uint256

	// Public key of sponsor.
	SponsorPublicKey []byte

	//fee amount
	Fee common.Fixed64

	//signature
	Sign []byte
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

	err = common.WriteVarBytes(w, p.Sign)
	if err != nil {
		return errors.New("[CRCProposalWithdraw], Signature serialize failed")
	}

	return nil
}

func (p *CRCProposalWithdraw) SerializeUnsigned(w io.Writer, version byte) error {
	if err := p.ProposalHash.Serialize(w); err != nil {
		return err
	}

	if err := common.WriteVarBytes(w, p.SponsorPublicKey); err != nil {
		return err
	}

	if err := common.WriteUint64(w, uint64(p.Fee)); err != nil {
		return errors.New("[CRCProposalWithdraw], Fee serialization error.")
	}

	return nil
}

func (p *CRCProposalWithdraw) Deserialize(r io.Reader, version byte) error {
	err := p.DeserializeUnsigned(r, version)
	if err != nil {
		return err
	}
	p.Sign, err = common.ReadVarBytes(r, crypto.MaxSignatureScriptLength, "sign")
	if err != nil {
		return errors.New("[CRCProposalWithdraw], Sign deserialize failed")
	}

	return nil
}

func (p *CRCProposalWithdraw) DeserializeUnsigned(r io.Reader,
	version byte) error {
	var err error
	if err = p.ProposalHash.Deserialize(r); err != nil {
		return err
	}
	SponsorPublicKey, err := common.ReadVarBytes(r, crypto.NegativeBigLength, "sponsor")
	if err != nil {
		return err
	}
	p.SponsorPublicKey = SponsorPublicKey

	var fee uint64
	if fee, err = common.ReadUint64(r); err != nil {
		return errors.New("[CRCProposalReview] Fee deserialization error.")
	}
	p.Fee = common.Fixed64(fee)
	return nil
}
