package state

import (
	"sync"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/utils"
)

const (
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

	mtx     sync.RWMutex
	params  *config.Params
	history *utils.History
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

// ProcessBlock takes a block and it's confirm to update CR state and
// votes accordingly.
func (s *State) ProcessBlock(block *types.Block, confirm *payload.Confirm) {
	s.mtx.Lock()
	defer s.mtx.Unlock()

	s.processTransactions(block.Transactions, block.Height)
	s.history.Commit(block.Height)
}

// RollbackTo restores the database state to the given height, if no enough
// history to rollback to return error.
func (s *State) RollbackTo(height uint32) error {
	s.mtx.Lock()
	defer s.mtx.Unlock()
	return s.history.RollbackTo(height)
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
		s.registerCR(tx.Payload.(*payload.CRInfo), height)

	case types.UpdateCR:
		s.updateCR(tx.Payload.(*payload.CRInfo), height)

	case types.UnregisterCR:
		s.unregisterCR(tx.Payload.(*payload.UnregisterCR), height)
	}
}

// registerCR handles the register CR transaction.
func (s *State) registerCR(info *payload.CRInfo, height uint32) {
	nickname := info.NickName
	code := common.BytesToHexString(info.Code)
	candidate := Candidate{
		info:           *info,
		registerHeight: height,
		votes:          0,
		state:          Pending,
	}

	s.history.Append(height, func() {
		s.Nicknames[nickname] = struct{}{}
		s.CodeDIDMap[code] = info.DID
		s.PendingCandidates[info.DID] = &candidate
	}, func() {
		delete(s.Nicknames, nickname)
		delete(s.CodeDIDMap, code)
		delete(s.PendingCandidates, info.DID)
	})
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
		candidate.state = Active
		candidate.cancelHeight = 0
		delete(s.CanceledCandidates, key)
		if isPending {
			s.PendingCandidates[key] = candidate
		} else {
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
