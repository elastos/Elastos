package state

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
)

type State struct {
	// todo complete me
}

func (s *State) GetCandidate(programCode []byte) *Candidate {
	panic("implement me")
}

func (s *State) GetCandidateByDID(did common.Uint168) *Candidate {
	panic("implement me")
}

func (s *State) GetAllCandidates() []*Candidate {
	panic("implement me")
}

func (s *State) GetCandidates(state CandidateState) []*Candidate {
	panic("implement me")
}

func (s *State) ExistCandidate(programCode []byte) bool {
	panic("implement me")
}

func (s *State) ExistCandidateByDID(did common.Uint168) bool {
	panic("implement me")
}

func (s *State) ExistCandidateByNickname(nickname string) bool {
	panic("implement me")
}

func (s *State) ProcessBlock(block *types.Block, confirm *payload.Confirm) {
	panic("implement me")
}

func NewState(chainParams *config.Params) *State {
	return &State{}
}
