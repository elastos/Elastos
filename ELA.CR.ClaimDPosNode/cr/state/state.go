// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package state

import (
	"sync"

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
	manager            *ProposalManager
	processImpeachment func([]*CRMember, common.Fixed64, []byte)

	tryStartVotingPeriod func(height uint32)
	processImpeachment   func(height uint32, member []byte, votes common.Fixed64,
		history *utils.History)

	mtx     sync.RWMutex
	params  *config.Params
	history *utils.History
}

// SetManager set current proposal manager that holds state of proposals.
func (s *State) SetManager(manager *ProposalManager) {
	s.manager = manager
}

// RegisterFunction set the tryStartVotingPeriod and processImpeachment function
// to change member state.
func (s *State) RegisterFunction(tryStartVotingPeriod func(height uint32),
	processImpeachment func(uint32, []byte, common.Fixed64, *utils.History)) {
	s.tryStartVotingPeriod = tryStartVotingPeriod
	s.processImpeachment = processImpeachment
}

// GetCandidate returns candidate with specified program code, it will return
// nil if not found.
func (s *State) GetCandidate(programCode []byte) *Candidate {
	s.mtx.RLock()
	defer s.mtx.RUnlock()
	return s.getCandidate(programCode)
}

// GetCandidateByDID returns candidate with specified did, it will return nil
// nil if not found.
func (s *State) GetCandidateByDID(did common.Uint168) *Candidate {
	s.mtx.RLock()
	defer s.mtx.RUnlock()
	return s.getCandidateByDID(did)
}

// GetAllCandidates returns all candidates holding within state.
func (s *State) GetAllCandidates() []*Candidate {
	s.mtx.RLock()
	defer s.mtx.RUnlock()

	result := s.getCandidates(Pending)
	result = append(result, s.getCandidates(Active)...)
	result = append(result, s.getCandidates(Canceled)...)
	result = append(result, s.getCandidates(Returned)...)
	return result
}

// GetCandidates returns candidates with specified candidate state.
func (s *State) GetCandidates(state CandidateState) []*Candidate {
	s.mtx.RLock()
	defer s.mtx.RUnlock()
	return s.getCandidates(state)
}

// ExistCandidate judges if there is a candidate with specified program code.
func (s *State) ExistCandidate(programCode []byte) bool {
	s.mtx.RLock()
	defer s.mtx.RUnlock()
	_, ok := s.CodeDIDMap[common.BytesToHexString(programCode)]
	return ok
}

// ExistCandidateByDID judges if there is a candidate with specified did.
func (s *State) ExistCandidateByDID(did common.Uint168) (ok bool) {
	s.mtx.RLock()
	defer s.mtx.RUnlock()

	if _, ok = s.PendingCandidates[did]; ok {
		return
	}

	if _, ok = s.ActivityCandidates[did]; ok {
		return
	}

	if _, ok = s.CanceledCandidates[did]; ok {
		return
	}
	return
}

// ExistCandidateByNickname judges if there is a candidate with specified
// nickname.
func (s *State) ExistCandidateByNickname(nickname string) bool {
	s.mtx.RLock()
	defer s.mtx.RUnlock()

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

	s.mtx.RLock()
	defer s.mtx.RUnlock()
	// Cancel votes.
	for _, input := range tx.Inputs {
		_, ok := s.Votes[input.ReferKey()]
		if ok {
			return true
		}
	}

	return false
}

// ProcessBlock takes a block and it's confirm to update CR state and
// votes accordingly.
func (s *State) ProcessBlock(block *types.Block, confirm *payload.Confirm) {
	s.mtx.Lock()
	defer s.mtx.Unlock()

	s.processTransactions(block.Transactions, block.Height)
	if s.tryStartVotingPeriod != nil {
		s.tryStartVotingPeriod(block.Height)
	}

	s.history.Commit(block.Height)
}

// ProcessElectionBlock takes a block and it's confirm to update CR member state
// and proposals accordingly, only in election period and not in voting period.
func (s *State) ProcessElectionBlock(block *types.Block) {
	s.mtx.Lock()
	defer s.mtx.Unlock()

	for _, tx := range block.Transactions {
		s.processElectionTransaction(tx, block.Height)
	}
	if s.manager != nil {
		s.manager.updateProposals(block.Height, s.history)
	}
	if s.tryStartVotingPeriod != nil {
		s.tryStartVotingPeriod(block.Height)
	}

	s.history.Commit(block.Height)
}

// processElectionTransaction take a transaction and the height it has been
// packed into a block, then update CR members state and proposals according to
// the transaction content.
func (s *State) processElectionTransaction(tx *types.Transaction, height uint32) {
	switch tx.TxType {
	case types.TransferAsset:
		s.processVotes(tx, height)

	case types.ReturnCRDepositCoin:
		s.returnDeposit(tx, height)

	case types.CRCProposal:
		if s.manager != nil {
			s.manager.registerProposal(tx, height, s.history)
		}
	}

	s.processCancelVotes(tx, height)
}

// RollbackTo restores the database state to the given height, if no enough
// history to rollback to return error.
func (s *State) RollbackTo(height uint32) error {
	s.mtx.Lock()
	defer s.mtx.Unlock()
	return s.history.RollbackTo(height)
}

// FinishVoting will close all voting util next voting period
func (s *State) FinishVoting(dids []common.Uint168) *StateKeyFrame {
	s.mtx.Lock()
	defer s.mtx.Unlock()
	for _, v := range dids {
		if _, ok := s.ActivityCandidates[v]; !ok {
			log.Warnf("not found active candidate %s when finish voting",
				v.String())
		}
		delete(s.ActivityCandidates, v)
	}
	s.history = utils.NewHistory(maxHistoryCapacity)

	result := s.StateKeyFrame.Snapshot()
	return result
}

// processTransactions takes the transactions and the height when they have been
// packed into a block.  Then loop through the transactions to update CR
// state and votes according to transactions content.
func (s *State) processTransactions(txs []*types.Transaction, height uint32) {
	for _, tx := range txs {
		s.processTransaction(tx, height)
	}

	// Check if any pending producers has got 6 confirms, set them to activate.
	activateCandidateFromPending :=
		func(key common.Uint168, candidate *Candidate) {
			s.history.Append(height, func() {
				candidate.state = Active
				s.ActivityCandidates[key] = candidate
				delete(s.PendingCandidates, key)
			}, func() {
				candidate.state = Pending
				s.PendingCandidates[key] = candidate
				delete(s.ActivityCandidates, key)
			})
		}

	if len(s.PendingCandidates) > 0 {
		for key, candidate := range s.PendingCandidates {
			if height-candidate.registerHeight+1 >= ActivateDuration {
				activateCandidateFromPending(key, candidate)
			}
		}
	}
}

// processTransaction take a transaction and the height it has been packed into
// a block, then update producers state and votes according to the transaction
// content.
func (s *State) processTransaction(tx *types.Transaction, height uint32) {
	switch tx.TxType {
	case types.RegisterCR:
		s.registerCR(tx, height)

	case types.UpdateCR:
		s.updateCR(tx.Payload.(*payload.CRInfo), height)

	case types.UnregisterCR:
		s.unregisterCR(tx.Payload.(*payload.UnregisterCR), height)

	case types.TransferAsset:
		s.processVotes(tx, height)
		s.processDeposit(tx, height)

	case types.ReturnCRDepositCoin:
		s.returnDeposit(tx, height)

	case types.CRCProposal:
		if s.manager != nil {
			s.manager.registerProposal(tx, height, s.history)
		}
	}

	if s.manager != nil {
		s.manager.updateProposals(height, s.history)
	}
	s.processCancelVotes(tx, height)
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
			s.DepositOutputs[op.ReferKey()] = output
		}
	}
	candidate.depositAmount = amount

	c := s.getCandidateByDID(info.DID)
	if c == nil {
		s.history.Append(height, func() {
			s.Nicknames[nickname] = struct{}{}
			s.CodeDIDMap[code] = info.DID
			s.PendingCandidates[info.DID] = &candidate
		}, func() {
			delete(s.Nicknames, nickname)
			delete(s.CodeDIDMap, code)
			delete(s.PendingCandidates, info.DID)
		})
	} else {
		candidate.votes = c.votes
		s.history.Append(height, func() {
			delete(s.CanceledCandidates, c.Info().DID)
			s.Nicknames[nickname] = struct{}{}
			s.PendingCandidates[info.DID] = &candidate
		}, func() {
			delete(s.PendingCandidates, info.DID)
			delete(s.Nicknames, nickname)
			s.CanceledCandidates[c.Info().DID] = c
		})
	}

}

// updateCR handles the update CR transaction.
func (s *State) updateCR(info *payload.CRInfo, height uint32) {
	candidate := s.getCandidateByDID(info.DID)
	crInfo := candidate.info
	s.history.Append(height, func() {
		s.updateCandidateInfo(&crInfo, info)
	}, func() {
		s.updateCandidateInfo(info, &crInfo)
	})
}

// unregisterCR handles the cancel producer transaction.
func (s *State) unregisterCR(info *payload.UnregisterCR, height uint32) {
	candidate := s.getCandidate(info.Code)
	key := candidate.info.DID
	isPending := candidate.state == Pending
	s.history.Append(height, func() {
		candidate.state = Canceled
		candidate.cancelHeight = height
		s.CanceledCandidates[key] = candidate
		if isPending {
			delete(s.PendingCandidates, key)
		} else {
			delete(s.ActivityCandidates, key)
		}
		delete(s.Nicknames, candidate.info.NickName)
	}, func() {
		candidate.cancelHeight = 0
		delete(s.CanceledCandidates, key)
		if isPending {
			candidate.state = Pending
			s.PendingCandidates[key] = candidate
		} else {
			candidate.state = Active
			s.ActivityCandidates[key] = candidate
		}
		s.Nicknames[candidate.info.NickName] = struct{}{}
	})
}

// updateCandidateInfo updates the candidate's info with value compare,
// any change will be updated.
func (s *State) updateCandidateInfo(origin *payload.CRInfo, update *payload.CRInfo) {
	candidate := s.getCandidateByDID(origin.DID)

	// compare and update node nickname.
	if origin.NickName != update.NickName {
		delete(s.Nicknames, origin.NickName)
		s.Nicknames[update.NickName] = struct{}{}
	}

	candidate.info = *update
}

// processVotes takes a transaction, if the transaction including any vote
// inputs or outputs, validate and update CR votes.
func (s *State) processVotes(tx *types.Transaction, height uint32) {
	if tx.Version >= types.TxVersion09 {
		for i, output := range tx.Outputs {
			if output.Type != types.OTVote {
				continue
			}
			p, _ := output.Payload.(*outputpayload.VoteOutput)
			if p.Version < outputpayload.VoteProducerAndCRVersion {
				continue
			}

			// process CRC content
			var exist bool
			for _, content := range p.Contents {
				if content.VoteType == outputpayload.CRC ||
					content.VoteType == outputpayload.CRCProposal ||
					content.VoteType == outputpayload.CRCImpeachment {
					exist = true
					break
				}
			}
			if exist {
				op := types.NewOutPoint(tx.Hash(), uint16(i))
				s.Votes[op.ReferKey()] = output
				s.processVoteOutput(output, height)
			}
		}
	}
}

// processDeposit takes a transaction output with deposit program hash.
func (s *State) processDeposit(tx *types.Transaction, height uint32) {
	for i, output := range tx.Outputs {
		if contract.GetPrefixType(output.ProgramHash) == contract.PrefixDeposit {
			if s.addCandidateAssert(output, height) {
				op := types.NewOutPoint(tx.Hash(), uint16(i))
				s.DepositOutputs[op.ReferKey()] = output
			}
		}
	}
}

// returnDeposit change producer state to ReturnedDeposit
func (s *State) returnDeposit(tx *types.Transaction, height uint32) {
	var inputValue common.Fixed64
	for _, input := range tx.Inputs {
		inputValue += s.DepositOutputs[input.ReferKey()].Value
	}

	returnAction := func(candidate *Candidate, originState CandidateState) {
		s.history.Append(height, func() {
			candidate.depositAmount -= inputValue
			candidate.state = Returned
			delete(s.Nicknames, candidate.info.NickName)
		}, func() {
			candidate.depositAmount += inputValue
			candidate.state = originState
			s.Nicknames[candidate.info.NickName] = struct{}{}
		})
	}

	for _, program := range tx.Programs {
		if candidate := s.getCandidate(program.Code); candidate != nil {
			returnAction(candidate, candidate.state)
		}
	}
}

// addCandidateAssert will plus deposit amount for candidates referenced in
// program hash of transaction output.
func (s *State) addCandidateAssert(output *types.Output, height uint32) bool {
	if candidate := s.getCandidateByDepositHash(output.ProgramHash); candidate != nil {
		s.history.Append(height, func() {
			candidate.depositAmount += output.Value
		}, func() {
			candidate.depositAmount -= output.Value
		})
		return true
	}
	return false
}

// getCandidateByDepositHash will try to get candidate with specified program
// hash, note the candidate state should be pending or active.
func (s *State) getCandidateByDepositHash(hash common.Uint168) *Candidate {
	for _, candidate := range s.PendingCandidates {
		if candidate.depositHash.IsEqual(hash) {
			return candidate
		}
	}
	for _, candidate := range s.ActivityCandidates {
		if candidate.depositHash.IsEqual(hash) {
			return candidate
		}
	}
	return nil
}

// processVoteOutput takes a transaction output with vote payload.
func (s *State) processVoteOutput(output *types.Output, height uint32) {
	p := output.Payload.(*outputpayload.VoteOutput)
	for _, vote := range p.Contents {
		for _, cv := range vote.CandidateVotes {
			switch vote.VoteType {
			case outputpayload.CRC:
				candidate := s.getCandidate(cv.Candidate)
				if candidate == nil {
					continue
				}
				v := cv.Votes
				s.history.Append(height, func() {
					candidate.votes += v
				}, func() {
					candidate.votes -= v
				})

			case outputpayload.CRCProposal:
				proposalHash, _ := common.Uint256FromBytes(cv.Candidate)
				proposalState := s.manager.GetProposal(*proposalHash)
				v := cv.Votes
				s.history.Append(height, func() {
					proposalState.VotersRejectAmount += v
				}, func() {
					proposalState.VotersRejectAmount -= v
				})
			case outputpayload.CRCImpeachment:
				s.processImpeachment(height, cv.Candidate, cv.Votes, s.history)
			}
		}
	}
}

// processCancelVotes takes a transaction, if the transaction takes a previous
// vote output then try to subtract the vote
func (s *State) processCancelVotes(tx *types.Transaction, height uint32) {
	for _, input := range tx.Inputs {
		referKey := input.ReferKey()
		output, ok := s.Votes[referKey]
		if ok {
			s.processVoteCancel(output, height)
			// todo consider rollback
			s.Votes[referKey] = nil
		}
	}
}

// processVoteCancel takes a previous vote output and decrease CR votes.
func (s *State) processVoteCancel(output *types.Output, height uint32) {
	p := output.Payload.(*outputpayload.VoteOutput)
	for _, vote := range p.Contents {
		for _, cv := range vote.CandidateVotes {
			producer := s.getCandidate(cv.Candidate)
			if producer == nil {
				continue
			}
			switch vote.VoteType {
			case outputpayload.CRC:
				v := cv.Votes
				s.history.Append(height, func() {
					producer.votes -= v
				}, func() {
					producer.votes += v
				})

			case outputpayload.CRCProposal:
				proposalHash, _ := common.Uint256FromBytes(cv.Candidate)
				proposalState := s.manager.GetProposal(*proposalHash)
				v := cv.Votes
				s.history.Append(height, func() {
					proposalState.VotersRejectAmount -= v
					//// todo get current circulation by calculation
					//circulation := common.Fixed64(3300 * 10000 * 100000000)
					//if proposalState.VotersRejectAmount > circulation/10 {
					//	proposalState.Status = VoterCanceled
					//}
				}, func() {
					proposalState.VotersRejectAmount += v
				})
			}
		}
	}
}

func (s *State) getCandidateByDID(did common.Uint168) *Candidate {
	if c, ok := s.PendingCandidates[did]; ok {
		return c
	}

	if c, ok := s.ActivityCandidates[did]; ok {
		return c
	}

	if c, ok := s.CanceledCandidates[did]; ok {
		return c
	}
	return nil
}

func (s *State) getCandidate(programCode []byte) *Candidate {
	did, ok := s.getDIDByCode(programCode)
	if !ok {
		return nil
	}
	return s.getCandidateByDID(did)
}

func (s *State) getDIDByCode(programCode []byte) (did common.Uint168,
	exist bool) {
	codeStr := common.BytesToHexString(programCode)
	did, exist = s.CodeDIDMap[codeStr]
	return
}

func (s *State) getCandidates(state CandidateState) []*Candidate {
	switch state {
	case Pending:
		return s.getCandidateFromMap(s.PendingCandidates, nil)
	case Active:
		return s.getCandidateFromMap(s.ActivityCandidates, nil)
	case Canceled:
		return s.getCandidateFromMap(s.CanceledCandidates,
			func(candidate *Candidate) bool {
				return candidate.state == Canceled
			})
	case Returned:
		return s.getCandidateFromMap(s.CanceledCandidates,
			func(candidate *Candidate) bool {
				return candidate.state == Returned
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
