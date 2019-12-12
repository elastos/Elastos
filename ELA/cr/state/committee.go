// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package state

import (
	"bytes"
	"errors"
	"fmt"
	"sort"
	"sync"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
)

type Committee struct {
	KeyFrame
	mtx    sync.RWMutex
	state  *State
	params *config.Params

	getCheckpoint func(height uint32) *Checkpoint
}

func (c *Committee) GetState() *State {
	return c.state
}

func (c *Committee) ExistCR(programCode []byte) bool {
	existCandidate := c.state.ExistCandidate(programCode)
	if existCandidate {
		return true
	}

	c.mtx.RLock()
	defer c.mtx.RUnlock()
	for _, v := range c.Members {
		if bytes.Equal(programCode, v.Info.Code) {
			return true
		}
	}
	return false
}

func (c *Committee) IsInVotingPeriod(height uint32) bool {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.isInVotingPeriod(height)
}

func (c *Committee) GetMembersDIDs() []common.Uint168 {
	c.mtx.RLock()
	defer c.mtx.RUnlock()

	result := make([]common.Uint168, 0, len(c.Members))
	for _, v := range c.Members {
		result = append(result, v.Info.DID)
	}
	return result
}

//get all CRMembers
func (c *Committee) GetAllMembers() []*CRMember {
	c.mtx.RLock()
	defer c.mtx.RUnlock()

	return copyCRMembers(c.Members)
}

func (c *Committee) GetMembersCodes() [][]byte {
	c.mtx.RLock()
	defer c.mtx.RUnlock()

	result := make([][]byte, 0, len(c.Members))
	for _, v := range c.Members {
		result = append(result, v.Info.Code)
	}
	return result
}

func (c *Committee) ProcessBlock(block *types.Block, confirm *payload.Confirm) {
	c.mtx.Lock()
	defer c.mtx.Unlock()
	isVoting := c.isInVotingPeriod(block.Height)

	if isVoting {
		c.state.ProcessBlock(block, confirm)
	} else {
		c.state.ProcessReturnDepositTxs(block)
	}

	if c.shouldChange(block) {
		committeeDIDs, err := c.changeCommitteeMembers(block.Height)
		if err != nil {
			log.Error("[ProcessBlock] change committee members error: ", err)
			return
		}

		checkpoint := Checkpoint{
			KeyFrame: c.KeyFrame,
		}
		checkpoint.StateKeyFrame = *c.state.FinishVoting(committeeDIDs)
	}
}

func (c *Committee) RollbackTo(height uint32) error {
	c.mtx.Lock()
	defer c.mtx.Unlock()
	lastCommitHeight := c.LastCommitteeHeight

	if height >= lastCommitHeight {
		if height > c.state.history.Height() {
			return fmt.Errorf("can't rollback to height: %d", height)
		}

		if err := c.state.RollbackTo(height); err != nil {
			log.Warn("state rollback err: ", err)
		}
	} else {
		point := c.getCheckpoint(height)
		if point == nil {
			return errors.New("can't find checkpoint")
		}

		c.state.StateKeyFrame = point.StateKeyFrame
		c.KeyFrame = point.KeyFrame
	}
	return nil
}

func (c *Committee) Recover(checkpoint *Checkpoint) {
	c.mtx.Lock()
	defer c.mtx.Unlock()
	c.state.StateKeyFrame = checkpoint.StateKeyFrame
	c.KeyFrame = checkpoint.KeyFrame
}

func (c *Committee) shouldChange(block *types.Block) bool {
	//todo judge by change cr committee tx later
	return block.Height >= c.params.CRCommitteeStartHeight &&
		(c.LastCommitteeHeight == 0 || block.Height >=
			c.LastCommitteeHeight+c.params.CRDutyPeriod)
}

func (c *Committee) isInVotingPeriod(height uint32) bool {
	//todo consider emergency election later
	inVotingPeriod := func(committeeUpdateHeight uint32) bool {
		return height >= committeeUpdateHeight-c.params.CRVotingPeriod &&
			height < committeeUpdateHeight
	}

	if c.LastCommitteeHeight < c.params.CRCommitteeStartHeight {
		return height >= c.params.CRVotingStartHeight &&
			height < c.params.CRCommitteeStartHeight
	} else {
		return inVotingPeriod(c.LastCommitteeHeight + c.params.CRDutyPeriod)
	}
}

func (c *Committee) changeCommitteeMembers(height uint32) (
	[]common.Uint168, error) {
	candidates, err := c.getActiveCRCandidatesDesc()
	if err != nil {
		return nil, err
	}

	result := make([]common.Uint168, 0, c.params.CRMemberCount)
	c.Members = make([]*CRMember, 0, c.params.CRMemberCount)
	for i := 0; i < int(c.params.CRMemberCount); i++ {
		c.Members = append(c.Members, c.generateMember(candidates[i]))
		result = append(result, candidates[i].info.DID)
	}

	c.LastCommitteeHeight = height
	return result, nil
}

func (c *Committee) generateMember(candidate *Candidate) *CRMember {
	return &CRMember{
		Info:             candidate.info,
		ImpeachmentVotes: 0,
		DepositHash:      candidate.depositHash,
		DepositAmount:    candidate.depositAmount,
		Penalty:          candidate.penalty,
	}
}

func (c *Committee) getActiveCRCandidatesDesc() ([]*Candidate, error) {
	candidates := c.state.GetCandidates(Active)
	if uint32(len(candidates)) < c.params.CRMemberCount {
		return nil, errors.New("candidates count less than required count")
	}

	sort.Slice(candidates, func(i, j int) bool {
		if candidates[i].votes == candidates[j].votes {
			iCRInfo := candidates[i].Info()
			jCRInfo := candidates[j].Info()
			return iCRInfo.GetCodeHash().Compare(jCRInfo.GetCodeHash()) < 0
		}
		return candidates[i].votes > candidates[j].votes
	})
	return candidates, nil
}

func NewCommittee(params *config.Params) *Committee {
	committee := &Committee{
		state:    NewState(params),
		params:   params,
		KeyFrame: *NewKeyFrame(),
	}
	params.CkpManager.Register(NewCheckpoint(committee))
	return committee
}
