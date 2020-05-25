// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

type RequestProposal struct {
	ProposalHash common.Uint256
}

func (msg *RequestProposal) CMD() string {
	return CmdRequestProposal
}

func (msg *RequestProposal) MaxLength() uint32 {
	return 32
}

func (msg *RequestProposal) Serialize(w io.Writer) error {
	if err := msg.ProposalHash.Serialize(w); err != nil {
		return err
	}

	return nil
}

func (msg *RequestProposal) Deserialize(r io.Reader) error {
	if err := msg.ProposalHash.Deserialize(r); err != nil {
		return err
	}

	return nil
}
