// Copyright (c) 2017-2019 Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package state

import (
	"fmt"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types/payload"
)

// CandidateState defines states during a CR candidates lifetime
type CandidateState byte

const (
	// Pending indicates the producer is just registered and didn't get 6
	// confirmations yet.
	Pending CandidateState = iota

	// Active indicates the CR is registered and confirmed by more than
	// 6 blocks.
	Active

	// Canceled indicates the CR was canceled.
	Canceled

	// Returned indicates the CR has canceled and deposit returned.
	Returned
)

// candidateStateStrings is a array of CR states back to their constant
// names for pretty printing.
var candidateStateStrings = []string{"Pending", "Active", "Canceled",
	"Returned"}

func (ps CandidateState) String() string {
	if int(ps) < len(candidateStateStrings) {
		return candidateStateStrings[ps]
	}
	return fmt.Sprintf("CandidateState-%d", ps)
}

// Candidate defines information about CR candidates during the CR vote period
type Candidate struct {
	info           payload.CRInfo
	state          CandidateState
	votes          common.Fixed64
	registerHeight uint32
	cancelHeight   uint32
}

func (c *Candidate) Serialize(w io.Writer) (err error) {
	if err = c.info.SerializeUnsigned(w, payload.CRInfoVersion); err != nil {
		return
	}

	if err = common.WriteUint8(w, uint8(c.state)); err != nil {
		return
	}

	if err = common.WriteUint64(w, uint64(c.votes)); err != nil {
		return
	}

	if err = common.WriteUint32(w, c.registerHeight); err != nil {
		return
	}

	return common.WriteUint32(w, c.cancelHeight)
}

func (c *Candidate) Deserialize(r io.Reader) (err error) {
	if err = c.info.DeserializeUnsigned(r, payload.CRInfoVersion); err != nil {
		return
	}

	var state uint8
	if state, err = common.ReadUint8(r); err != nil {
		return
	}
	c.state = CandidateState(state)

	var votes uint64
	if votes, err = common.ReadUint64(r); err != nil {
		return
	}
	c.votes = common.Fixed64(votes)

	if c.registerHeight, err = common.ReadUint32(r); err != nil {
		return
	}

	c.cancelHeight, err = common.ReadUint32(r)
	return
}

func (c *Candidate) Info() payload.CRInfo {
	return c.info
}

func (c *Candidate) State() CandidateState {
	return c.state
}

func (c *Candidate) Votes() common.Fixed64 {
	return c.votes
}
