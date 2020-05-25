// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package payload

import (
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

type Confirm struct {
	Proposal DPOSProposal
	Votes    []DPOSProposalVote
}

func (p *Confirm) TryAppend(v DPOSProposalVote) bool {
	if p.Proposal.Hash().IsEqual(v.ProposalHash) {
		p.Votes = append(p.Votes, v)
		return true
	}
	return false
}

func (p *Confirm) Serialize(w io.Writer) error {
	if err := p.Proposal.Serialize(w); err != nil {
		return err
	}

	if err := common.WriteUint64(w, uint64(len(p.Votes))); err != nil {
		return err
	}

	for _, sign := range p.Votes {
		if err := sign.Serialize(w); err != nil {
			return err
		}
	}

	return nil
}

func (p *Confirm) Deserialize(r io.Reader) error {
	if err := p.Proposal.Deserialize(r); err != nil {
		return err
	}

	signCount, err := common.ReadUint64(r)
	if err != nil {
		return err
	}
	p.Votes = make([]DPOSProposalVote, signCount)

	for i := uint64(0); i < signCount; i++ {
		err := p.Votes[i].Deserialize(r)
		if err != nil {
			return err
		}
	}

	return nil
}
