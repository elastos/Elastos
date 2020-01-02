// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package state

import (
	"bytes"
	"errors"
	"sort"
	"sync"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	elaerr "github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/p2p"
	"github.com/elastos/Elastos.ELA/p2p/msg"
	"github.com/elastos/Elastos.ELA/utils"
)

type Committee struct {
	KeyFrame
	mtx          sync.RWMutex
	state        *State
	params       *config.Params
	manager      *ProposalManager
	firstHistory *utils.History
	lastHistory  *utils.History

	getCheckpoint            func(height uint32) *Checkpoint
	getHeight                func() uint32
	isCurrent                func() bool
	broadcast                func(msg p2p.Message)
	appendToTxpool           func(transaction *types.Transaction) elaerr.ELAError
	createCRCAppropriationTx func() (*types.Transaction, error)
	getUTXO                  func(programHash *common.Uint168) ([]*types.UTXO, error)

	recordBalanceHeight uint32
}

type CommitteeKeyFrame struct {
	*KeyFrame
	*StateKeyFrame
	*ProposalKeyFrame
}

// Deprecated: just for testing
func (c *Committee) GetState() *State {
	return c.state
}

// Deprecated: just for testing
func (c *Committee) GetProposalManager() *ProposalManager {
	return c.manager
}

func (c *Committee) ExistCR(programCode []byte) bool {
	existCandidate := c.state.existCandidate(programCode)
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

//get all impeachable CRMembers
func (c *Committee) GetImpeachableMembers() []*CRMember {
	c.mtx.RLock()
	defer c.mtx.RUnlock()

	return getImpeachableCRMembers(c.Members)
}

//get all history CRMembers
func (c *Committee) GetAllHistoryMembers() []*CRMember {
	c.mtx.RLock()
	defer c.mtx.RUnlock()

	return getHistoryMembers(c.HistoryMembers)
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
		c.ProcessBlockInVotingPeriod(block)
	} else {
		c.processBlockInElectionPeriod(block)
	}
	c.manager.updateProposals(block.Height, c.CirculationAmount)
	c.tryStartVotingPeriod(block.Height)
	c.freshCirculationAmount(c.lastHistory, block.Height, block.Height)

	if c.shouldChange(block.Height) {
		c.changeCommittee(block.Height)
		c.createAppropriationTransaction(block.Height)
	}

	c.lastHistory.Commit(block.Height)
}

func (c *Committee) changeCommittee(height uint32) {
	if c.shouldCleanHistory() {
		c.HistoryMembers = make(map[uint64]map[common.Uint168]*CRMember)
		c.state.HistoryCandidates = make(map[uint64]map[common.Uint168]*Candidate)
	}
	err := c.changeCommitteeMembers(height)
	if err != nil {
		log.Warn("[ProcessBlock] change committee members error: ", err)
		return
	}

	c.resetCRCCommitteeUsedAmount(height)
}

func (c *Committee) createAppropriationTransaction(height uint32) {
	if c.createCRCAppropriationTx != nil && height == c.getHeight() {
		tx, err := c.createCRCAppropriationTx()
		if err != nil {
			log.Error("create appropriation tx failed")
			return
		} else if tx == nil {
			log.Info("no need to create appropriation")
			oriNeedAppropriation := c.NeedAppropriation
			c.lastHistory.Append(height, func() {
				c.NeedAppropriation = false
			}, func() {
				c.NeedAppropriation = oriNeedAppropriation
			})
			return
		}
		log.Info("create CRCAppropriation transaction:", tx.Hash())
		if c.isCurrent != nil && c.broadcast != nil && c.
			appendToTxpool != nil {
			go func() {
				if c.isCurrent() {
					if err := c.appendToTxpool(tx); err == nil {
						c.broadcast(msg.NewTx(tx))
					}
				}
			}()
		}
	}
}

func (c *Committee) resetCRCCommitteeUsedAmount(height uint32) {
	// todo add finished proposals into finished map
	var budget common.Fixed64
	for _, v := range c.manager.Proposals {
		if v.Status == Finished || v.Status == CRCanceled ||
			v.Status == VoterCanceled || v.Status == Aborted {
			continue
		}
		for _, b := range v.Proposal.Budgets {
			if _, ok := v.WithdrawnBudgets[b.Stage]; !ok {
				budget += b.Amount
			}
		}
	}

	oriNeedAppropriation := c.NeedAppropriation
	oriUsedAmount := c.CRCCommitteeUsedAmount
	c.lastHistory.Append(height, func() {
		c.NeedAppropriation = true
		c.CRCCommitteeUsedAmount = budget
	}, func() {
		c.NeedAppropriation = oriNeedAppropriation
		c.CRCCommitteeUsedAmount = oriUsedAmount
	})

}

func (c *Committee) tryInitCRCRelatedAddressBalance(height uint32) {
	if c.recordBalanceHeight == 0 {
		bestHeight := c.getHeight()
		utxos, _ := c.getUTXO(&c.params.CRCFoundation)
		var foundationBalance common.Fixed64
		for _, u := range utxos {
			foundationBalance += u.Value
			op := types.NewOutPoint(u.TxID, uint16(u.Index))
			c.firstHistory.Append(height, func() {
				c.state.CRCFoundationOutputs[op.ReferKey()] = u.Value
			}, func() {
				delete(c.state.CRCFoundationOutputs, op.ReferKey())
			})
		}
		var committeeBalance common.Fixed64
		utxos, _ = c.getUTXO(&c.params.CRCCommitteeAddress)
		for _, u := range utxos {
			committeeBalance += u.Value
			op := types.NewOutPoint(u.TxID, uint16(u.Index))
			c.firstHistory.Append(height, func() {
				c.state.CRCCommitteeOutputs[op.ReferKey()] = u.Value
			}, func() {
				delete(c.state.CRCCommitteeOutputs, op.ReferKey())
			})
		}
		utxos, _ = c.getUTXO(&c.params.DestroyELAAddress)
		var destroyBalance common.Fixed64
		for _, u := range utxos {
			destroyBalance += u.Value
		}
		c.firstHistory.Append(height, func() {
			c.CRCFoundationBalance = foundationBalance
			c.CRCCommitteeBalance = committeeBalance
			c.DestroyedAmount = destroyBalance
		}, func() {
			c.CRCFoundationBalance = 0
			c.CRCCommitteeBalance = 0
			c.DestroyedAmount = 0
		})

		c.freshCirculationAmount(c.firstHistory, bestHeight, height)
		c.firstHistory.Append(height, func() {
			c.recordBalanceHeight = bestHeight
		}, func() {
			c.recordBalanceHeight = 0
		})
		c.firstHistory.Commit(height)
		log.Infof("record balance at height of %d, balance of CRC "+
			"foundation is %s, balance of CRC committee address is %s",
			bestHeight, c.CRCFoundationBalance, c.CRCCommitteeBalance)
	}
}

func (c *Committee) freshCirculationAmount(history *utils.History,
	bestHeight uint32, height uint32) {
	circulationAmount := common.Fixed64(config.OriginIssuanceAmount) +
		common.Fixed64(bestHeight)*c.params.RewardPerBlock -
		c.CRCFoundationBalance - c.CRCCommitteeBalance - c.DestroyedAmount
	oriCirculationAmount := c.CirculationAmount
	history.Append(height, func() {
		c.CirculationAmount = circulationAmount
	}, func() {
		c.CirculationAmount = oriCirculationAmount
	})
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
	inElectionPeriod := c.InElectionPeriod
	c.lastHistory.Append(height, func() {
		var normalCount uint32
		for _, m := range c.Members {
			if m.MemberState == MemberElected {
				normalCount++
			}
		}
		if normalCount < c.params.CRAgreementCount {
			c.InElectionPeriod = false
			if !c.isInVotingPeriod(height) {
				c.LastVotingStartHeight = height
			}
		}
	}, func() {
		c.InElectionPeriod = inElectionPeriod
		c.LastVotingStartHeight = lastVotingStartHeight
	})
}

func (c *Committee) processImpeachment(height uint32, member []byte,
	votes common.Fixed64, history *utils.History) {
	circulation := c.CirculationAmount

	var crMember *CRMember
	for _, v := range c.Members {
		if bytes.Equal(v.Info.DID.Bytes(), member) {
			crMember = v
			break
		}
	}
	oriPenalty := c.state.depositInfo[crMember.Info.DID].Penalty
	oriMemberState := crMember.MemberState
	penalty := c.getMemberPenalty(height, crMember)
	history.Append(height, func() {
		crMember.ImpeachmentVotes += votes
		if crMember.ImpeachmentVotes >= common.Fixed64(float64(circulation)*
			c.params.VoterRejectPercentage/100.0) {
			crMember.MemberState = MemberImpeached
			c.state.depositInfo[crMember.Info.DID].Penalty = penalty
		}
	}, func() {
		crMember.ImpeachmentVotes -= votes
		crMember.MemberState = oriMemberState
		c.state.depositInfo[crMember.Info.DID].Penalty = oriPenalty
	})
	return
}

func (c *Committee) processCRCAppropriation(tx *types.Transaction, height uint32,
	history *utils.History) {
	history.Append(height, func() {
		c.NeedAppropriation = false
	}, func() {
		c.NeedAppropriation = true
	})
}

func (c *Committee) processCRCRelatedAmount(tx *types.Transaction, height uint32,
	history *utils.History, foundationInputsAmounts map[string]common.Fixed64,
	committeeInputsAmounts map[string]common.Fixed64) {
	if tx.IsCRCProposalTx() {
		proposal := tx.Payload.(*payload.CRCProposal)
		var budget common.Fixed64
		for _, b := range proposal.Budgets {
			budget += b.Amount
		}
		history.Append(height, func() {
			c.CRCCommitteeUsedAmount += budget
		}, func() {
			c.CRCCommitteeUsedAmount -= budget
		})
	}

	if height <= c.recordBalanceHeight {
		return
	}
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
		amount := output.Value
		if output.ProgramHash.IsEqual(c.params.CRCFoundation) {
			history.Append(height, func() {
				c.CRCFoundationBalance += amount
			}, func() {
				c.CRCFoundationBalance -= amount
			})
		} else if output.ProgramHash.IsEqual(c.params.CRCCommitteeAddress) {
			history.Append(height, func() {
				c.CRCCommitteeBalance += amount
			}, func() {
				c.CRCCommitteeBalance -= amount
			})
		} else if output.ProgramHash.IsEqual(c.params.DestroyELAAddress) {
			history.Append(height, func() {
				c.DestroyedAmount += amount
			}, func() {
				c.DestroyedAmount -= amount
			})
		}
	}
}

func (c *Committee) GetAvailableDepositAmount(did common.Uint168) common.Fixed64 {
	c.mtx.RLock()
	defer c.mtx.RUnlock()

	currentHeight := c.getHeight()
	inVoting := c.isInVotingPeriod(currentHeight)
	return c.state.getAvailableDepositAmount(did, currentHeight, inVoting)
}

func (c *Committee) GetHistoryMember(code []byte) *CRMember {
	c.mtx.RLock()
	defer c.mtx.RUnlock()

	return c.getHistoryMember(code)
}

func (c *Committee) getHistoryMember(code []byte) *CRMember {
	for _, v := range c.HistoryMembers {
		for _, m := range v {
			if bytes.Equal(m.Info.Code, code) {
				return m
			}
		}
	}
	return nil
}

func (c *Committee) RollbackTo(height uint32) error {
	c.mtx.Lock()
	defer c.mtx.Unlock()
	if err := c.lastHistory.RollbackTo(height); err != nil {
		log.Debug("committee last history rollback err:", err)
	}
	if err := c.manager.history.RollbackTo(height); err != nil {
		log.Debug("manager rollback err:", err)
	}
	if err := c.state.rollbackTo(height); err != nil {
		log.Debug("state rollback err:", err)
	}
	if err := c.firstHistory.RollbackTo(height); err != nil {
		log.Debug("committee first history rollback err:", err)
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

	if c.LastCommitteeHeight < c.params.CRCommitteeStartHeight &&
		height <= c.params.CRCommitteeStartHeight {
		return height >= c.params.CRVotingStartHeight &&
			height < c.params.CRCommitteeStartHeight
	} else {
		if !c.InElectionPeriod {
			if c.LastVotingStartHeight == 0 {
				return true
			}
			return height < c.LastVotingStartHeight+c.params.CRVotingPeriod
		}
		return inVotingPeriod(c.LastCommitteeHeight + c.params.CRDutyPeriod)
	}
}

func (c *Committee) changeCommitteeMembers(height uint32) error {

	// Process current members.
	candidates := c.getActiveCRCandidatesDesc()
	c.processCurrentMembers(height, candidates)

	oriInElectionPeriod := c.InElectionPeriod
	oriLastVotingStartHeight := c.LastVotingStartHeight
	if uint32(len(candidates)) < c.params.CRMemberCount {
		c.lastHistory.Append(height, func() {
			c.InElectionPeriod = false
			c.LastVotingStartHeight = height
		}, func() {
			c.InElectionPeriod = oriInElectionPeriod
			c.LastVotingStartHeight = oriLastVotingStartHeight
		})
		return errors.New("candidates count less than required count")
	}

	// Process current candidates.
	c.processCurrentCandidates(height, candidates)

	oriLastCommitteeHeight := c.LastCommitteeHeight
	c.lastHistory.Append(height, func() {
		c.state.CurrentSession += 1
		c.InElectionPeriod = true
		c.LastCommitteeHeight = height
	}, func() {
		c.state.CurrentSession -= 1
		c.InElectionPeriod = oriInElectionPeriod
		c.LastCommitteeHeight = oriLastCommitteeHeight
	})

	return nil
}

func (c *Committee) processCurrentMembers(height uint32,
	activeCandidates []*Candidate) {
	if uint32(len(activeCandidates)) < c.params.CRMemberCount {
		return
	}

	if _, ok := c.HistoryMembers[c.state.CurrentSession]; !ok {
		currentSession := c.state.CurrentSession
		c.lastHistory.Append(height, func() {
			c.HistoryMembers[currentSession] =
				make(map[common.Uint168]*CRMember)
		}, func() {
			delete(c.HistoryMembers, currentSession)
		})
	}

	oriMembers := copyMembersMap(c.Members)
	for _, m := range oriMembers {
		member := *m
		oriPenalty := c.state.depositInfo[m.Info.DID].Penalty
		oriRefundable := c.state.depositInfo[m.Info.DID].Refundable
		oriDepositAmount := c.state.depositInfo[m.Info.DID].DepositAmount
		c.lastHistory.Append(height, func() {
			c.state.depositInfo[m.Info.DID].Penalty = c.getMemberPenalty(height, m)
			c.state.depositInfo[m.Info.DID].Refundable = true
			c.state.depositInfo[m.Info.DID].DepositAmount -= MinDepositAmount
			c.HistoryMembers[c.state.CurrentSession][m.Info.DID] = &member
		}, func() {
			c.state.depositInfo[m.Info.DID].Penalty = oriPenalty
			c.state.depositInfo[m.Info.DID].Refundable = oriRefundable
			c.state.depositInfo[m.Info.DID].DepositAmount -= oriDepositAmount
			delete(c.HistoryMembers[c.state.CurrentSession], m.Info.DID)
		})
	}

	newMembers := make(map[common.Uint168]*CRMember, c.params.CRMemberCount)
	for i := 0; i < int(c.params.CRMemberCount); i++ {
		newMembers[activeCandidates[i].info.DID] =
			c.generateMember(activeCandidates[i])
	}

	oriNicknames := utils.CopyStringSet(c.state.Nicknames)
	oriVotes := utils.CopyStringSet(c.state.Votes)
	c.lastHistory.Append(height, func() {
		c.Members = newMembers
		c.state.Nicknames = map[string]struct{}{}
		c.state.Votes = map[string]struct{}{}
	}, func() {
		c.Members = oriMembers
		c.state.Nicknames = oriNicknames
		c.state.Votes = oriVotes
	})
}

func (c *Committee) processCurrentCandidates(height uint32,
	activeCandidates []*Candidate) {
	newHistoryCandidates := make(map[common.Uint168]*Candidate)
	if _, ok := c.state.HistoryCandidates[c.state.CurrentSession]; !ok {
		c.state.HistoryCandidates[c.state.CurrentSession] =
			make(map[common.Uint168]*Candidate)
	}
	membersMap := make(map[common.Uint168]struct{})
	for _, c := range activeCandidates {
		membersMap[c.info.DID] = struct{}{}
	}
	for k, v := range c.state.Candidates {
		if _, ok := membersMap[k]; !ok {
			newHistoryCandidates[k] = v
		}
	}

	oriCandidate := copyCandidateMap(c.state.Candidates)
	currentSession := c.state.CurrentSession
	c.lastHistory.Append(height, func() {
		c.state.Candidates = make(map[common.Uint168]*Candidate)
		c.state.HistoryCandidates[currentSession] = newHistoryCandidates
	}, func() {
		c.state.Candidates = oriCandidate
		delete(c.state.HistoryCandidates, currentSession)
	})
}

func (c *Committee) generateMember(candidate *Candidate) *CRMember {
	return &CRMember{
		Info:             candidate.info,
		ImpeachmentVotes: 0,
		DepositHash:      candidate.depositHash,
	}
}

func (c *Committee) getMemberPenalty(height uint32, member *CRMember) common.Fixed64 {
	// Calculate penalty by election block count.
	electionCount := height - c.LastCommitteeHeight
	electionRate := float64(electionCount) / float64(c.params.CRDutyPeriod)

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
	voteRate := float64(1.0)
	if proposalsCount != 0 {
		voteRate = float64(voteCount) / float64(proposalsCount)
	}

	// Calculate the final penalty.
	penalty := c.state.depositInfo[member.Info.DID].Penalty
	currentPenalty := MinDepositAmount * common.Fixed64(1-electionRate*voteRate)
	finalPenalty := penalty + currentPenalty

	log.Infof("member %s impeached, not election and not vote proposal"+
		" penalty: %s, old penalty: %s, final penalty: %s",
		member.Info.DID, currentPenalty, penalty, finalPenalty)

	return finalPenalty
}

func (c *Committee) generateCandidate(height uint32, member *CRMember) *Candidate {
	return &Candidate{
		info:         member.Info,
		state:        Canceled,
		cancelHeight: height,
		depositHash:  member.DepositHash,
	}
}

func (c *Committee) getActiveCRCandidatesDesc() []*Candidate {
	candidates := c.state.getCandidates(Active)

	sort.Slice(candidates, func(i, j int) bool {
		if candidates[i].votes == candidates[j].votes {
			iCRInfo := candidates[i].Info()
			jCRInfo := candidates[j].Info()
			return iCRInfo.GetCodeHash().Compare(jCRInfo.GetCodeHash()) < 0
		}
		return candidates[i].votes > candidates[j].votes
	})
	return candidates
}

func (c *Committee) GetCandidate(did common.Uint168) *Candidate {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.state.getCandidate(did)
}

func (c *Committee) GetCandidates(state CandidateState) []*Candidate {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.state.getCandidates(state)
}

func (c *Committee) ExistCandidateByNickname(nickname string) bool {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.state.existCandidateByNickname(nickname)
}

func (c *Committee) ExistCandidateByDepositHash(did common.Uint168) bool {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.state.existCandidateByDepositHash(did)
}

func (c *Committee) GetPenalty(did common.Uint168) common.Fixed64 {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.state.getPenalty(did)
}

func (c *Committee) ExistProposal(hash common.Uint256) bool {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.manager.existProposal(hash)
}

func (c *Committee) GetProposal(hash common.Uint256) *ProposalState {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.manager.getProposal(hash)
}

func (c *Committee) AvailableWithdrawalAmount(hash common.Uint256) common.Fixed64 {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.manager.availableWithdrawalAmount(hash)
}

func (c *Committee) IsProposalFull(did common.Uint168) bool {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.manager.isProposalFull(did)
}

func (c *Committee) ExistDraft(hash common.Uint256) bool {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.manager.existDraft(hash)
}

func (c *Committee) Exist(did common.Uint168) bool {
	c.mtx.RLock()
	defer c.mtx.RUnlock()

	return c.state.exist(did)
}

func (c *Committee) IsRefundable(did common.Uint168) bool {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.state.isRefundable(did)
}

func (c *Committee) GetAllCandidates() []*Candidate {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.state.getAllCandidates()
}

func (c *Committee) GetAllProposals() ProposalsMap {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.manager.getAllProposals()
}

func (c *Committee) GetProposals(status ProposalStatus) ProposalsMap {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.manager.getProposals(status)
}

func (c *Committee) GetProposalByDraftHash(draftHash common.Uint256) *ProposalState {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.manager.getProposalByDraftHash(draftHash)
}

type CommitteeFuncsConfig struct {
	GetTxReference func(tx *types.Transaction) (
		map[*types.Input]*types.Output, error)
	GetHeight                        func() uint32
	CreateCRAppropriationTransaction func() (*types.Transaction, error)
	IsCurrent                        func() bool
	Broadcast                        func(msg p2p.Message)
	AppendToTxpool                   func(transaction *types.Transaction) elaerr.ELAError
	GetUTXO                          func(programHash *common.Uint168) ([]*types.UTXO, error)
}

func (c *Committee) RegisterFuncitons(cfg *CommitteeFuncsConfig) {
	c.createCRCAppropriationTx = cfg.CreateCRAppropriationTransaction
	c.isCurrent = cfg.IsCurrent
	c.broadcast = cfg.Broadcast
	c.appendToTxpool = cfg.AppendToTxpool
	c.state.registerFunctions(&FunctionsConfig{
		GetHistoryMember: c.getHistoryMember,
		GetTxReference:   cfg.GetTxReference,
	})
	c.getUTXO = cfg.GetUTXO
	c.getHeight = cfg.GetHeight
}

func (c *Committee) SnapShort() *CommitteeKeyFrame {
	keyFrame := &CommitteeKeyFrame{
		KeyFrame:         c.KeyFrame.Snapshot(),
		StateKeyFrame:    c.state.StateKeyFrame.Snapshot(),
		ProposalKeyFrame: c.manager.ProposalKeyFrame.Snapshot(),
	}

	return keyFrame
}

func NewCommittee(params *config.Params) *Committee {
	committee := &Committee{
		state:        NewState(params),
		params:       params,
		KeyFrame:     *NewKeyFrame(),
		manager:      NewProposalManager(params),
		firstHistory: utils.NewHistory(maxHistoryCapacity),
		lastHistory:  utils.NewHistory(maxHistoryCapacity),
	}
	committee.state.SetManager(committee.manager)
	params.CkpManager.Register(NewCheckpoint(committee))
	return committee
}
