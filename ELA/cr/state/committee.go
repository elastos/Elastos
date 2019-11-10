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

	"github.com/elastos/Elastos.ELA/account"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/utils"
)

type Committee struct {
	KeyFrame
	mtx     sync.RWMutex
	state   *State
	params  *config.Params
	manager *ProposalManager

	getCheckpoint             func(height uint32) *Checkpoint
	getUnspentFromProgramHash func(programHash common.Uint168,
		assetid common.Uint256) ([]*types.UTXO, error)
	getHeight func() uint32

	recordBalanceHeight uint32
}

func (c *Committee) GetState() *State {
	return c.state
}

func (c *Committee) GetProposalManager() *ProposalManager {
	return c.manager
}

func (c *Committee) ExistCR(programCode []byte) bool {
	existCandidate := c.state.ExistCandidate(programCode)
	if existCandidate {
		return true
	}

	did, err := getDIDByCode(programCode)
	if err != nil {
		return false
	}

	return c.IsCRMemberByDID(*did)
}

func (c *Committee) IsCRMember(programCode []byte) bool {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	for _, v := range c.Members {
		if bytes.Equal(programCode, v.Info.Code) {
			return true
		}
	}
	return false
}

func (c *Committee) IsCRMemberByDID(did common.Uint168) bool {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	for _, v := range c.Members {
		if v.Info.DID.IsEqual(did) {
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

func (c *Committee) IsInElectionPeriod() bool {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.InElectionPeriod
}

func (c *Committee) IsAppropriationNeeded() bool {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.NeedAppropriation
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

// get all CRMembers ordered by DID
func (c *Committee) GetAllMembers() []*CRMember {
	c.mtx.RLock()
	defer c.mtx.RUnlock()

	result := getCRMembers(c.Members)
	sort.Slice(result, func(i, j int) bool {
		return result[i].Info.DID.Compare(result[j].Info.DID) <= 0
	})
	return result
}

//get all elected CRMembers
func (c *Committee) GetElectedMembers() []*CRMember {
	c.mtx.RLock()
	defer c.mtx.RUnlock()

	return getElectedCRMembers(c.Members)
}

//get all history CRMembers
func (c *Committee) GetAllHistoryMembers() []*CRMember {
	c.mtx.RLock()
	defer c.mtx.RUnlock()

	return getCRMembers(c.HistoryMembers)
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

func (c *Committee) GetMember(did common.Uint168) *CRMember {
	c.mtx.RLock()
	defer c.mtx.RUnlock()

	return c.getMember(did)
}

func (c *Committee) getMember(did common.Uint168) *CRMember {
	for _, m := range c.Members {
		if m.Info.DID.IsEqual(did) {
			return m
		}
	}
	return nil
}

func (c *Committee) ProcessBlock(block *types.Block, confirm *payload.Confirm) {
	c.mtx.Lock()
	defer c.mtx.Unlock()

	if block.Height < c.params.CRVotingStartHeight {
		return
	}

	// Get CRC foundation and committee balance at CRVotingStartHeight.
	c.tryInitCRCRelatedAddressBalance(block.Height)

	// If reached the voting start height, record the last voting start height.
	c.recordLastVotingStartHeight(block.Height)

	// If in election period and not in voting period, deal with TransferAsset
	// ReturnCRDepositCoin CRCProposal type of transaction only.
	isVoting := c.isInVotingPeriod(block.Height)
	if isVoting {
		c.state.ProcessBlock(block, confirm)
	} else {
		c.state.ProcessElectionBlock(block)
	}

	if c.shouldChange(block.Height) {
		if c.shouldCleanHistory() {
			c.HistoryMembers = make(map[common.Uint168]*CRMember)
		}
		committeeDIDs, err := c.changeCommitteeMembers(block.Height)
		if err != nil {
			log.Error("[ProcessBlock] change committee members error: ", err)
			return
		}

		checkpoint := Checkpoint{
			KeyFrame: c.KeyFrame,
		}
		checkpoint.StateKeyFrame = *c.state.FinishVoting(committeeDIDs)

		c.createCRCAppropriationTransaction()
		c.NeedAppropriation = true
		// todo calculate used amount by current proposal.
		c.CRCCommitteeUsedAmount = 0
	}
}

func (c *Committee) createCRCAppropriationTransaction() error {
	// todo create CR appropriation transaction and send to txpool.
	return nil
}

func (c *Committee) tryInitCRCRelatedAddressBalance(height uint32) {
	if c.recordBalanceHeight == 0 {
		utxos, _ := c.getUnspentFromProgramHash(c.params.CRCFoundation,
			*account.SystemAssetID)
		for _, u := range utxos {
			c.CRCFoundationBalance += u.Value
			op := types.NewOutPoint(u.TxID, uint16(u.Index))
			c.state.CRCFoundationOutputs[op.ReferKey()] = u.Value
		}
		utxos, _ = c.getUnspentFromProgramHash(c.params.CRCCommitteeAddress,
			*account.SystemAssetID)
		for _, u := range utxos {
			c.CRCCommitteeBalance += u.Value
			op := types.NewOutPoint(u.TxID, uint16(u.Index))
			c.state.CRCCommitteeOutputs[op.ReferKey()] = u.Value

		}
		c.recordBalanceHeight = height
		log.Infof("record balance at height of %d, balance of CRC "+
			"foundation is %s, balance of CRC committee address is %s",
			height, c.CRCFoundationBalance, c.CRCCommitteeBalance)

	}
}

func (c *Committee) recordLastVotingStartHeight(height uint32) {
	// Update last voting start height one block ahead.
	if height == c.LastCommitteeHeight+c.params.CRDutyPeriod-
		c.params.CRVotingPeriod-1 {
		lastVotingStartHeight := c.LastVotingStartHeight
		c.state.history.Append(height, func() {
			c.LastVotingStartHeight = height + 1
		}, func() {
			c.LastVotingStartHeight = lastVotingStartHeight
		})
	}
}

func (c *Committee) tryStartVotingPeriod(height uint32) {
	if !c.InElectionPeriod {
		return
	}

	lastVotingStartHeight := c.LastVotingStartHeight
	c.state.history.Append(height, func() {
		var normalCount uint32
		for _, m := range c.Members {
			if m.MemberState != MemberElected {
				normalCount++
			}
		}
		if normalCount < c.params.CRAgreementCount {
			c.InElectionPeriod = false
			c.LastVotingStartHeight = height
		}
	}, func() {
		var normalCount uint32
		for _, m := range c.Members {
			if m.MemberState != MemberElected {
				normalCount++
			}
		}
		if normalCount >= c.params.CRAgreementCount {
			c.InElectionPeriod = true
			c.LastVotingStartHeight = lastVotingStartHeight
		}
	})
}

func (c *Committee) processImpeachment(height uint32, member []byte,
	votes common.Fixed64, history *utils.History) {
	// todo get current circulation by calculation
	circulation := common.Fixed64(3300 * 10000 * 100000000)
	for _, v := range c.Members {
		if bytes.Equal(v.Info.Code, member) {
			penalty := v.Penalty
			history.Append(height, func() {
				v.ImpeachmentVotes += votes
				if v.ImpeachmentVotes >= common.Fixed64(float64(circulation)*
					c.params.VoterRejectPercentage/100.0) {
					v.MemberState = MemberImpeached
					v.Penalty = c.getMemberPenalty(height, v)
				}
			}, func() {
				v.ImpeachmentVotes -= votes
				if v.ImpeachmentVotes < common.Fixed64(float64(circulation)*
					c.params.VoterRejectPercentage/100.0) {
					v.MemberState = MemberElected
					v.Penalty = penalty
				}
			})
			return
		}
	}
}

func (c *Committee) processCRCAppropriation(tx *types.Transaction, height uint32,
	history *utils.History) {
	if height <= c.recordBalanceHeight {
		return
	}
	history.Append(height, func() {
		c.NeedAppropriation = false
		c.CRCCommitteeUsedAmount += tx.Outputs[0].Value
	}, func() {
		c.NeedAppropriation = true
		c.CRCCommitteeUsedAmount -= tx.Outputs[0].Value
	})
}

func (c *Committee) processCRCRelatedAmount(tx *types.Transaction, height uint32,
	history *utils.History, foundationInputsAmounts map[string]common.Fixed64,
	committeeInputsAmounts map[string]common.Fixed64) {
	for _, input := range tx.Inputs {
		if amount, ok := foundationInputsAmounts[input.Previous.ReferKey()]; ok {
			history.Append(height, func() {
				c.CRCFoundationBalance -= amount
			}, func() {
				c.CRCFoundationBalance += amount
			})
		} else if amount, ok := committeeInputsAmounts[input.Previous.ReferKey()]; ok {
			history.Append(height, func() {
				c.CRCCommitteeBalance -= amount
			}, func() {
				c.CRCCommitteeBalance += amount
			})
		}
	}

	for _, output := range tx.Outputs {
		if output.ProgramHash.IsEqual(c.params.CRCFoundation) {
			history.Append(height, func() {
				c.CRCFoundationBalance += output.Value
			}, func() {
				c.CRCFoundationBalance -= output.Value
			})
		} else if output.ProgramHash.IsEqual(c.params.CRCCommitteeAddress) {
			history.Append(height, func() {
				c.CRCCommitteeBalance += output.Value
			}, func() {
				c.CRCCommitteeBalance -= output.Value
			})
		}
	}
}

func (c *Committee) GetHistoryMember(code []byte) *CRMember {
	c.mtx.RLock()
	defer c.mtx.RUnlock()

	return c.getHistoryMember(code)
}

func (c *Committee) getHistoryMember(code []byte) *CRMember {
	for _, m := range c.HistoryMembers {
		if bytes.Equal(m.Info.Code, code) {
			return m
		}
	}
	return nil
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

func (c *Committee) shouldChange(height uint32) bool {
	if c.LastCommitteeHeight == 0 {
		if height < c.params.CRCommitteeStartHeight {
			return false
		} else if height == c.params.CRCommitteeStartHeight {
			return true
		}
	}

	return height == c.LastVotingStartHeight+c.params.CRVotingPeriod
}

func (c *Committee) shouldCleanHistory() bool {
	return c.LastVotingStartHeight == c.LastCommitteeHeight+
		c.params.CRDutyPeriod-c.params.CRVotingPeriod
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
		if !c.InElectionPeriod {
			return height < c.LastVotingStartHeight+c.params.CRVotingPeriod
		}
		return inVotingPeriod(c.LastCommitteeHeight + c.params.CRDutyPeriod)
	}
}

func (c *Committee) changeCommitteeMembers(height uint32) (
	[]common.Uint168, error) {
	candidates, err := c.getActiveCRCandidatesDesc()
	if err != nil {
		c.InElectionPeriod = false
		c.LastVotingStartHeight = height
		return nil, err
	}

	// Record history members.
	for _, m := range c.Members {
		m.Penalty = c.getMemberPenalty(height, m)
		c.HistoryMembers[m.Info.DID] = m
	}

	result := make([]common.Uint168, 0, c.params.CRMemberCount)
	c.Members = make(map[common.Uint168]*CRMember, c.params.CRMemberCount)
	c.manager.Proposals = make(map[common.Uint256]*ProposalState)
	c.manager.ProposalHashs = make(map[common.Uint168]ProposalHashSet)
	for i := 0; i < int(c.params.CRMemberCount); i++ {
		c.Members[candidates[i].info.DID] = c.generateMember(candidates[i])
		result = append(result, candidates[i].info.DID)
	}

	c.InElectionPeriod = true
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

func (c *Committee) getMemberPenalty(height uint32, member *CRMember) common.Fixed64 {
	// Calculate penalty by election block count.
	electionCount := height - c.LastCommitteeHeight
	electionRate := float64(electionCount) / float64(c.params.CRDutyPeriod)
	notElectionPenalty := MinDepositAmount * common.Fixed64(1-electionRate)

	// Calculate penalty by vote proposal count.
	var voteCount int
	for _, v := range c.manager.Proposals {
		for did, _ := range v.CRVotes {
			if member.Info.DID.IsEqual(did) {
				voteCount++
				break
			}
		}
	}
	proposalsCount := len(c.manager.Proposals)
	voteRate := float64(voteCount) / float64(proposalsCount)
	notVoteProposalPenalty := MinDepositAmount * common.Fixed64(1-voteRate)

	// Calculate the final penalty.
	finalPenalty := member.Penalty + notElectionPenalty + notVoteProposalPenalty

	log.Infof("member %s impeached, not election penalty: %s,"+
		"not vote proposal penalty: %s, old penalty: %s, final penalty: %s",
		member.Info.DID, notElectionPenalty, notVoteProposalPenalty,
		member.Penalty, finalPenalty)

	return finalPenalty
}

func (c *Committee) generateCandidate(height uint32, member *CRMember) *Candidate {
	return &Candidate{
		info:          member.Info,
		state:         Canceled,
		cancelHeight:  height,
		depositAmount: member.DepositAmount,
		depositHash:   member.DepositHash,
		penalty:       member.Penalty,
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

type CommitteeFuncsConfig struct {
	GetTxReference func(tx *types.Transaction) (
		map[*types.Input]*types.Output, error)
	GetUnspentFromProgramHash func(programHash common.Uint168,
		assetid common.Uint256) ([]*types.UTXO, error)
	GetHeight func() uint32
}

func (c *Committee) RegisterFuncitons(cfg *CommitteeFuncsConfig) {
	c.state.RegisterFunctions(&FunctionsConfig{
		TryStartVotingPeriod:    c.tryStartVotingPeriod,
		ProcessImpeachment:      c.processImpeachment,
		ProcessCRCAppropriation: c.processCRCAppropriation,
		ProcessCRCRelatedAmount: c.processCRCRelatedAmount,
		GetHistoryMember:        c.getHistoryMember,
		GetTxReference:          cfg.GetTxReference,
	})
	c.getUnspentFromProgramHash = cfg.GetUnspentFromProgramHash
	c.getHeight = cfg.GetHeight
}

func NewCommittee(params *config.Params) *Committee {
	committee := &Committee{
		state:    NewState(params),
		params:   params,
		KeyFrame: *NewKeyFrame(),
		manager:  NewProposalManager(params),
	}
	committee.state.SetManager(committee.manager)
	params.CkpManager.Register(NewCheckpoint(committee))
	return committee
}
