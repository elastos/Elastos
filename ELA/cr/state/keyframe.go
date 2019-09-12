// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package state

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/utils"
)

// CRMember defines CR committee member related info.
type CRMember struct {
	Info             payload.CRInfo
	ImpeachmentVotes common.Fixed64
	DepositAmount    common.Fixed64
	DepositHash      common.Uint168
	Penalty          common.Fixed64
}

// StateKeyFrame holds necessary state about CR committee.
type KeyFrame struct {
	Members             []*CRMember
	LastCommitteeHeight uint32
}

// StateKeyFrame holds necessary state about CR state.
type StateKeyFrame struct {
	CodeDIDMap         map[string]common.Uint168
	PendingCandidates  map[common.Uint168]*Candidate
	ActivityCandidates map[common.Uint168]*Candidate
	CanceledCandidates map[common.Uint168]*Candidate
	Nicknames          map[string]struct{}
	Votes              map[string]*types.Output
	DepositOutputs     map[string]*types.Output
}

// ProposalState defines necessary state about an CR proposals.
type ProposalState struct {
	Status   ProposalStatus
	Proposal payload.CRCProposal
	TxHash   common.Uint256

	CRVotes            map[common.Uint168]VoteResult
	VotersRejectAmount common.Fixed64
	RegisterHeight     uint32
	VoteStartHeight    uint32
}

// ProposalKeyFrame holds all runtime state about CR proposals.
type ProposalKeyFrame struct {
	Proposals map[common.Uint256]*ProposalState
}

func (c *CRMember) Serialize(w io.Writer) (err error) {
	if err = c.Info.SerializeUnsigned(w, payload.CRInfoVersion); err != nil {
		return
	}

	if err = common.WriteUint64(w, uint64(c.ImpeachmentVotes)); err != nil {
		return
	}

	if err = common.WriteUint64(w, uint64(c.DepositAmount)); err != nil {
		return
	}

	if err = common.WriteUint64(w, uint64(c.Penalty)); err != nil {
		return
	}

	return c.DepositHash.Serialize(w)
}

func (c *CRMember) Deserialize(r io.Reader) (err error) {
	if err = c.Info.DeserializeUnsigned(r, payload.CRInfoVersion); err != nil {
		return
	}

	var votes uint64
	if votes, err = common.ReadUint64(r); err != nil {
		return
	}
	c.ImpeachmentVotes = common.Fixed64(votes)

	var depositAmount uint64
	if depositAmount, err = common.ReadUint64(r); err != nil {
		return
	}
	c.DepositAmount = common.Fixed64(depositAmount)

	var penalty uint64
	if penalty, err = common.ReadUint64(r); err != nil {
		return
	}
	c.Penalty = common.Fixed64(penalty)

	return c.DepositHash.Deserialize(r)
}

func (k *KeyFrame) Serialize(w io.Writer) (err error) {
	if err = common.WriteVarUint(w, uint64(len(k.Members))); err != nil {
		return
	}

	for _, v := range k.Members {
		if err = v.Serialize(w); err != nil {
			return
		}
	}

	return common.WriteUint32(w, k.LastCommitteeHeight)
}

func (k *KeyFrame) Deserialize(r io.Reader) (err error) {
	var memLen uint64
	if memLen, err = common.ReadVarUint(r, 0); err != nil {
		return
	}

	k.Members = make([]*CRMember, 0, memLen)
	for i := uint64(0); i < memLen; i++ {
		member := &CRMember{}
		if err = member.Deserialize(r); err != nil {
			return
		}
		k.Members = append(k.Members, member)
	}

	k.LastCommitteeHeight, err = common.ReadUint32(r)
	return
}

func (k *KeyFrame) Snapshot() *KeyFrame {
	frame := NewKeyFrame()
	frame.LastCommitteeHeight = k.LastCommitteeHeight
	frame.Members = copyCRMembers(k.Members)
	return frame
}

func NewKeyFrame() *KeyFrame {
	return &KeyFrame{
		Members:             make([]*CRMember, 0),
		LastCommitteeHeight: 0,
	}
}

func (k *StateKeyFrame) Serialize(w io.Writer) (err error) {
	if err = k.serializeCodeAddressMap(w, k.CodeDIDMap); err != nil {
		return
	}

	if err = k.serializeCandidateMap(w, k.PendingCandidates); err != nil {
		return
	}

	if err = k.serializeCandidateMap(w, k.ActivityCandidates); err != nil {
		return
	}

	if err = k.serializeCandidateMap(w, k.CanceledCandidates); err != nil {
		return
	}

	if err = utils.SerializeStringSet(w, k.Nicknames); err != nil {
		return
	}

	if err = k.serializeOutputsMap(w, k.Votes); err != nil {
		return
	}

	return k.serializeOutputsMap(w, k.DepositOutputs)
}

func (k *StateKeyFrame) Deserialize(r io.Reader) (err error) {
	if k.CodeDIDMap, err = k.deserializeCodeAddressMap(r); err != nil {
		return
	}

	if k.PendingCandidates, err = k.deserializeCandidateMap(r); err != nil {
		return
	}

	if k.ActivityCandidates, err = k.deserializeCandidateMap(r); err != nil {
		return
	}

	if k.CanceledCandidates, err = k.deserializeCandidateMap(r); err != nil {
		return
	}

	if k.Nicknames, err = utils.DeserializeStringSet(r); err != nil {
		return
	}

	if k.Votes, err = k.deserializeOutputsMap(r); err != nil {
		return
	}

	if k.DepositOutputs, err = k.deserializeOutputsMap(r); err != nil {
		return
	}
	return
}

func (k *StateKeyFrame) serializeCodeAddressMap(w io.Writer,
	cmap map[string]common.Uint168) (err error) {
	if err = common.WriteVarUint(w, uint64(len(cmap))); err != nil {
		return
	}
	for k, v := range cmap {
		if err = common.WriteVarString(w, k); err != nil {
			return
		}

		if err = v.Serialize(w); err != nil {
			return
		}
	}
	return
}

func (k *StateKeyFrame) deserializeCodeAddressMap(r io.Reader) (
	cmap map[string]common.Uint168, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	cmap = make(map[string]common.Uint168)

	for i := uint64(0); i < count; i++ {
		var k string
		if k, err = common.ReadVarString(r); err != nil {
			return
		}
		var v common.Uint168
		if err = v.Deserialize(r); err != nil {
			return
		}
		cmap[k] = v
	}
	return
}

func (k *StateKeyFrame) serializeCandidateMap(w io.Writer,
	cmap map[common.Uint168]*Candidate) (err error) {
	if err = common.WriteVarUint(w, uint64(len(cmap))); err != nil {
		return
	}
	for k, v := range cmap {
		if err = k.Serialize(w); err != nil {
			return
		}

		if err = v.Serialize(w); err != nil {
			return
		}
	}
	return
}

func (k *StateKeyFrame) deserializeCandidateMap(
	r io.Reader) (cmap map[common.Uint168]*Candidate, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	cmap = make(map[common.Uint168]*Candidate)
	for i := uint64(0); i < count; i++ {
		var k common.Uint168
		if err = k.Deserialize(r); err != nil {
			return
		}
		candidate := &Candidate{}
		if err = candidate.Deserialize(r); err != nil {
			return
		}
		cmap[k] = candidate
	}
	return
}

func (s *StateKeyFrame) serializeOutputsMap(w io.Writer,
	vmap map[string]*types.Output) (err error) {
	if err = common.WriteVarUint(w, uint64(len(vmap))); err != nil {
		return
	}
	for k, v := range vmap {
		if err = common.WriteVarString(w, k); err != nil {
			return
		}

		if v == nil {
			if err = common.WriteUint8(w, 0); err != nil {
				return
			}
		} else {
			if err = common.WriteUint8(w, 1); err != nil {
				return
			}

			if err = v.Serialize(w, types.TxVersion09); err != nil {
				return
			}
		}
	}
	return
}

func (s *StateKeyFrame) deserializeOutputsMap(
	r io.Reader) (vmap map[string]*types.Output, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	vmap = make(map[string]*types.Output)
	for i := uint64(0); i < count; i++ {
		var k string
		if k, err = common.ReadVarString(r); err != nil {
			return
		}

		var exist uint8
		if exist, err = common.ReadUint8(r); err != nil {
			return
		}

		if exist == 1 {
			vote := &types.Output{}
			if err = vote.Deserialize(r, types.TxVersion09); err != nil {
				return
			}
			vmap[k] = vote
		} else {
			vmap[k] = nil
		}
	}
	return
}

// Snapshot will create a new StateKeyFrame object and deep copy all related data.
func (k *StateKeyFrame) Snapshot() *StateKeyFrame {
	state := NewStateKeyFrame()
	state.CodeDIDMap = copyCodeAddressMap(k.CodeDIDMap)
	state.PendingCandidates = copyCandidateMap(k.PendingCandidates)
	state.ActivityCandidates = copyCandidateMap(k.ActivityCandidates)
	state.CanceledCandidates = copyCandidateMap(k.CanceledCandidates)
	state.Nicknames = utils.CopyStringSet(k.Nicknames)
	state.Votes = copyOutputsMap(k.Votes)
	state.DepositOutputs = copyOutputsMap(k.DepositOutputs)

	return state
}

func (p *ProposalState) Serialize(w io.Writer) (err error) {
	if err = p.Proposal.Serialize(w, payload.CRCProposalVersion); err != nil {
		return
	}

	if err = common.WriteUint8(w, uint8(p.Status)); err != nil {
		return
	}

	if err = common.WriteUint32(w, p.RegisterHeight); err != nil {
		return
	}

	if err = common.WriteUint32(w, p.VoteStartHeight); err != nil {
		return
	}

	if err = common.WriteUint64(w, uint64(p.VotersRejectAmount)); err != nil {
		return
	}

	if err = common.WriteVarUint(w, uint64(len(p.CRVotes))); err != nil {
		return
	}

	for k, v := range p.CRVotes {
		if err = k.Serialize(w); err != nil {
			return
		}

		if err = common.WriteUint8(w, uint8(v)); err != nil {
			return
		}
	}

	return p.TxHash.Serialize(w)
}

func (p *ProposalState) Deserialize(r io.Reader) (err error) {
	if err = p.Proposal.Deserialize(r, payload.CRCProposalVersion); err != nil {
		return
	}

	var status uint8
	if status, err = common.ReadUint8(r); err != nil {
		return
	}
	p.Status = ProposalStatus(status)

	if p.RegisterHeight, err = common.ReadUint32(r); err != nil {
		return
	}

	if p.VoteStartHeight, err = common.ReadUint32(r); err != nil {
		return
	}

	var amount uint64
	if amount, err = common.ReadUint64(r); err != nil {
		return
	}
	p.VotersRejectAmount = common.Fixed64(amount)

	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}

	p.CRVotes = make(map[common.Uint168]VoteResult, count)
	for i := uint64(0); i < count; i++ {
		var key common.Uint168
		if err = key.Deserialize(r); err != nil {
			return
		}

		var value uint8
		if value, err = common.ReadUint8(r); err != nil {
			return
		}
		p.CRVotes[key] = VoteResult(value)
	}

	return p.TxHash.Deserialize(r)
}

func (p *ProposalKeyFrame) Serialize(w io.Writer) (err error) {
	if err = common.WriteVarUint(w, uint64(len(p.Proposals))); err != nil {
		return
	}

	for k, v := range p.Proposals {
		if err = k.Serialize(w); err != nil {
			return
		}

		if err = v.Serialize(w); err != nil {
			return
		}
	}
	return
}

func (p *ProposalKeyFrame) Deserialize(r io.Reader) (err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}

	p.Proposals = make(map[common.Uint256]*ProposalState, count)
	for i := uint64(0); i < count; i++ {
		var k common.Uint256
		if err = k.Deserialize(r); err != nil {
			return
		}

		var v ProposalState
		if err = v.Deserialize(r); err != nil {
			return
		}
		p.Proposals[k] = &v
	}
	return
}

// Snapshot will create a new ProposalKeyFrame object and deep copy all related data.
func (p *ProposalKeyFrame) Snapshot() *ProposalKeyFrame {
	buf := new(bytes.Buffer)
	p.Serialize(buf)

	state := NewProposalKeyFrame()
	state.Deserialize(buf)
	return state
}

func NewProposalKeyFrame() *ProposalKeyFrame {
	return &ProposalKeyFrame{
		Proposals: make(map[common.Uint256]*ProposalState),
	}
}

func NewStateKeyFrame() *StateKeyFrame {
	return &StateKeyFrame{
		CodeDIDMap:         make(map[string]common.Uint168),
		PendingCandidates:  make(map[common.Uint168]*Candidate),
		ActivityCandidates: make(map[common.Uint168]*Candidate),
		CanceledCandidates: make(map[common.Uint168]*Candidate),
		Nicknames:          make(map[string]struct{}),
		Votes:              make(map[string]*types.Output),
		DepositOutputs:     make(map[string]*types.Output),
	}
}

// copyCandidateMap copy the CR map's key and value, and return the dst map.
func copyCandidateMap(src map[common.Uint168]*Candidate) (
	dst map[common.Uint168]*Candidate) {
	dst = map[common.Uint168]*Candidate{}
	for k, v := range src {
		p := *v
		dst[k] = &p
	}
	return
}

// copyCodeAddressMap copy the map's key and value, and return the dst map.
func copyCodeAddressMap(src map[string]common.Uint168) (
	dst map[string]common.Uint168) {
	dst = map[string]common.Uint168{}
	for k, v := range src {
		dst[k] = v
	}
	return
}

func copyOutputsMap(src map[string]*types.Output) (dst map[string]*types.Output) {
	dst = map[string]*types.Output{}
	for k, v := range src {
		if v == nil {
			dst[k] = nil
		} else {
			p := *v
			dst[k] = &p
		}
	}
	return
}

func copyCRMembers(src []*CRMember) []*CRMember {
	dst := make([]*CRMember, 0, len(src))
	for _, v := range src {
		m := *v
		dst = append(dst, &m)
	}
	return dst
}
