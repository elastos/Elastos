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

const CRCProposalReviewVersion byte = 0x00

const (
	Approve VoteResult = 0x00
	Reject  VoteResult = 0x01
	Abstain VoteResult = 0x02
)

type VoteResult byte

func (v VoteResult) Name() string {
	switch v {
	case Approve:
		return "approve"
	case Reject:
		return "reject"
	case Abstain:
		return "abstain"
	default:
		return "unknown"
	}
}

type CRCProposalReview struct {
	ProposalHash common.Uint256
	VoteResult   VoteResult
	OpinionHash  common.Uint256
	DID          common.Uint168
	Signature    []byte
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

	err = common.WriteVarBytes(w, a.Signature)
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
		return errors.New("[CRCProposalReview], failed to serialize VoteResult")
	}
	if err := a.OpinionHash.Serialize(w); err != nil {
		return errors.New("[CRCProposalReview], failed to serialize OpinionHash")
	}
	if err := a.DID.Serialize(w); err != nil {
		return errors.New("[CRCProposalReview], failed to serialize DID")
	}

	return nil
}

func (a *CRCProposalReview) Deserialize(r io.Reader, version byte) error {
	err := a.DeserializeUnsigned(r, version)
	if err != nil {
		return err
	}
	a.Signature, err = common.ReadVarBytes(r, crypto.MaxSignatureScriptLength, "sign")
	if err != nil {
		return errors.New("[CRCProposalReview], Signature deserialize failed")
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
		return errors.New("[CRCProposalReview], failed to deserialize VoteResult")
	}
	a.VoteResult = VoteResult(val[0])

	if err = a.OpinionHash.Deserialize(r); err != nil {
		return errors.New("[CRCProposalReview], failed to deserialize OpinionHash")
	}
	if err := a.DID.Deserialize(r); err != nil {
		return errors.New("[CRCProposalReview], failed to deserialize DID")
	}
	return nil
}
