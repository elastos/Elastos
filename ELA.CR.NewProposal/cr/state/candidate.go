// Copyright (c) 2017-2020 The Elastos Foundation
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
	"Returned", "Impeached"}

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
	depositHash    common.Uint168
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

	if err = common.WriteUint32(w, c.cancelHeight); err != nil {
		return
	}

	return c.depositHash.Serialize(w)
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

	return c.depositHash.Deserialize(r)
}

// Info returns a copy of the origin registered CR info.
func (c *Candidate) Info() payload.CRInfo {
	return c.info
}

// State returns the CR's state, can be pending, active, canceled or returned.
func (c *Candidate) State() CandidateState {
	return c.state
}

// Votes returns the votes of the CR.
func (c *Candidate) Votes() common.Fixed64 {
	return c.votes
}

// RegisterHeight returns the height when the CR was registered.
func (c *Candidate) RegisterHeight() uint32 {
	return c.registerHeight
}

// RegisterHeight returns the height when the CR was unregistered.
func (c *Candidate) CancelHeight() uint32 {
	return c.cancelHeight
}
