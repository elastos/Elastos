package state

import (
	"sync"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
)

type State struct {
	KeyFrame

	mtx    sync.RWMutex
	params *config.Params
}

func (s *State) GetCandidate(programCode []byte) *Candidate {
	s.mtx.RLock()
	defer s.mtx.RUnlock()

	did, ok := s.getDIDByCode(programCode)
	if !ok {
		return nil
	}
	return s.getCandidateByDID(did)
}

func (s *State) GetCandidateByDID(did common.Uint168) *Candidate {
	s.mtx.RLock()
	defer s.mtx.RUnlock()
	return s.getCandidateByDID(did)
}

func (s *State) GetAllCandidates() []*Candidate {
	s.mtx.RLock()
	defer s.mtx.RUnlock()

	result := s.getCandidates(Pending)
	result = append(result, s.getCandidates(Active)...)
	result = append(result, s.getCandidates(Canceled)...)
	result = append(result, s.getCandidates(Returned)...)
	return result
}

func (s *State) GetCandidates(state CandidateState) []*Candidate {
	s.mtx.RLock()
	defer s.mtx.RUnlock()
	return s.getCandidates(state)
}

func (s *State) ExistCandidate(programCode []byte) bool {
	s.mtx.RLock()
	defer s.mtx.RUnlock()
	_, ok := s.CodeDIDMap[common.BytesToHexString(programCode)]
	return ok
}

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

func (s *State) ExistCandidateByNickname(nickname string) bool {
	s.mtx.RLock()
	defer s.mtx.RUnlock()

	_, ok := s.Nicknames[nickname]
	return ok
}

func (s *State) ProcessBlock(block *types.Block, confirm *payload.Confirm) {
	panic("implement me")
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
		KeyFrame: *NewKeyFrame(),
		params:   chainParams,
	}
}
