// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA/core/types/payload"
)

const DefaultProposalMessageDataSize = 168 //67+32+4+65

type Proposal struct {
	Proposal payload.DPOSProposal
}

func (m *Proposal) CMD() string {
	return CmdReceivedProposal
}

func (m *Proposal) MaxLength() uint32 {
	return DefaultProposalMessageDataSize
}

func (m *Proposal) Serialize(w io.Writer) error {
	return m.Proposal.Serialize(w)
}

func (m *Proposal) Deserialize(r io.Reader) error {
	return m.Proposal.Deserialize(r)
}
