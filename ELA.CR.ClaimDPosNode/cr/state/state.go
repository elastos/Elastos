// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package state

import (
	"errors"
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

	getHistoryMember func(code []byte) []*CRMember
	getTxReference   func(tx *types.Transaction) (
		map[*types.Input]types.Output, error)

	params  *config.Params
	history *utils.History
}

// SetManager set current proposal manager that holds state of proposals.
func (s *State) SetManager(manager *ProposalManager) {
	s.manager = manager
}

type FunctionsConfig struct {
	GetHistoryMember func(code []byte) []*CRMember
	GetTxReference   func(tx *types.Transaction) (
		map[*types.Input]types.Output, error)
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

func (s *State) exist(cid common.Uint168) bool {
	_, ok := s.depositInfo[cid]
	return ok
}

// getTotalAmount returns total amount with specified candidate or member cid.
func (s *State) getTotalAmount(cid common.Uint168) common.Fixed64 {
	return s.depositInfo[cid].TotalAmount
}

// getDepositAmount returns deposit amount with specified candidate or member cid.
func (s *State) getDepositAmount(cid common.Uint168) common.Fixed64 {
	return s.depositInfo[cid].DepositAmount
}

// getPenalty returns penalty with specified candidate or member cid.
func (s *State) getPenalty(cid common.Uint168) common.Fixed64 {
	return s.depositInfo[cid].Penalty
}

// getAvailableDepositAmount returns available deposit amount with specified
// candidate or member cid.
func (s *State) getAvailableDepositAmount(cid common.Uint168) common.Fixed64 {
	depositInfo, ok := s.depositInfo[cid]
	if !ok {
		return 0
	}
	return depositInfo.TotalAmount - depositInfo.DepositAmount -
		depositInfo.Penalty
}

// getDepositAmountByCID returns available deposit amount and penalty with
// specified cid.
func (s *State) getDepositAmountByCID(
	cid common.Uint168) (common.Fixed64, common.Fixed64, error) {
	depositInfo, ok := s.depositInfo[cid]
	if !ok {
		return 0, 0, errors.New("deposit information does not exist")
	}
	return depositInfo.TotalAmount - depositInfo.DepositAmount -
		depositInfo.Penalty, depositInfo.Penalty, nil
}

// getDepositAmountByPublicKey return available deposit amount and
// penalty by the given public key.
func (s *State) getDepositAmountByPublicKey(
	publicKey []byte) (common.Fixed64, common.Fixed64, error) {
	cid, err := getCIDByPublicKey(publicKey)
	if err != nil {
		return 0, 0, err
	}
	depositInfo, ok := s.depositInfo[*cid]
	if !ok {
		return 0, 0, errors.New("CID does not exist")
	}
	return depositInfo.TotalAmount - depositInfo.DepositAmount -
		depositInfo.Penalty, depositInfo.Penalty, nil
}

// existCandidate judges if there is a candidate with specified program code.
func (s *State) existCandidate(programCode []byte) bool {
	_, ok := s.CodeCIDMap[common.BytesToHexString(programCode)]
	return ok
}

// ExistCandidateByCID judges if there is a candidate with specified cid.
func (s *State) ExistCandidateByCID(cid common.Uint168) (ok bool) {
	if _, ok = s.Candidates[cid]; ok {
		return
	}
	return
}

// existCandidateByDepositHash judges if there is a candidate with deposit hash.
func (s *State) existCandidateByDepositHash(hash common.Uint168) bool {
	_, ok := s.DepositHashCIDMap[hash]
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
	if _, ok := s.depositInfo[info.CID]; !ok {
		firstTimeRegister = true
	}
	s.history.Append(height, func() {
		if firstTimeRegister {
			s.depositInfo[info.CID] = &DepositInfo{}
			s.CodeCIDMap[code] = info.CID
			s.DepositHashCIDMap[candidate.depositHash] = info.CID
		}
		s.Nicknames[nickname] = struct{}{}
		s.Candidates[info.CID] = &candidate
		s.depositInfo[info.CID].DepositAmount += MinDepositAmount
		s.depositInfo[info.CID].TotalAmount += amount
	}, func() {
		delete(s.Candidates, info.CID)
		delete(s.Nicknames, nickname)
		s.depositInfo[info.CID].DepositAmount -= MinDepositAmount
		s.depositInfo[info.CID].TotalAmount -= amount
		if firstTimeRegister {
			delete(s.depositInfo, info.CID)
			delete(s.CodeCIDMap, code)
			delete(s.DepositHashCIDMap, candidate.depositHash)
		}
	})
}

// updateCR handles the update CR transaction.
func (s *State) updateCR(info *payload.CRInfo, height uint32) {
	candidate := s.getCandidate(info.CID)
	crInfo := candidate.info
	s.history.Append(height, func() {
		s.updateCandidateInfo(&crInfo, info)
	}, func() {
		s.updateCandidateInfo(info, &crInfo)
	})
}

// unregisterCR handles the cancel producer transaction.
func (s *State) unregisterCR(info *payload.UnregisterCR, height uint32) {
	candidate := s.getCandidate(info.CID)
	if candidate == nil {
		return
	}
	oriState := candidate.state
	s.history.Append(height, func() {
		candidate.cancelHeight = height
		candidate.state = Canceled
		delete(s.Nicknames, candidate.info.NickName)
	}, func() {
		candidate.cancelHeight = 0
		candidate.state = oriState
		s.Nicknames[candidate.info.NickName] = struct{}{}
	})
}

// updateCandidateInfo updates the candidate's info with value compare,
// any change will be updated.
func (s *State) updateCandidateInfo(origin *payload.CRInfo, update *payload.CRInfo) {
	candidate := s.getCandidate(origin.CID)

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
		}, func() {
			candidate.state = originState
		})
	}

	returnMemberAction := func(member *CRMember, originState MemberState) {
		s.history.Append(height, func() {
			member.MemberState = MemberReturned
		}, func() {
			member.MemberState = originState
		})
	}

	updateAmountAction := func(cid common.Uint168) {
		s.history.Append(height, func() {
			s.depositInfo[cid].TotalAmount -= inputValue
			s.depositInfo[cid].Refundable = false
		}, func() {
			s.depositInfo[cid].TotalAmount += inputValue
			s.depositInfo[cid].Refundable = true
		})
	}

	for _, program := range tx.Programs {
		cid, _ := getCIDByCode(program.Code)

		if candidate := s.getCandidate(*cid); candidate != nil {
			if candidate.state == Canceled {
				if height-candidate.cancelHeight > s.params.CRDepositLockupBlocks {
					returnCandidateAction(candidate, candidate.state)
				}
			}
		}
		if candidates := s.getHistoryCandidate(*cid); len(candidates) != 0 {
			for _, c := range candidates {
				if c.state != Returned {
					returnCandidateAction(c, c.state)
				}
			}
		}
		if members := s.getHistoryMember(program.Code); len(members) != 0 {
			for _, m := range members {
				returnMemberAction(m, m.MemberState)
			}
		}

		updateAmountAction(*cid)
	}
}

// addCRCRelatedAssert will plus deposit amount for CRC referenced in
// program hash of transaction output.
func (s *State) addCRCRelatedAssert(output *types.Output, height uint32) bool {
	if cid, ok := s.getCIDByDepositHash(output.ProgramHash); ok {
		s.history.Append(height, func() {
			s.depositInfo[cid].TotalAmount += output.Value
		}, func() {
			s.depositInfo[cid].TotalAmount -= output.Value
		})
		return true
	}
	return false
}

// getCIDByDepositHash will try to get cid of candidate or member with specified
// program hash.
func (s *State) getCIDByDepositHash(hash common.Uint168) (common.Uint168, bool) {
	cid, ok := s.DepositHashCIDMap[hash]
	return cid, ok
}

// processVoteCRC record candidate votes.
func (s *State) processVoteCRC(height uint32, cv outputpayload.CandidateVotes) {
	cid, err := common.Uint168FromBytes(cv.Candidate)
	if err != nil {
		return
	}
	candidate := s.getCandidate(*cid)
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
	proposalHash, err := common.Uint256FromBytes(cv.Candidate)
	if err != nil {
		return
	}
	proposalState := s.manager.getProposal(*proposalHash)
	if proposalState == nil || proposalState.Status != CRAgreed {
		return
	}
	v := cv.Votes
	s.history.Append(height, func() {
		proposalState.VotersRejectAmount += v
	}, func() {
		proposalState.VotersRejectAmount -= v
	})
}

// getCandidate returns candidate with specified cid, it will return nil
// nil if not found.
func (s *State) getCandidate(cid common.Uint168) *Candidate {
	if c, ok := s.Candidates[cid]; ok {
		return c
	}
	return nil
}

// getExistCIDByPublicKey return existing candidate by the given CID or DID.
func (s *State) getCandidateByID(id common.Uint168) *Candidate {
	for k, v := range s.CodeCIDMap {
		if v.IsEqual(id) {
			return s.getCandidate(v)
		}
		code, err := common.HexStringToBytes(k)
		if err != nil {
			return nil
		}
		did, err := getDIDByCode(code)
		if err != nil {
			return nil
		}
		if did.IsEqual(id) {
			return s.getCandidate(v)
		}
	}
	return nil
}

// getExistCIDByID return existing CID by the given CID or DID.
func (s *State) getExistCIDByID(id common.Uint168) (*common.Uint168, bool) {
	for k, v := range s.CodeCIDMap {
		cid := v
		if cid.IsEqual(id) {
			return &cid, true
		}
		code, err := common.HexStringToBytes(k)
		if err != nil {
			return nil, false
		}
		did, err := getDIDByCode(code)
		if err != nil {
			return nil, false
		}
		if did.IsEqual(id) {
			return &cid, true
		}
	}
	return nil, false
}

// getExistCIDByID return existing candidate by the given public key.
func (s *State) getCandidateByPublicKey(publicKey []byte) *Candidate {
	cid, err := getCIDByPublicKey(publicKey)
	if err != nil {
		return nil
	}
	return s.getCandidate(*cid)
}

func (s *State) getHistoryCandidate(cid common.Uint168) []*Candidate {
	candidates := make([]*Candidate, 0)
	for _, v := range s.HistoryCandidates {
		if c, ok := v[cid]; ok {
			candidates = append(candidates, c)
		}
	}
	return candidates
}

func (s *State) getCandidateByCode(programCode []byte) *Candidate {
	cid, ok := s.getCIDByCode(programCode)
	if !ok {
		return nil
	}
	return s.getCandidate(cid)
}

func (s *State) getCIDByCode(programCode []byte) (cid common.Uint168,
	exist bool) {
	codeStr := common.BytesToHexString(programCode)
	cid, exist = s.CodeCIDMap[codeStr]
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
