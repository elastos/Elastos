// Copyright (c) 2017-2020 The Elastos Foundation
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
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/utils"
)

// MemberState defines states during a CR member lifetime
type MemberState byte

const (
	// MemberElected indicates the CR member is Elected.
	MemberElected MemberState = iota

	// MemberImpeached indicates the CR member was impeached.
	MemberImpeached

	// MemberTerminated indicates the CR member was terminated because elected
	// CR members are not enough.
	MemberTerminated

	// MemberReturned indicates the CR member has deposit returned.
	MemberReturned

	// MemberInactive indicates the CR member was inactive because the dpos node
	// is inactive.
	MemberInactive
)

func (s *MemberState) String() string {
	switch *s {
	case MemberElected:
		return "Elected"
	case MemberImpeached:
		return "Impeached"
	case MemberReturned:
		return "Returned"
	case MemberTerminated:
		return "Terminated"
	}

	return "Unknown"
}

type BudgetStatus uint8

const (
	// Unfinished indicates the proposal owner haven't started or uploaded the
	// status of the budget to CR Council.
	Unfinished BudgetStatus = iota

	// Withdrawable indicates the proposal owner have finished the milestone,
	// but haven't get the payment.
	Withdrawable

	// Withdrawn indicates the proposal owner have finished the milestone and
	// got the payment.
	Withdrawn

	// Rejected indicates the proposal owner have uploaded the status to CR
	// Council but the secretary-general rejected my request.
	Rejected

	// Closed indicates the proposal has been terminated and the milestone will
	// not be finished forever.
	Closed
)

func (s *BudgetStatus) Name() string {
	switch *s {
	case Unfinished:
		return "Unfinished"
	case Withdrawable:
		return "Withdrawable"
	case Withdrawn:
		return "Withdrawn"
	case Rejected:
		return "Rejected"
	case Closed:
		return "Closed"
	}
	return "Unknown"
}

// CRMember defines CR committee member related info.
type CRMember struct {
	Info             payload.CRInfo
	ImpeachmentVotes common.Fixed64
	DepositHash      common.Uint168
	MemberState      MemberState
	DPOSPublicKey    []byte
	InactiveCount    uint32
}

// StateKeyFrame holds necessary state about CR committee.
type KeyFrame struct {
	Members                    map[common.Uint168]*CRMember
	HistoryMembers             map[uint64]map[common.Uint168]*CRMember
	LastCommitteeHeight        uint32
	LastVotingStartHeight      uint32
	InElectionPeriod           bool
	NeedAppropriation          bool
	CRCFoundationLockedAmounts []common.Fixed64
	CRCFoundationBalance       common.Fixed64
	CRCCommitteeBalance        common.Fixed64
	CRCCommitteeUsedAmount     common.Fixed64
	CRCCurrentStageAmount      common.Fixed64
	DestroyedAmount            common.Fixed64
	CirculationAmount          common.Fixed64
	AppropriationAmount        common.Fixed64
	CRAssetsAddressUTXOCount   uint32
}

type DepositInfo struct {
	Refundable    bool
	DepositAmount common.Fixed64
	Penalty       common.Fixed64
	TotalAmount   common.Fixed64
}

// StateKeyFrame holds necessary state about CR state.
type StateKeyFrame struct {
	CodeCIDMap           map[string]common.Uint168
	DepositHashCIDMap    map[common.Uint168]common.Uint168
	Candidates           map[common.Uint168]*Candidate
	HistoryCandidates    map[uint64]map[common.Uint168]*Candidate
	depositInfo          map[common.Uint168]*DepositInfo
	CurrentSession       uint64
	Nicknames            map[string]struct{}
	Votes                map[string]struct{}
	DepositOutputs       map[string]common.Fixed64
	CRCFoundationOutputs map[string]common.Fixed64
	CRCCommitteeOutputs  map[string]common.Fixed64
}

// ProposalState defines necessary state about an CR proposals.
type ProposalState struct {
	Status   ProposalStatus
	Proposal payload.CRCProposal
	TxHash   common.Uint256

	CRVotes            map[common.Uint168]payload.VoteResult
	VotersRejectAmount common.Fixed64
	RegisterHeight     uint32
	VoteStartHeight    uint32

	WithdrawnBudgets    map[uint8]common.Fixed64 // proposalWithdraw
	WithdrawableBudgets map[uint8]common.Fixed64 // proposalTracking
	BudgetsStatus       map[uint8]BudgetStatus
	FinalPaymentStatus  bool

	TrackingCount    uint8
	TerminatedHeight uint32
	ProposalOwner    []byte
	Recipient        common.Uint168
}

type ProposalHashSet map[common.Uint256]struct{}

func NewProposalHashSet() ProposalHashSet {
	return make(ProposalHashSet)
}

func (set *ProposalHashSet) Add(proposalHash common.Uint256) bool {
	_, found := (*set)[proposalHash]
	if found {
		return false //False if it existed already
	}
	(*set)[proposalHash] = struct{}{}
	return true
}

func (set *ProposalHashSet) Clear() {
	*set = NewProposalHashSet()
}

func (set *ProposalHashSet) Remove(proposalHash common.Uint256) {
	delete(*set, proposalHash)
}

func (set *ProposalHashSet) Contains(proposalHash common.Uint256) bool {
	if _, ok := (*set)[proposalHash]; !ok {
		return false
	}
	return true
}

func (set *ProposalHashSet) Len() int {
	return len(*set)
}

func (set *ProposalHashSet) Equal(other ProposalHashSet) bool {
	if set.Len() != other.Len() {
		return false
	}
	for elem := range *set {
		if !other.Contains(elem) {
			return false
		}
	}
	return true
}

type ProposalsMap map[common.Uint256]*ProposalState

// ProposalKeyFrame holds all runtime state about CR proposals.
type ProposalKeyFrame struct {
	Proposals ProposalsMap
	//key is did value is proposalhash set
	ProposalHashes  map[common.Uint168]ProposalHashSet
	ProposalSession map[uint64][]common.Uint256
	// proposalWithdraw info
	WithdrawableTxInfo map[common.Uint256]types.OutputInfo
	//publicKey of SecretaryGeneral
	SecretaryGeneralPublicKey string
}

func NewProposalMap() ProposalsMap {
	return make(ProposalsMap)
}

func (c *CRMember) Serialize(w io.Writer) (err error) {
	if err = c.Info.SerializeUnsigned(w, payload.CRInfoVersion); err != nil {
		return
	}

	if err = c.ImpeachmentVotes.Serialize(w); err != nil {
		return
	}

	return c.DepositHash.Serialize(w)
}

func (c *CRMember) Deserialize(r io.Reader) (err error) {
	if err = c.Info.DeserializeUnsigned(r, payload.CRInfoVersion); err != nil {
		return
	}

	if err = c.ImpeachmentVotes.Deserialize(r); err != nil {
		return
	}

	return c.DepositHash.Deserialize(r)
}

func (kf *KeyFrame) Serialize(w io.Writer) (err error) {
	if err = kf.serializeMembersMap(w, kf.Members); err != nil {
		return
	}

	if err = kf.serializeHistoryMembersMap(w, kf.HistoryMembers); err != nil {
		return
	}

	if err = kf.serializeAmountList(w, kf.CRCFoundationLockedAmounts); err != nil {
		return
	}

	return common.WriteElements(w, kf.LastCommitteeHeight,
		kf.LastVotingStartHeight, kf.InElectionPeriod, kf.NeedAppropriation,
		kf.CRCFoundationBalance, kf.CRCCommitteeBalance, kf.CRCCommitteeUsedAmount,
		kf.DestroyedAmount, kf.CirculationAmount, kf.AppropriationAmount)
}

func (kf *KeyFrame) Deserialize(r io.Reader) (err error) {
	if kf.Members, err = kf.deserializeMembersMap(r); err != nil {
		return
	}

	if kf.HistoryMembers, err = kf.deserializeHistoryMembersMap(r); err != nil {
		return
	}

	if kf.CRCFoundationLockedAmounts, err = kf.deserializeAmountList(r); err != nil {
		return
	}

	err = common.ReadElements(r, &kf.LastCommitteeHeight,
		&kf.LastVotingStartHeight, &kf.InElectionPeriod, &kf.NeedAppropriation,
		&kf.CRCFoundationBalance, &kf.CRCCommitteeBalance,
		&kf.CRCCommitteeUsedAmount, &kf.DestroyedAmount, &kf.CirculationAmount,
		&kf.AppropriationAmount)
	return
}

func (kf *KeyFrame) serializeMembersMap(w io.Writer,
	mmap map[common.Uint168]*CRMember) (err error) {
	if err = common.WriteVarUint(w, uint64(len(mmap))); err != nil {
		return
	}
	for k, v := range mmap {
		if err = k.Serialize(w); err != nil {
			return
		}

		if err = v.Serialize(w); err != nil {
			return
		}
	}
	return
}

func (kf *KeyFrame) serializeHistoryMembersMap(w io.Writer,
	hmap map[uint64]map[common.Uint168]*CRMember) (err error) {
	if err = common.WriteVarUint(w, uint64(len(hmap))); err != nil {
		return
	}
	for k, v := range hmap {
		if err = common.WriteVarUint(w, k); err != nil {
			return
		}

		if err = kf.serializeMembersMap(w, v); err != nil {
			return
		}
	}

	return
}

func (kf *KeyFrame) serializeAmountList(w io.Writer,
	amounts []common.Fixed64) (err error) {
	if err = common.WriteVarUint(w, uint64(len(amounts))); err != nil {
		return
	}
	for _, a := range amounts {
		if err = a.Serialize(w); err != nil {
			return err
		}
	}
	return
}

func (kf *KeyFrame) deserializeAmountList(
	r io.Reader) (amounts []common.Fixed64, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	amounts = make([]common.Fixed64, 0)
	for i := uint64(0); i < count; i++ {
		var amount common.Fixed64
		if err = amount.Deserialize(r); err != nil {
			return
		}
		amounts = append(amounts, amount)
	}
	return
}

func (kf *KeyFrame) deserializeMembersMap(
	r io.Reader) (mmap map[common.Uint168]*CRMember, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	mmap = make(map[common.Uint168]*CRMember)
	for i := uint64(0); i < count; i++ {
		var k common.Uint168
		if err = k.Deserialize(r); err != nil {
			return
		}
		candidate := &CRMember{}
		if err = candidate.Deserialize(r); err != nil {
			return
		}
		mmap[k] = candidate
	}
	return
}

func (kf *KeyFrame) deserializeHistoryMembersMap(
	r io.Reader) (hmap map[uint64]map[common.Uint168]*CRMember, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	hmap = make(map[uint64]map[common.Uint168]*CRMember)
	for i := uint64(0); i < count; i++ {
		var k uint64
		k, err = common.ReadVarUint(r, 0)
		if err != nil {
			return
		}
		var cmap map[common.Uint168]*CRMember
		cmap, err = kf.deserializeMembersMap(r)
		if err != nil {
			return
		}
		hmap[k] = cmap
	}
	return
}

func (kf *KeyFrame) Snapshot() *KeyFrame {
	frame := NewKeyFrame()
	frame.LastCommitteeHeight = kf.LastCommitteeHeight
	frame.LastVotingStartHeight = kf.LastVotingStartHeight
	frame.InElectionPeriod = kf.InElectionPeriod
	frame.NeedAppropriation = kf.NeedAppropriation
	frame.CRCFoundationBalance = kf.CRCFoundationBalance
	frame.CRCCommitteeBalance = kf.CRCCommitteeBalance
	frame.CRCCommitteeUsedAmount = kf.CRCCommitteeUsedAmount
	frame.DestroyedAmount = kf.DestroyedAmount
	frame.CirculationAmount = kf.CirculationAmount
	frame.AppropriationAmount = kf.AppropriationAmount
	frame.Members = copyMembersMap(kf.Members)
	frame.HistoryMembers = copyHistoryMembersMap(kf.HistoryMembers)
	frame.CRAssetsAddressUTXOCount = kf.CRAssetsAddressUTXOCount
	return frame
}

func NewKeyFrame() *KeyFrame {
	return &KeyFrame{
		Members:                    make(map[common.Uint168]*CRMember, 0),
		HistoryMembers:             make(map[uint64]map[common.Uint168]*CRMember, 0),
		CRCFoundationLockedAmounts: make([]common.Fixed64, 0),
		LastCommitteeHeight:        0,
	}
}

func (d *DepositInfo) Serialize(w io.Writer) (err error) {
	if err = common.WriteElement(w, d.Refundable); err != nil {
		return
	}

	if err = d.DepositAmount.Serialize(w); err != nil {
		return
	}

	if err = d.Penalty.Serialize(w); err != nil {
		return
	}

	if err = d.TotalAmount.Serialize(w); err != nil {
		return
	}

	return
}

func (d *DepositInfo) Deserialize(r io.Reader) (err error) {
	if err = common.ReadElement(r, &d.Refundable); err != nil {
		return
	}

	if err = d.DepositAmount.Deserialize(r); err != nil {
		return
	}

	if err = d.Penalty.Deserialize(r); err != nil {
		return
	}

	if err = d.TotalAmount.Deserialize(r); err != nil {
		return
	}

	return
}

func (kf *StateKeyFrame) Serialize(w io.Writer) (err error) {
	if err = kf.serializeCodeAddressMap(w, kf.CodeCIDMap); err != nil {
		return
	}

	if err = kf.serializeDepositCIDMap(w, kf.DepositHashCIDMap); err != nil {
		return
	}

	if err = kf.serializeCandidateMap(w, kf.Candidates); err != nil {
		return
	}

	if err = kf.serializeHistoryCandidateMap(w, kf.HistoryCandidates); err != nil {
		return
	}

	if err = kf.serializeDepositInfoMap(w, kf.depositInfo); err != nil {
		return
	}

	if err = common.WriteVarUint(w, kf.CurrentSession); err != nil {
		return
	}

	if err = utils.SerializeStringSet(w, kf.Nicknames); err != nil {
		return
	}

	if err = utils.SerializeStringSet(w, kf.Votes); err != nil {
		return
	}

	if err = kf.SerializeFixed64Map(w, kf.DepositOutputs); err != nil {
		return
	}

	if err = kf.SerializeFixed64Map(w, kf.CRCFoundationOutputs); err != nil {
		return
	}

	return kf.SerializeFixed64Map(w, kf.CRCCommitteeOutputs)
}

func (kf *StateKeyFrame) Deserialize(r io.Reader) (err error) {
	if kf.CodeCIDMap, err = kf.deserializeCodeAddressMap(r); err != nil {
		return
	}

	if kf.DepositHashCIDMap, err = kf.deserializeDepositCIDMap(r); err != nil {
		return
	}

	if kf.Candidates, err = kf.deserializeCandidateMap(r); err != nil {
		return
	}

	if kf.HistoryCandidates, err = kf.deserializeHistoryCandidateMap(r); err != nil {
		return
	}

	if kf.depositInfo, err = kf.deserializeDepositInfoMap(r); err != nil {
		return
	}

	if kf.CurrentSession, err = common.ReadVarUint(r, 0); err != nil {
		return
	}

	if kf.Nicknames, err = utils.DeserializeStringSet(r); err != nil {
		return
	}

	if kf.Votes, err = utils.DeserializeStringSet(r); err != nil {
		return
	}

	if kf.DepositOutputs, err = kf.DeserializeFixed64Map(r); err != nil {
		return
	}

	if kf.CRCFoundationOutputs, err = kf.DeserializeFixed64Map(r); err != nil {
		return
	}

	if kf.CRCCommitteeOutputs, err = kf.DeserializeFixed64Map(r); err != nil {
		return
	}
	return
}

func (kf *StateKeyFrame) serializeCodeAddressMap(w io.Writer,
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

func (kf *StateKeyFrame) deserializeCodeAddressMap(r io.Reader) (
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

func (kf *StateKeyFrame) serializeDepositCIDMap(w io.Writer,
	cmap map[common.Uint168]common.Uint168) (err error) {
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

func (kf *StateKeyFrame) deserializeDepositCIDMap(r io.Reader) (
	cmap map[common.Uint168]common.Uint168, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	cmap = make(map[common.Uint168]common.Uint168)

	for i := uint64(0); i < count; i++ {
		var k common.Uint168
		if err = k.Deserialize(r); err != nil {
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

func (kf *StateKeyFrame) serializeCandidateMap(w io.Writer,
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

func (kf *StateKeyFrame) serializeHistoryCandidateMap(w io.Writer,
	hmap map[uint64]map[common.Uint168]*Candidate) (err error) {
	if err = common.WriteVarUint(w, uint64(len(hmap))); err != nil {
		return
	}
	for k, v := range hmap {
		if err = common.WriteVarUint(w, k); err != nil {
			return
		}

		if err = kf.serializeCandidateMap(w, v); err != nil {
			return
		}
	}

	return
}

func (kf *StateKeyFrame) serializeDepositInfoMap(w io.Writer,
	vmap map[common.Uint168]*DepositInfo) (err error) {
	if err = common.WriteVarUint(w, uint64(len(vmap))); err != nil {
		return
	}
	for k, v := range vmap {
		if err = k.Serialize(w); err != nil {
			return
		}

		if err = v.Serialize(w); err != nil {
			return
		}
	}

	return
}

func (kf *StateKeyFrame) deserializeCandidateMap(
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

func (kf *StateKeyFrame) deserializeDepositInfoMap(
	r io.Reader) (vmap map[common.Uint168]*DepositInfo, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	vmap = make(map[common.Uint168]*DepositInfo)
	for i := uint64(0); i < count; i++ {
		var k common.Uint168
		if err = k.Deserialize(r); err != nil {
			return
		}
		v := &DepositInfo{}
		if err = v.Deserialize(r); err != nil {
			return
		}
		vmap[k] = v
	}
	return
}

func (kf *StateKeyFrame) deserializeHistoryCandidateMap(
	r io.Reader) (hmap map[uint64]map[common.Uint168]*Candidate, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	hmap = make(map[uint64]map[common.Uint168]*Candidate)
	for i := uint64(0); i < count; i++ {
		var k uint64
		k, err = common.ReadVarUint(r, 0)
		if err != nil {
			return
		}
		var cmap map[common.Uint168]*Candidate
		cmap, err = kf.deserializeCandidateMap(r)
		if err != nil {
			return
		}
		hmap[k] = cmap
	}
	return
}

func (kf *StateKeyFrame) SerializeFixed64Map(w io.Writer,
	vmap map[string]common.Fixed64) (err error) {
	if err = common.WriteVarUint(w, uint64(len(vmap))); err != nil {
		return
	}
	for k, v := range vmap {
		if err = common.WriteVarString(w, k); err != nil {
			return
		}
		if err = v.Serialize(w); err != nil {
			return
		}
	}
	return
}

func (kf *StateKeyFrame) DeserializeFixed64Map(
	r io.Reader) (vmap map[string]common.Fixed64, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	vmap = make(map[string]common.Fixed64)
	for i := uint64(0); i < count; i++ {
		var k string
		if k, err = common.ReadVarString(r); err != nil {
			return
		}
		var v common.Fixed64
		if err = v.Deserialize(r); err != nil {
			return
		}
		vmap[k] = v
	}
	return
}

// Snapshot will create a new StateKeyFrame object and deep copy all related data.
func (kf *StateKeyFrame) Snapshot() *StateKeyFrame {
	state := NewStateKeyFrame()
	state.CodeCIDMap = copyCodeAddressMap(kf.CodeCIDMap)
	state.DepositHashCIDMap = copyHashIDMap(kf.DepositHashCIDMap)
	state.Candidates = copyCandidateMap(kf.Candidates)
	state.HistoryCandidates = copyHistoryCandidateMap(kf.HistoryCandidates)
	state.depositInfo = copyDepositInfoMap(kf.depositInfo)
	state.CurrentSession = kf.CurrentSession
	state.Nicknames = utils.CopyStringSet(kf.Nicknames)
	state.Votes = utils.CopyStringSet(kf.Votes)
	state.DepositOutputs = copyFixed64Map(kf.DepositOutputs)
	state.CRCFoundationOutputs = copyFixed64Map(kf.DepositOutputs)
	state.CRCCommitteeOutputs = copyFixed64Map(kf.DepositOutputs)

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
	if err = p.serializeBudgets(p.WithdrawnBudgets, w); err != nil {
		return
	}
	if err = p.serializeBudgets(p.WithdrawableBudgets, w); err != nil {
		return
	}
	if err = p.serializeBudgetsStatus(p.BudgetsStatus, w); err != nil {
		return
	}
	if err := common.WriteElement(w, p.FinalPaymentStatus); err != nil {
		return err
	}
	if err = common.WriteUint8(w, p.TrackingCount); err != nil {
		return
	}
	if err = common.WriteUint32(w, p.TerminatedHeight); err != nil {
		return
	}
	if err := common.WriteVarBytes(w, p.ProposalOwner); err != nil {
		return err
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

	p.CRVotes = make(map[common.Uint168]payload.VoteResult, count)
	for i := uint64(0); i < count; i++ {
		var key common.Uint168
		if err = key.Deserialize(r); err != nil {
			return
		}

		var value uint8
		if value, err = common.ReadUint8(r); err != nil {
			return
		}
		p.CRVotes[key] = payload.VoteResult(value)
	}

	if p.WithdrawnBudgets, err = p.deserializeBudgets(r); err != nil {
		return
	}
	if p.WithdrawableBudgets, err = p.deserializeBudgets(r); err != nil {
		return
	}
	if p.BudgetsStatus, err = p.deserializeBudgetsStatus(r); err != nil {
		return
	}
	if err = common.ReadElement(r, &p.FinalPaymentStatus); err != nil {
		return err
	}
	if p.TrackingCount, err = common.ReadUint8(r); err != nil {
		return
	}
	if p.TerminatedHeight, err = common.ReadUint32(r); err != nil {
		return
	}
	if p.ProposalOwner, err = common.ReadVarBytes(r, crypto.NegativeBigLength,
		"proposal owner"); err != nil {
		return err
	}
	return p.TxHash.Deserialize(r)
}

func (p *ProposalState) serializeBudgets(withdrawableBudgets map[uint8]common.Fixed64,
	w io.Writer) (err error) {
	if err = common.WriteVarUint(w, uint64(len(withdrawableBudgets))); err != nil {
		return
	}
	for k, v := range withdrawableBudgets {
		if err = common.WriteElement(w, k); err != nil {
			return
		}
		if err = v.Serialize(w); err != nil {
			return
		}
	}
	return
}

func (p *ProposalState) serializeBudgetsStatus(budgetsStatus map[uint8]BudgetStatus,
	w io.Writer) (err error) {
	if err = common.WriteVarUint(w, uint64(len(budgetsStatus))); err != nil {
		return
	}
	for k, v := range budgetsStatus {
		if err = common.WriteElements(w, k, uint8(v)); err != nil {
			return
		}
	}
	return
}

func (p *ProposalState) deserializeBudgets(r io.Reader) (
	withdrawableBudgets map[uint8]common.Fixed64, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	withdrawableBudgets = make(map[uint8]common.Fixed64)
	for i := uint64(0); i < count; i++ {
		var stage uint8
		if err = common.ReadElement(r, &stage); err != nil {
			return
		}
		var amount common.Fixed64
		if err = amount.Deserialize(r); err != nil {
			return
		}
		withdrawableBudgets[stage] = amount
	}
	return
}

func (p *ProposalState) deserializeBudgetsStatus(r io.Reader) (
	budgetsStatus map[uint8]BudgetStatus, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	budgetsStatus = make(map[uint8]BudgetStatus)
	for i := uint64(0); i < count; i++ {
		var stage uint8
		var status uint8
		if err = common.ReadElements(r, &stage, &status); err != nil {
			return
		}
		budgetsStatus[stage] = BudgetStatus(status)
	}
	return
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
	if err = p.serializeProposalHashsMap(p.ProposalHashes, w); err != nil {
		return
	}
	if err = p.serializeProposalSessionMap(p.ProposalSession, w); err != nil {
		return
	}
	if err = p.serializeWithdrawableTransactionsMap(p.WithdrawableTxInfo, w); err != nil {
		return
	}
	return
}

func (p *ProposalKeyFrame) serializeProposalHashsMap(proposalHashMap map[common.Uint168]ProposalHashSet,
	w io.Writer) (err error) {
	if err = common.WriteVarUint(w, uint64(len(proposalHashMap))); err != nil {
		return
	}
	for k, ProposalHashSet := range proposalHashMap {
		if err = k.Serialize(w); err != nil {
			return
		}
		if err := common.WriteVarUint(w,
			uint64(len(ProposalHashSet))); err != nil {
			return err
		}
		for proposalHash, _ := range ProposalHashSet {
			if err := proposalHash.Serialize(w); err != nil {
				return err
			}
		}
	}
	return
}

func (p *ProposalKeyFrame) serializeProposalSessionMap(
	proposalSessionMap map[uint64][]common.Uint256, w io.Writer) (err error) {
	if err = common.WriteVarUint(w, uint64(len(proposalSessionMap))); err != nil {
		return
	}
	for k, v := range proposalSessionMap {
		if err = common.WriteUint64(w, k); err != nil {
			return
		}
		if err := common.WriteVarUint(w,
			uint64(len(v))); err != nil {
			return err
		}
		for _, proposalHash := range v {
			if err := proposalHash.Serialize(w); err != nil {
				return err
			}
		}
	}
	return
}

func (p *ProposalKeyFrame) serializeWithdrawableTransactionsMap(
	proposalWithdrableTx map[common.Uint256]types.OutputInfo, w io.Writer) (err error) {
	if err = common.WriteVarUint(w, uint64(len(proposalWithdrableTx))); err != nil {
		return
	}
	for k, v := range proposalWithdrableTx {
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
	if p.ProposalHashes, err = p.deserializeProposalHashsMap(r); err != nil {
		return
	}
	if p.ProposalSession, err = p.deserializeProposalSessionMap(r); err != nil {
		return
	}
	if p.WithdrawableTxInfo, err = p.deserializeWithdrawableTransactionsMap(r); err != nil {
		return
	}
	return
}

func (p *ProposalKeyFrame) deserializeProposalHashsMap(r io.Reader) (
	proposalHashMap map[common.Uint168]ProposalHashSet, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	proposalHashMap = make(map[common.Uint168]ProposalHashSet)
	for i := uint64(0); i < count; i++ {

		var did common.Uint168
		if err = did.Deserialize(r); err != nil {
			return
		}
		var lenProposalHashSet uint64
		proposalHashSet := NewProposalHashSet()

		if lenProposalHashSet, err = common.ReadVarUint(r, 0); err != nil {
			return
		}
		for i := uint64(0); i < lenProposalHashSet; i++ {
			hash := &common.Uint256{}
			if err = hash.Deserialize(r); err != nil {
				return
			}
			proposalHashSet.Add(*hash)
		}

		proposalHashMap[did] = proposalHashSet
	}
	return
}

func (p *ProposalKeyFrame) deserializeProposalSessionMap(r io.Reader) (
	proposalSessionMap map[uint64][]common.Uint256, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	proposalSessionMap = make(map[uint64][]common.Uint256)
	for i := uint64(0); i < count; i++ {
		var session uint64
		if session, err = common.ReadUint64(r); err != nil {
			return
		}

		var lenHashes uint64
		if lenHashes, err = common.ReadVarUint(r, 0); err != nil {
			return
		}
		hashes := make([]common.Uint256, 0, lenHashes)
		for i := uint64(0); i < lenHashes; i++ {
			hash := &common.Uint256{}
			if err = hash.Deserialize(r); err != nil {
				return
			}
			hashes = append(hashes, *hash)
		}

		proposalSessionMap[session] = hashes
	}
	return
}

func (p *ProposalKeyFrame) deserializeWithdrawableTransactionsMap(r io.Reader) (
	withdrawableTxsMap map[common.Uint256]types.OutputInfo, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	withdrawableTxsMap = make(map[common.Uint256]types.OutputInfo)
	for i := uint64(0); i < count; i++ {
		var hash common.Uint256
		if err = hash.Deserialize(r); err != nil {
			return
		}
		var withdrawInfo types.OutputInfo
		if err = withdrawInfo.Deserialize(r); err != nil {
			return
		}
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
		Proposals:          make(map[common.Uint256]*ProposalState),
		ProposalHashes:     make(map[common.Uint168]ProposalHashSet),
		ProposalSession:    make(map[uint64][]common.Uint256),
		WithdrawableTxInfo: make(map[common.Uint256]types.OutputInfo),
	}
}

func NewStateKeyFrame() *StateKeyFrame {
	return &StateKeyFrame{
		CodeCIDMap:           make(map[string]common.Uint168),
		DepositHashCIDMap:    make(map[common.Uint168]common.Uint168),
		Candidates:           make(map[common.Uint168]*Candidate),
		HistoryCandidates:    make(map[uint64]map[common.Uint168]*Candidate),
		depositInfo:          make(map[common.Uint168]*DepositInfo),
		CurrentSession:       1,
		Nicknames:            make(map[string]struct{}),
		Votes:                make(map[string]struct{}),
		DepositOutputs:       make(map[string]common.Fixed64),
		CRCFoundationOutputs: make(map[string]common.Fixed64),
		CRCCommitteeOutputs:  make(map[string]common.Fixed64),
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

// copyHistoryCandidateMap copy the CR history map's key and value, and return
// the dst map.
func copyHistoryCandidateMap(src map[uint64]map[common.Uint168]*Candidate) (
	dst map[uint64]map[common.Uint168]*Candidate) {
	dst = map[uint64]map[common.Uint168]*Candidate{}
	for k, v := range src {
		dst[k] = copyCandidateMap(v)
	}
	return
}

// copyHashIDMap copy the map's key and value, and return the dst map.
func copyHashIDMap(src map[common.Uint168]common.Uint168) (
	dst map[common.Uint168]common.Uint168) {
	dst = map[common.Uint168]common.Uint168{}
	for k, v := range src {
		dst[k] = v
	}
	return
}

// copyDepositInfoMap copy the map's key and value, and return the dst map.
func copyDepositInfoMap(src map[common.Uint168]*DepositInfo) (
	dst map[common.Uint168]*DepositInfo) {
	dst = map[common.Uint168]*DepositInfo{}
	for k, v := range src {
		d := *v
		dst[k] = &d
	}
	return
}

// copyDIDAmountMap copy the map's key and value, and return the dst map.
func copyDIDAmountMap(src map[common.Uint168]common.Fixed64) (
	dst map[common.Uint168]common.Fixed64) {
	dst = map[common.Uint168]common.Fixed64{}
	for k, v := range src {
		dst[k] = v
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

func copyFixed64Map(src map[string]common.Fixed64) (dst map[string]common.Fixed64) {
	dst = map[string]common.Fixed64{}
	for k, v := range src {
		p := v
		dst[k] = p
	}
	return
}

func getCRMembers(src map[common.Uint168]*CRMember) []*CRMember {
	dst := make([]*CRMember, 0, len(src))
	for _, v := range src {
		m := *v
		dst = append(dst, &m)
	}
	return dst
}

func getHistoryMembers(src map[uint64]map[common.Uint168]*CRMember) []*CRMember {
	dst := make([]*CRMember, 0, len(src))
	for _, v := range src {
		for _, m := range v {
			m := *m
			dst = append(dst, &m)
		}
	}
	return dst
}

func getElectedCRMembers(src map[common.Uint168]*CRMember) []*CRMember {
	dst := make([]*CRMember, 0)
	for _, v := range src {
		if v.MemberState == MemberElected {
			m := *v
			dst = append(dst, &m)
		}
	}
	return dst
}

func getImpeachableCRMembers(src map[common.Uint168]*CRMember) []*CRMember {
	dst := make([]*CRMember, 0)
	for _, v := range src {
		if v.MemberState == MemberElected || v.MemberState == MemberImpeached {
			m := *v
			dst = append(dst, &m)
		}
	}
	return dst
}

// copyMembersMap copy the CR members map's key and value, and return the dst map.
func copyMembersMap(src map[common.Uint168]*CRMember) (
	dst map[common.Uint168]*CRMember) {
	dst = map[common.Uint168]*CRMember{}
	for k, v := range src {
		p := *v
		dst[k] = &p
	}
	return
}

// copyHistoryMembersMap copy the CR members map's key and value, and return
// the dst map.
func copyHistoryMembersMap(src map[uint64]map[common.Uint168]*CRMember) (
	dst map[uint64]map[common.Uint168]*CRMember) {
	dst = map[uint64]map[common.Uint168]*CRMember{}
	for k, v := range src {
		dst[k] = copyMembersMap(v)
	}
	return
}
