// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package state

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/utils"
)

const (
	// MinDepositAmount is the minimum deposit as a producer.
	MinDepositAmount = 5000 * 100000000

	// maxHistoryCapacity indicates the maximum capacity of change history.
	maxHistoryCapacity = 10

	// ActivateDuration is about how long we should activate from pending or
	// inactive state.
	ActivateDuration = 6
)

// State hold all CR candidates related information, and process block by block
// to update votes and any other changes about candidates.
type State struct {
	StateKeyFrame
	manager *ProposalManager

	getHistoryMember func(code []byte) *CRMember
	getTxReference   func(tx *types.Transaction) (
		map[*types.Input]*types.Output, error)

	params  *config.Params
	history *utils.History
}

// SetManager set current proposal manager that holds state of proposals.
func (s *State) SetManager(manager *ProposalManager) {
	s.manager = manager
}

type FunctionsConfig struct {
	GetHistoryMember func(code []byte) *CRMember
	GetTxReference   func(tx *types.Transaction) (
		map[*types.Input]*types.Output, error)
}

// registerFunctions set the tryStartVotingPeriod and processImpeachment function
// to change member state.
func (s *State) registerFunctions(cfg *FunctionsConfig) {
	s.getHistoryMember = cfg.GetHistoryMember
	s.getTxReference = cfg.GetTxReference
}

// getAllCandidates returns all candidates holding within state.
func (s *State) getAllCandidates() []*Candidate {
	return s.getCandidateFromMap(s.Candidates, nil)
}

func (s *State) exist(did common.Uint168) bool {
	_, ok := s.depositInfo[did]
	return ok
}

func (s *State) isRefundable(did common.Uint168) bool {
	return s.depositInfo[did].Refundable
}

// getTotalAmount returns total amount with specified candidate or member did.
func (s *State) getTotalAmount(did common.Uint168) common.Fixed64 {
	return s.depositInfo[did].TotalAmount
}

// getDepositAmount returns deposit amount with specified candidate or member did.
func (s *State) getDepositAmount(did common.Uint168) common.Fixed64 {
	return s.depositInfo[did].DepositAmount
}

// getPenalty returns penalty with specified candidate or member did.
func (s *State) getPenalty(did common.Uint168) common.Fixed64 {
	return s.depositInfo[did].Penalty
}

// getAvailableDepositAmount returns available deposit amount with specified
// candidate or member did.
func (s *State) getAvailableDepositAmount(did common.Uint168,
	currentHeight uint32, isInVotingPeriod bool) common.Fixed64 {

	var lockedDepositAmount common.Fixed64
	if isInVotingPeriod {
		c := s.getCandidate(did)
		if c != nil && c.state == Canceled && currentHeight-c.cancelHeight <
			s.params.CRDepositLockupBlocks {
			lockedDepositAmount = MinDepositAmount
		}
	}
	depositInfo := s.depositInfo[did]
	return depositInfo.TotalAmount - depositInfo.DepositAmount -
		depositInfo.Penalty - lockedDepositAmount
}

// existCandidate judges if there is a candidate with specified program code.
func (s *State) existCandidate(programCode []byte) bool {
	_, ok := s.CodeDIDMap[common.BytesToHexString(programCode)]
	return ok
}

// ExistCandidateByDID judges if there is a candidate with specified did.
func (s *State) ExistCandidateByDID(did common.Uint168) (ok bool) {
	if _, ok = s.Candidates[did]; ok {
		return
	}
	return
}

// existCandidateByDepositHash judges if there is a candidate with deposit hash.
func (s *State) existCandidateByDepositHash(did common.Uint168) bool {
	_, ok := s.DepositHashDIDMap[did]
	return ok
}

// existCandidateByNickname judges if there is a candidate with specified
// nickname.
func (s *State) existCandidateByNickname(nickname string) bool {
	_, ok := s.Nicknames[nickname]
	return ok
}

// IsCRTransaction returns if a transaction will change the CR and votes state.
func (s *State) IsCRTransaction(tx *types.Transaction) bool {
	switch tx.TxType {
	// Transactions will changes the producers state.
	case types.RegisterCR, types.UpdateCR,
		types.UnregisterCR, types.ReturnCRDepositCoin:
		return true

	// Transactions will change the producer votes state.
	case types.TransferAsset:
		if tx.Version >= types.TxVersion09 {
			for _, output := range tx.Outputs {
				if output.Type != types.OTVote {
					continue
				}
				p, _ := output.Payload.(*outputpayload.VoteOutput)
				if p.Version < outputpayload.VoteProducerAndCRVersion {
					continue
				}
				for _, content := range p.Contents {
					if content.VoteType == outputpayload.CRC {
						return true
					}
				}
			}
		}
	}

	// Cancel votes.
	for _, input := range tx.Inputs {
		_, ok := s.Votes[input.ReferKey()]
		if ok {
			return true
		}
	}

	return false
}

// rollbackTo restores the database state to the given height, if no enough
// history to rollback to return error.
func (s *State) rollbackTo(height uint32) error {
	return s.history.RollbackTo(height)
}

// registerCR handles the register CR transaction.
func (s *State) registerCR(tx *types.Transaction, height uint32) {
	info := tx.Payload.(*payload.CRInfo)
	nickname := info.NickName
	code := common.BytesToHexString(info.Code)

	depositContract, _ := contract.CreateDepositContractByCode(info.Code)
	candidate := Candidate{
		info:           *info,
		registerHeight: height,
		votes:          0,
		state:          Pending,
		depositHash:    *depositContract.ToProgramHash(),
	}

	amount := common.Fixed64(0)
	for i, output := range tx.Outputs {
		if output.ProgramHash.IsEqual(candidate.depositHash) {
			amount += output.Value
			op := types.NewOutPoint(tx.Hash(), uint16(i))
			s.DepositOutputs[op.ReferKey()] = output.Value
		}
	}

	var firstTimeRegister bool
	if _, ok := s.depositInfo[info.DID]; !ok {
		firstTimeRegister = true
	}

	s.history.Append(height, func() {
		if firstTimeRegister {
			s.depositInfo[info.DID] = &DepositInfo{}
		}
		s.depositInfo[info.DID].DepositAmount += MinDepositAmount
		s.depositInfo[info.DID].TotalAmount += amount
		s.Nicknames[nickname] = struct{}{}
		s.CodeDIDMap[code] = info.DID
		s.DepositHashDIDMap[candidate.depositHash] = info.DID
		s.Candidates[info.DID] = &candidate
	}, func() {
		s.depositInfo[info.DID].DepositAmount -= MinDepositAmount
		s.depositInfo[info.DID].TotalAmount -= amount
		if firstTimeRegister {
			delete(s.depositInfo, info.DID)
		}
		delete(s.Nicknames, nickname)
		delete(s.CodeDIDMap, code)
		delete(s.DepositHashDIDMap, candidate.depositHash)
		delete(s.Candidates, info.DID)
	})
}

// updateCR handles the update CR transaction.
func (s *State) updateCR(info *payload.CRInfo, height uint32) {
	candidate := s.getCandidate(info.DID)
	crInfo := candidate.info
	s.history.Append(height, func() {
		s.updateCandidateInfo(&crInfo, info)
	}, func() {
		s.updateCandidateInfo(info, &crInfo)
	})
}

// unregisterCR handles the cancel producer transaction.
func (s *State) unregisterCR(info *payload.UnregisterCR, height uint32) {
	candidate := s.getCandidate(info.DID)
	if candidate == nil {
		return
	}
	oriState := candidate.state
	oriRefundable := s.depositInfo[info.DID].Refundable
	s.history.Append(height, func() {
		s.depositInfo[info.DID].DepositAmount -= MinDepositAmount
		s.depositInfo[info.DID].Refundable = true
		candidate.cancelHeight = height
		candidate.state = Canceled
		delete(s.Nicknames, candidate.info.NickName)
	}, func() {
		s.depositInfo[info.DID].DepositAmount += MinDepositAmount
		s.depositInfo[info.DID].Refundable = oriRefundable
		candidate.cancelHeight = 0
		candidate.state = oriState
		s.Nicknames[candidate.info.NickName] = struct{}{}
	})
}

// updateCandidateInfo updates the candidate's info with value compare,
// any change will be updated.
func (s *State) updateCandidateInfo(origin *payload.CRInfo, update *payload.CRInfo) {
	candidate := s.getCandidate(origin.DID)

	// compare and update node nickname.
	if origin.NickName != update.NickName {
		delete(s.Nicknames, origin.NickName)
		s.Nicknames[update.NickName] = struct{}{}
	}

	candidate.info = *update
}

// processDeposit takes a transaction output with deposit program hash.
func (s *State) processDeposit(tx *types.Transaction, height uint32) {
	for i, output := range tx.Outputs {
		if contract.GetPrefixType(output.ProgramHash) == contract.PrefixDeposit {
			if s.addCRCRelatedAssert(output, height) {
				op := types.NewOutPoint(tx.Hash(), uint16(i))
				s.DepositOutputs[op.ReferKey()] = output.Value
			}
		}
	}
}

// returnDeposit change producer state to ReturnedDeposit
func (s *State) returnDeposit(tx *types.Transaction, height uint32) {
	var inputValue common.Fixed64
	for _, input := range tx.Inputs {
		inputValue += s.DepositOutputs[input.ReferKey()]
	}

	returnCandidateAction := func(candidate *Candidate, originState CandidateState) {
		s.history.Append(height, func() {
			candidate.state = Returned
			delete(s.Nicknames, candidate.info.NickName)
		}, func() {
			candidate.state = originState
			s.Nicknames[candidate.info.NickName] = struct{}{}
		})
	}

	returnMemberAction := func(member *CRMember, originState MemberState) {
		s.history.Append(height, func() {
			member.MemberState = MemberReturned
		}, func() {
			member.MemberState = originState
		})
	}

	updateAmountAction := func(did common.Uint168) {
		s.history.Append(height, func() {
			s.depositInfo[did].TotalAmount -= inputValue
			s.depositInfo[did].Refundable = false
		}, func() {
			s.depositInfo[did].TotalAmount += inputValue
			s.depositInfo[did].Refundable = true
		})
	}

	for _, program := range tx.Programs {
		did, _ := getDIDByCode(program.Code)
		if candidate := s.getCandidate(*did); candidate != nil {
			returnCandidateAction(candidate, candidate.state)
		}
		if candidates := s.getHistoryCandidate(*did); len(candidates) != 0 {
			for _, c := range candidates {
				if c.state != Returned {
					returnCandidateAction(c, c.state)
				}
			}
		}
		if member := s.getHistoryMember(program.Code); member != nil {
			returnMemberAction(member, member.MemberState)
		}
		updateAmountAction(*did)
	}
}

// addCRCRelatedAssert will plus deposit amount for CRC referenced in
// program hash of transaction output.
func (s *State) addCRCRelatedAssert(output *types.Output, height uint32) bool {
	if did, ok := s.getDIDByDepositHash(output.ProgramHash); ok {
		s.history.Append(height, func() {
			s.depositInfo[did].TotalAmount += output.Value
		}, func() {
			s.depositInfo[did].TotalAmount -= output.Value
		})
		return true
	}
	return false
}

// getDIDByDepositHash will try to get did of candidate or member with specified
// program hash.
func (s *State) getDIDByDepositHash(hash common.Uint168) (common.Uint168, bool) {
	did, ok := s.DepositHashDIDMap[hash]
	return did, ok
}

// processCancelVotes takes a transaction, if the transaction takes a previous
// vote output then try to subtract the vote.
func (s *State) processCancelVotes(tx *types.Transaction, height uint32) {
	var exist bool
	for _, input := range tx.Inputs {
		referKey := input.ReferKey()
		if _, ok := s.Votes[referKey]; ok {
			exist = true
		}
	}
	if !exist {
		return
	}

	references, err := s.getTxReference(tx)
	if err != nil {
		log.Errorf("get tx reference failed, tx hash:%s", tx.Hash())
		return
	}
	for _, input := range tx.Inputs {
		referKey := input.ReferKey()
		_, ok := s.Votes[referKey]
		if ok {
			s.processVoteCancel(references[input], height)
		}
	}
}

// processVoteCRC record candidate votes.
func (s *State) processVoteCRC(height uint32, cv outputpayload.CandidateVotes) {
	did, err := common.Uint168FromBytes(cv.Candidate)
	if err != nil {
		return
	}
	candidate := s.getCandidate(*did)
	if candidate == nil {
		return
	}
	v := cv.Votes
	s.history.Append(height, func() {
		candidate.votes += v
	}, func() {
		candidate.votes -= v
	})
}

// processVoteCRCProposal record proposal reject votes.
func (s *State) processVoteCRCProposal(height uint32,
	cv outputpayload.CandidateVotes) {
	proposalHash, _ := common.Uint256FromBytes(cv.Candidate)
	proposalState := s.manager.getProposal(*proposalHash)
	v := cv.Votes
	s.history.Append(height, func() {
		proposalState.VotersRejectAmount += v
	}, func() {
		proposalState.VotersRejectAmount -= v
	})
}

// processVoteCancel takes a previous vote output and decrease CR votes.
func (s *State) processVoteCancel(output *types.Output, height uint32) {
	p := output.Payload.(*outputpayload.VoteOutput)
	for _, vote := range p.Contents {
		for _, cv := range vote.CandidateVotes {
			switch vote.VoteType {
			case outputpayload.CRC:
				did, err := common.Uint168FromBytes(cv.Candidate)
				if err != nil {
					continue
				}
				candidate := s.getCandidate(*did)
				if candidate == nil {
					continue
				}
				v := cv.Votes
				s.history.Append(height, func() {
					candidate.votes -= v
				}, func() {
					candidate.votes += v
				})

			case outputpayload.CRCProposal:
				proposalHash, _ := common.Uint256FromBytes(cv.Candidate)
				proposalState := s.manager.getProposal(*proposalHash)
				v := cv.Votes
				s.history.Append(height, func() {
					proposalState.VotersRejectAmount -= v
				}, func() {
					proposalState.VotersRejectAmount += v
				})
			}
		}
	}
}

// getCandidate returns candidate with specified did, it will return nil
// nil if not found.
func (s *State) getCandidate(did common.Uint168) *Candidate {
	if c, ok := s.Candidates[did]; ok {
		return c
	}
	return nil
}

func (s *State) getHistoryCandidate(did common.Uint168) []*Candidate {
	candidates := make([]*Candidate, 0)
	for _, v := range s.HistoryCandidates {
		if c, ok := v[did]; ok {
			candidates = append(candidates, c)
		}
	}
	return candidates
}

func (s *State) getDIDByCode(programCode []byte) (did common.Uint168,
	exist bool) {
	codeStr := common.BytesToHexString(programCode)
	did, exist = s.CodeDIDMap[codeStr]
	return
}

// getCandidates returns candidates with specified candidate state.
func (s *State) getCandidates(state CandidateState) []*Candidate {
	switch state {
	case Pending, Active, Canceled, Returned:
		return s.getCandidateFromMap(s.Candidates,
			func(candidate *Candidate) bool {
				return candidate.state == state
			})
	default:
		return []*Candidate{}
	}
}

func (s *State) getCandidateFromMap(cmap map[common.Uint168]*Candidate,
	filter func(*Candidate) bool) []*Candidate {
	result := make([]*Candidate, 0, len(cmap))
	for _, v := range cmap {
		if filter != nil && !filter(v) {
			continue
		}
		result = append(result, v)
	}
	return result
}

func NewState(chainParams *config.Params) *State {
	return &State{
		StateKeyFrame: *NewStateKeyFrame(),
		params:        chainParams,
		history:       utils.NewHistory(maxHistoryCapacity),
	}
}
