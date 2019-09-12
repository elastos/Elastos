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

type VoteResult byte

const (
	Approve VoteResult = 0x00
	Reject  VoteResult = 0x01
	Abstain VoteResult = 0x02
)
const CRCProposalReviewVersion byte = 0x00

type CRCProposalReview struct {
	ProposalHash common.Uint256
	VoteResult   VoteResult
	Code         []byte
	Sign         []byte
}

func (a *CRCProposalReview) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := a.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (a *CRCProposalReview) Serialize(w io.Writer, version byte) error {
	err := a.SerializeUnsigned(w, version)
	if err != nil {
		return err
	}

	err = common.WriteVarBytes(w, a.Sign)
	if err != nil {
		return errors.New("[CRCProposalReview], Signature serialize failed")
	}

	return nil
}

func (a *CRCProposalReview) SerializeUnsigned(w io.Writer, version byte) error {
	if err := a.ProposalHash.Serialize(w); err != nil {
		return err
	}
	if err := common.WriteUint8(w, byte(a.VoteResult)); err != nil {
		return errors.New("CRCProposalReview VoteResult serialization error.")
	}
	if err := common.WriteVarBytes(w, a.Code); err != nil {
		return errors.New("CRCProposalReview Code serialization error.")
	}

	return nil
}

func (a *CRCProposalReview) Deserialize(r io.Reader, version byte) error {
	err := a.DeserializeUnsigned(r, version)
	if err != nil {
		return err
	}
	a.Sign, err = common.ReadVarBytes(r, crypto.MaxSignatureScriptLength, "sign")
	if err != nil {
		return errors.New("[CRCProposalReview], Sign deserialize failed")
	}

	return nil
}

func (a *CRCProposalReview) DeserializeUnsigned(r io.Reader, version byte) error {
	var err error
	if err = a.ProposalHash.Deserialize(r); err != nil {
		return err
	}
	val, err := common.ReadBytes(r, 1)
	if err != nil {
		return errors.New("[CRCProposalReview] VoteResult deserialization error.")
	}
	a.VoteResult = VoteResult(val[0])

	a.Code, err = common.ReadVarBytes(r, crypto.MaxMultiSignCodeLength, "code")
	if err != nil {
		return errors.New("[CRCProposalReview], Code deserialize failed")
	}
	return nil
}
