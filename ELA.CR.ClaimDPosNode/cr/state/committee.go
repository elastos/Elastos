package state

import (
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
	store  ICRRecord
	params *config.Params
}

func (c *Committee) GetState() *State {
	return c.state
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

func (c *Committee) GetMembersCodes() [][]byte {
	c.mtx.RLock()
	defer c.mtx.RUnlock()

	result := make([][]byte, 0, len(c.Members))
	for _, v := range c.Members {
		result = append(result, v.Info.Code)
	}
	return result
}

func (c *Committee) ProcessBlock(block *types.Block,
	confirm *payload.Confirm) {
	c.mtx.Lock()
	defer c.mtx.Unlock()
	isVoting := c.isInVotingPeriod(block.Height)

	if isVoting {
		c.state.ProcessBlock(block, confirm)
	}

	if c.shouldChange(block) {
		checkpoint := CheckPoint{
			KeyFrame: c.KeyFrame,
		}

		if err := c.changeCommitteeMembers(block.Height); err != nil {
			log.Error("[ProcessBlock] change committee members error: ", err)
			return
		}

		checkpoint.StateKeyFrame = *c.state.FinishVoting()
		if c.store != nil {
			if err := c.store.SaveCheckpoint(&checkpoint); err != nil {
				log.Error("[ProcessBlock] save checkpoint error: ", err)
				return
			}
		}
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
		if c.store == nil {
			return errors.New("can't find check point store")
		}
		point, err := c.store.GetCheckpoint(height)
		if err != nil {
			return err
		}

		c.state.StateKeyFrame = point.StateKeyFrame
		c.KeyFrame = point.KeyFrame
	}
	return nil
}

func (c *Committee) shouldChange(block *types.Block) bool {
	//todo judge by change cr committee tx later
	return block.Height >= c.params.CRCommitteeStartHeight &&
		block.Height >= c.LastCommitteeHeight+c.params.CRDutyPeriod
}

func (c *Committee) isInVotingPeriod(height uint32) bool {
	//todo consider emergency election later
	inVotingPeriod := func(committeeUpdateHeight uint32) bool {
		return height >= committeeUpdateHeight-c.params.CRVotingPeriod &&
			height < committeeUpdateHeight
	}

	if c.LastCommitteeHeight < c.params.CRCommitteeStartHeight {
		return inVotingPeriod(c.params.CRCommitteeStartHeight)
	} else {
		return inVotingPeriod(c.LastCommitteeHeight + c.params.CRDutyPeriod)
	}
}

func (c *Committee) changeCommitteeMembers(height uint32) error {
	candidates, err := c.getActiveCRCandidatesDesc()
	if err != nil {
		return err
	}

	c.Members = make([]*CRMember, 0, c.params.CRMemberCount)
	for i := 0; i < int(c.params.CRMemberCount); i++ {
		c.Members = append(c.Members, c.generateMember(candidates[i]))
	}

	c.LastCommitteeHeight = height
	return nil
}

func (c *Committee) generateMember(candidate *Candidate) *CRMember {
	return &CRMember{
		Info:             candidate.info,
		ImpeachmentVotes: 0,
	}
}

func (c *Committee) getActiveCRCandidatesDesc() ([]*Candidate, error) {
	candidates := c.state.GetCandidates(Active)
	if uint32(len(candidates)) < c.params.CRMemberCount {
		return nil, errors.New("candidates count less than required count")
	}

	sort.Slice(candidates, func(i, j int) bool {
		return candidates[i].votes > candidates[j].votes
	})
	return candidates, nil
}

func NewCommittee(params *config.Params) *Committee {
	return &Committee{
		state:    NewState(params),
		params:   params,
		KeyFrame: *NewKeyFrame(),
		store:    nil, // todo complete me
	}
}
