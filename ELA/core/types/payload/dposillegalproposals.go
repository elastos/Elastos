// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package payload

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/elanet/pact"
)

const (
	IllegalProposalVersion byte = 0x00
)

type ProposalEvidence struct {
	Proposal    DPOSProposal
	BlockHeader []byte
	BlockHeight uint32
}

type DPOSIllegalProposals struct {
	Evidence        ProposalEvidence
	CompareEvidence ProposalEvidence

	hash *common.Uint256
}

func (d *ProposalEvidence) Serialize(w io.Writer) error {
	if err := d.Proposal.Serialize(w); err != nil {
		return err
	}

	if err := common.WriteVarBytes(w, d.BlockHeader); err != nil {
		return err
	}

	if err := common.WriteUint32(w, d.BlockHeight); err != nil {
		return err
	}

	return nil
}

func (d *ProposalEvidence) Deserialize(r io.Reader) (err error) {
	if err = d.Proposal.Deserialize(r); err != nil {
		return err
	}

	if d.BlockHeader, err = common.ReadVarBytes(r, pact.MaxBlockContextSize,
		"block header"); err != nil {
		return err
	}

	if d.BlockHeight, err = common.ReadUint32(r); err != nil {
		return err
	}

	return nil
}

func (d *DPOSIllegalProposals) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := d.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (d *DPOSIllegalProposals) Serialize(w io.Writer, version byte) error {
	if err := d.Evidence.Serialize(w); err != nil {
		return err
	}

	if err := d.CompareEvidence.Serialize(w); err != nil {
		return err
	}

	return nil
}

func (d *DPOSIllegalProposals) Deserialize(r io.Reader, version byte) error {
	if err := d.Evidence.Deserialize(r); err != nil {
		return err
	}

	if err := d.CompareEvidence.Deserialize(r); err != nil {
		return err
	}

	return nil
}

func (d *DPOSIllegalProposals) Hash() common.Uint256 {
	if d.hash == nil {
		buf := new(bytes.Buffer)
		d.Serialize(buf, IllegalProposalVersion)
		hash := common.Hash(buf.Bytes())
		d.hash = &hash
	}
	return *d.hash
}

func (d *DPOSIllegalProposals) GetBlockHeight() uint32 {
	return d.Evidence.BlockHeight
}

func (d *DPOSIllegalProposals) Type() IllegalDataType {
	return IllegalProposal
}
