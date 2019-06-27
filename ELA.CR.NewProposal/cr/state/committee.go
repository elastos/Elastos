package state

import (
	"errors"
	"sort"
	"sync"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
)

type CRMember struct {
	Info             payload.CRInfo
	ImpeachmentVotes common.Fixed64
}

type Committee struct {
	mtx     sync.RWMutex
	state   *State
	members []*CRMember
	params  *config.Params

	lastCommitteeHeight uint32
}

func (c *Committee) GetMembersDIDs() []common.Uint168 {
	c.mtx.RLock()
	defer c.mtx.RUnlock()

	result := make([]common.Uint168, 0, len(c.members))
	for _, v := range c.members {
		result = append(result, v.Info.DID)
	}
	return result
}

func (c *Committee) GetMembersCodes() [][]byte {
	c.mtx.RLock()
	defer c.mtx.RUnlock()

	result := make([][]byte, 0, len(c.members))
	for _, v := range c.members {
		result = append(result, v.Info.Code)
	}
	return result
}

func (c *Committee) ProcessBlock(block *types.Block,
	confirm *payload.Confirm) {
	c.state.ProcessBlock(block, confirm)

	c.mtx.Lock()
	defer c.mtx.Unlock()
	if c.shouldChange(block) {
		if err := c.changeCommitteeMembers(block.Height); err != nil {
			log.Error("[ProcessBlock] change committee members error: ", err)
		}
	}
}

func (c *Committee) shouldChange(block *types.Block) bool {
	//todo judge by change cr committee tx later
	return block.Height >= c.params.CRCommitteeStartHeight &&
		block.Height >= c.lastCommitteeHeight+c.params.CRDutyPeriod
}

func (c *Committee) changeCommitteeMembers(height uint32) error {
	candidates, err := c.getActiveCRCandidatesDesc()
	if err != nil {
		return err
	}

	c.members = make([]*CRMember, 0, c.params.CRMemberCount)
	for i := 0; i < int(c.params.CRMemberCount); i++ {
		c.members = append(c.members, c.generateMember(candidates[i]))
	}

	c.lastCommitteeHeight = height
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
		state:               NewState(params),
		members:             make([]*CRMember, 0),
		params:              params,
		lastCommitteeHeight: 0,
	}
}
