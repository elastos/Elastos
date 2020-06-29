// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package state

import (
	"bytes"
	"errors"
	"sort"
	"strconv"
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
	mtx                  sync.RWMutex
	state                *State
	params               *config.Params
	manager              *ProposalManager
	firstHistory         *utils.History
	lastHistory          *utils.History
	appropriationHistory *utils.History

	getCheckpoint                    func(height uint32) *Checkpoint
	getHeight                        func() uint32
	isCurrent                        func() bool
	broadcast                        func(msg p2p.Message)
	appendToTxpool                   func(transaction *types.Transaction) elaerr.ELAError
	createCRCAppropriationTx         func() (*types.Transaction, error)
	createCRAssetsRectifyTransaction func() (*types.Transaction, error)
	createCRRealWithdrawTransaction  func(withdrawTransactionHashes []common.Uint256,
		outputs []*types.OutputInfo) (*types.Transaction, error)
	getUTXO func(programHash *common.Uint168) ([]*types.UTXO, error)
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

func (c *Committee) IsElectedCRMemberByDID(did common.Uint168) bool {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	for _, v := range c.Members {
		if v.Info.DID.IsEqual(did) && v.MemberState == MemberElected {
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

func (c *Committee) GetCROnDutyStartHeight() uint32 {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.LastCommitteeHeight
}

func (c *Committee) GetCROnDutyPeriod() uint32 {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.params.CRDutyPeriod
}

func (c *Committee) GetCRVotingStartHeight() uint32 {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.LastVotingStartHeight
}

func (c *Committee) GetCRVotingPeriod() uint32 {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.params.CRVotingPeriod
}

func (c *Committee) IsProposalAllowed(height uint32) bool {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	if !c.InElectionPeriod {
		return false
	}
	return !c.isInVotingPeriod(height)
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

// get all CRMembers ordered by CID
func (c *Committee) GetAllMembers() []*CRMember {
	c.mtx.RLock()
	defer c.mtx.RUnlock()

	result := getCRMembers(c.Members)
	sort.Slice(result, func(i, j int) bool {
		return result[i].Info.DID.Compare(result[j].Info.DID) <= 0
	})
	return result
}

// get all elected CRMembers
func (c *Committee) GetElectedMembers() []*CRMember {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	if !c.InElectionPeriod {
		return []*CRMember{}
	}
	return getElectedCRMembers(c.Members)
}

// get all impeachable CRMembers
func (c *Committee) GetImpeachableMembers() []*CRMember {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	if !c.InElectionPeriod {
		return []*CRMember{}
	}
	return getImpeachableCRMembers(c.Members)
}

// get all history CRMembers
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

// update Candidates state in voting period
func (c *Committee) updateVotingCandidatesState(height uint32) {
	if c.isInVotingPeriod(height) {
		// Check if any pending candidates has got 6 confirms, set them to activate.
		activateCandidateFromPending :=
			func(key common.Uint168, candidate *Candidate) {
				c.state.history.Append(height, func() {
					candidate.state = Active
					c.state.Candidates[key] = candidate
				}, func() {
					candidate.state = Pending
					c.state.Candidates[key] = candidate
				})
			}

		pendingCandidates := c.state.getCandidates(Pending)

		if len(pendingCandidates) > 0 {
			for _, candidate := range pendingCandidates {
				if height-candidate.registerHeight+1 >= ActivateDuration {
					activateCandidateFromPending(candidate.info.CID, candidate)
				}
			}
		}
	}
}

// update Candidates deposit coin
func (c *Committee) updateCandidatesDepositCoin(height uint32) {
	updateDepositCoin := func(key common.Uint168, candidate *Candidate) {
		oriRefundable := c.state.depositInfo[key].Refundable
		oriDepositAmount := c.state.depositInfo[key].DepositAmount
		c.state.history.Append(height, func() {
			c.state.depositInfo[key].DepositAmount -= MinDepositAmount
			c.state.depositInfo[key].Refundable = true
		}, func() {
			c.state.depositInfo[key].DepositAmount = oriDepositAmount
			c.state.depositInfo[key].Refundable = oriRefundable
		})
	}

	canceledCandidates := c.state.getCandidates(Canceled)
	for _, candidate := range canceledCandidates {
		if height-candidate.CancelHeight() == c.params.CRDepositLockupBlocks {
			updateDepositCoin(candidate.info.CID, candidate)
		}
	}
}

func (c *Committee) ProcessBlock(block *types.Block, confirm *payload.Confirm) {
	c.mtx.Lock()
	defer c.mtx.Unlock()

	if block.Height < c.params.CRVotingStartHeight {
		return
	}

	// Get UTXOs of CR assets address and committee address.
	c.recordCRCRelatedAddressOutputs(block)

	// If reached the voting start height, record the last voting start height.
	c.recordLastVotingStartHeight(block.Height)

	c.processTransactions(block.Transactions, block.Height)
	c.updateVotingCandidatesState(block.Height)
	c.updateCandidatesDepositCoin(block.Height)
	c.state.history.Commit(block.Height)

	inElectionPeriod := c.tryStartVotingPeriod(block.Height)
	c.updateProposals(block.Height, inElectionPeriod)
	c.freshCirculationAmount(c.lastHistory, block.Height)
	needChg := false
	if c.shouldChange(block.Height) && c.changeCommittee(block.Height) {
		needChg = true
	}
	c.lastHistory.Commit(block.Height)

	if block.Height >= c.params.CRAssetsRectifyTransactionHeight &&
		len(c.manager.WithdrawableTxInfo) != 0 {
		c.createRealWithdrawTransaction(block.Height)
	}

	if needChg {
		c.createAppropriationTransaction(block.Height)
		c.recordCurrentStageAmount(block.Height)
		c.appropriationHistory.Commit(block.Height)
	} else {
		if c.CRAssetsAddressUTXOCount >=
			c.params.MaxCRAssetsAddressUTXOCount+c.params.CoinbaseMaturity &&
			block.Height >= c.params.CRAssetsRectifyTransactionHeight {
			c.createRectifyCRAssetsTransaction(block.Height)
		}
	}

}

func (c *Committee) updateProposals(height uint32, inElectionPeriod bool) {
	unusedAmount := c.manager.updateProposals(
		height, c.CirculationAmount, inElectionPeriod)
	c.manager.history.Append(height, func() {
		c.CRCCommitteeUsedAmount -= unusedAmount
	}, func() {
		c.CRCCommitteeUsedAmount += unusedAmount
	})
	c.manager.history.Commit(height)
}

func (c *Committee) updateCRMembers(height uint32, inElectionPeriod bool) uint32 {
	if !inElectionPeriod {
		return 0
	}
	circulation := c.CirculationAmount
	var count uint32
	for _, v := range c.Members {
		if v.MemberState != MemberElected {
			continue
		}

		if v.ImpeachmentVotes >= common.Fixed64(float64(circulation)*
			c.params.VoterRejectPercentage/100.0) {
			c.transferCRMemberState(v, height)
			count++
		}
	}
	return count
}

func (c *Committee) transferCRMemberState(crMember *CRMember, height uint32) {
	oriPenalty := c.state.depositInfo[crMember.Info.CID].Penalty
	oriRefundable := c.state.depositInfo[crMember.Info.CID].Refundable
	oriDepositAmount := c.state.depositInfo[crMember.Info.CID].DepositAmount
	oriMemberState := crMember.MemberState
	penalty := c.getMemberPenalty(height, crMember, true)
	c.lastHistory.Append(height, func() {
		crMember.MemberState = MemberImpeached
		c.state.depositInfo[crMember.Info.CID].Penalty = penalty
		c.state.depositInfo[crMember.Info.CID].DepositAmount -= MinDepositAmount
		c.state.depositInfo[crMember.Info.CID].Refundable = true
	}, func() {
		crMember.MemberState = oriMemberState
		c.state.depositInfo[crMember.Info.CID].Penalty = oriPenalty
		c.state.depositInfo[crMember.Info.CID].Refundable = oriRefundable
		c.state.depositInfo[crMember.Info.CID].DepositAmount = oriDepositAmount
	})
	return
}

func (c *Committee) changeCommittee(height uint32) bool {
	if c.shouldCleanHistory() {
		oriHistoryMembers := copyHistoryMembersMap(c.HistoryMembers)
		oriHistoryCandidates := copyHistoryCandidateMap(c.state.HistoryCandidates)
		c.lastHistory.Append(height, func() {
			c.HistoryMembers = make(map[uint64]map[common.Uint168]*CRMember)
			c.state.HistoryCandidates = make(map[uint64]map[common.Uint168]*Candidate)
		}, func() {
			c.HistoryMembers = oriHistoryMembers
			c.state.HistoryCandidates = oriHistoryCandidates
		})
	}
	if err := c.changeCommitteeMembers(height); err != nil {
		log.Warn("[ProcessBlock] change committee members error: ", err)
		return false
	}

	c.resetCRCCommitteeUsedAmount(height)
	c.resetProposalHashesSet(height)
	return true
}

func (c *Committee) createAppropriationTransaction(height uint32) {
	if c.createCRCAppropriationTx != nil && height == c.getHeight() {
		tx, err := c.createCRCAppropriationTx()
		if err != nil {
			log.Error("create appropriation tx failed:", err.Error())
			return
		} else if tx == nil {
			log.Info("no need to create appropriation")
			oriNeedAppropriation := c.NeedAppropriation
			c.appropriationHistory.Append(height, func() {
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
					} else {
						log.Warn("create CRCAppropriation append to tx pool err ", err)
					}
				}
			}()
		}
	}
	return
}

func (c *Committee) createRectifyCRAssetsTransaction(height uint32) {
	if c.createCRAssetsRectifyTransaction != nil && height == c.getHeight() {
		tx, err := c.createCRAssetsRectifyTransaction()
		if err != nil {
			log.Error("create rectify UTXOs tx failed:", err.Error())
			return
		}

		log.Info("create rectify UTXOs transaction:", tx.Hash())
		if c.isCurrent != nil && c.broadcast != nil && c.
			appendToTxpool != nil {
			go func() {
				if c.isCurrent() {
					if err := c.appendToTxpool(tx); err == nil {
						c.broadcast(msg.NewTx(tx))
					} else {
						log.Warn("create rectify UTXOs append to tx pool err ", err)
					}
				}
			}()
		}
	}
	return
}

func (c *Committee) createRealWithdrawTransaction(height uint32) {
	if c.createCRRealWithdrawTransaction != nil && height == c.getHeight() {
		withdrawTransactionHahses := make([]common.Uint256, 0)
		ouputs := make([]*types.OutputInfo, 0)
		for k, v := range c.manager.WithdrawableTxInfo {
			withdrawTransactionHahses = append(withdrawTransactionHahses, k)
			outputInfo := v
			ouputs = append(ouputs, &outputInfo)
		}
		tx, err := c.createCRRealWithdrawTransaction(withdrawTransactionHahses, ouputs)
		if err != nil {
			log.Error("create real withdraw tx failed:", err.Error())
			return
		}

		log.Info("create real withdraw transaction:", tx.Hash())
		if c.isCurrent != nil && c.broadcast != nil && c.
			appendToTxpool != nil {
			go func() {
				if c.isCurrent() {
					if err := c.appendToTxpool(tx); err == nil {
						c.broadcast(msg.NewTx(tx))
					} else {
						log.Warn("create real withdraw transaction "+
							"append to tx pool err ", err)
					}
				}
			}()
		}
	}
	return
}

func (c *Committee) resetCRCCommitteeUsedAmount(height uint32) {
	// todo add finished proposals into finished map
	var budget common.Fixed64
	for _, v := range c.manager.Proposals {
		if v.Status == CRCanceled || v.Status == VoterCanceled ||
			v.Status == Aborted {
			continue
		}
		if v.Status == Terminated || v.Status == Finished {
			for _, b := range v.Proposal.Budgets {
				if _, ok := v.WithdrawableBudgets[b.Stage]; !ok {
					continue
				}
				if _, ok := v.WithdrawnBudgets[b.Stage]; ok {
					continue
				}
				budget += b.Amount
			}
			continue
		}
		for _, b := range v.Proposal.Budgets {
			if _, ok := v.WithdrawnBudgets[b.Stage]; !ok {
				budget += b.Amount
			}
		}
	}

	oriUsedAmount := c.CRCCommitteeUsedAmount
	c.lastHistory.Append(height, func() {
		c.CRCCommitteeUsedAmount = budget
	}, func() {
		c.CRCCommitteeUsedAmount = oriUsedAmount
	})

	oriNeedAppropriation := c.NeedAppropriation
	if c.getHeight != nil && height == c.getHeight() {
		c.lastHistory.Append(height, func() {
			c.NeedAppropriation = true
		}, func() {
			c.NeedAppropriation = oriNeedAppropriation
		})
	}

}

func (c *Committee) resetProposalHashesSet(height uint32) {
	oriHashesSet := c.manager.ProposalHashes
	c.lastHistory.Append(height, func() {
		c.manager.ProposalHashes = make(map[common.Uint168]ProposalHashSet)
	}, func() {
		c.manager.ProposalHashes = oriHashesSet
	})
}

func (c *Committee) recordCurrentStageAmount(height uint32) {
	var lockedAmount common.Fixed64
	for _, v := range c.CRCFoundationLockedAmounts {
		lockedAmount += v
	}
	oriCurrentStageAmount := c.CRCCurrentStageAmount
	oriAppropriationAmount := c.AppropriationAmount
	c.appropriationHistory.Append(height, func() {
		c.AppropriationAmount = common.Fixed64(float64(c.CRCFoundationBalance-
			lockedAmount) * c.params.CRCAppropriatePercentage / 100.0)
		c.CRCCurrentStageAmount = c.CRCCommitteeBalance + c.AppropriationAmount
		log.Infof("current stage amount:%s,appropriation amount:%s",
			c.CRCCurrentStageAmount, c.AppropriationAmount)
		log.Infof("CR expenses address balance: %s,CR assets address "+
			"balance:%s, locked amount: %s,CR expenses address used amount: %s",
			c.CRCCommitteeBalance, c.CRCFoundationBalance,
			lockedAmount, c.CRCCommitteeUsedAmount)
	}, func() {
		c.CRCCurrentStageAmount = oriCurrentStageAmount
		c.AppropriationAmount = oriAppropriationAmount
	})
}

func (c *Committee) recordCRCRelatedAddressOutputs(block *types.Block) {
	for _, tx := range block.Transactions {
		for i, output := range tx.Outputs {
			if output.ProgramHash.IsEqual(c.params.CRAssetsAddress) {
				key := types.NewOutPoint(tx.Hash(), uint16(i)).ReferKey()
				value := output.Value
				c.firstHistory.Append(block.Height, func() {
					c.state.CRCFoundationOutputs[key] = value
				}, func() {
					delete(c.state.CRCFoundationOutputs, key)
				})
			} else if output.ProgramHash.IsEqual(c.params.CRExpensesAddress) {
				key := types.NewOutPoint(tx.Hash(), uint16(i)).ReferKey()
				value := output.Value
				c.firstHistory.Append(block.Height, func() {
					c.state.CRCCommitteeOutputs[key] = value
				}, func() {
					delete(c.state.CRCCommitteeOutputs, key)
				})
			}
		}
	}
	c.firstHistory.Commit(block.Height)
}

func (c *Committee) freshCirculationAmount(history *utils.History, height uint32) {
	circulationAmount := common.Fixed64(config.OriginIssuanceAmount) +
		common.Fixed64(height)*c.params.RewardPerBlock -
		c.CRCFoundationBalance - c.CRCCommitteeBalance - c.DestroyedAmount
	oriCirculationAmount := c.CirculationAmount
	history.Append(height, func() {
		c.CirculationAmount = circulationAmount
	}, func() {
		c.CirculationAmount = oriCirculationAmount
	})
}

func (c *Committee) recordLastVotingStartHeight(height uint32) {
	if c.isInVotingPeriod(height) {
		return
	}
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

func (c *Committee) tryStartVotingPeriod(height uint32) (inElection bool) {
	inElection = c.InElectionPeriod
	if !c.InElectionPeriod {
		return
	}

	newImpeachedCount := c.updateCRMembers(height, inElection)

	var normalCount uint32
	for _, m := range c.Members {
		if m.MemberState == MemberElected {
			normalCount++
		}
	}
	if normalCount-newImpeachedCount < c.params.CRAgreementCount {
		lastVotingStartHeight := c.LastVotingStartHeight
		inElectionPeriod := c.InElectionPeriod
		c.lastHistory.Append(height, func() {
			c.InElectionPeriod = false
			if c.LastVotingStartHeight == 0 {
				c.LastVotingStartHeight = height
			} else if !c.isInVotingPeriod(height) {
				c.LastVotingStartHeight = height
			}
		}, func() {
			c.InElectionPeriod = inElectionPeriod
			c.LastVotingStartHeight = lastVotingStartHeight
		})
		inElection = false

		for _, v := range c.Members {
			if v.MemberState == MemberElected &&
				v.ImpeachmentVotes < common.Fixed64(float64(c.CirculationAmount)*
					c.params.VoterRejectPercentage/100.0) {
				c.terminateCRMember(v, height)
			}
		}
	}

	return
}

func (c *Committee) terminateCRMember(crMember *CRMember, height uint32) {
	oriPenalty := c.state.depositInfo[crMember.Info.CID].Penalty
	oriRefundable := c.state.depositInfo[crMember.Info.CID].Refundable
	oriDepositAmount := c.state.depositInfo[crMember.Info.CID].DepositAmount
	oriMemberState := crMember.MemberState
	penalty := c.getMemberPenalty(height, crMember, false)
	c.lastHistory.Append(height, func() {
		crMember.MemberState = MemberTerminated
		c.state.depositInfo[crMember.Info.CID].Penalty = penalty
		c.state.depositInfo[crMember.Info.CID].DepositAmount -= MinDepositAmount
		c.state.depositInfo[crMember.Info.CID].Refundable = true
	}, func() {
		crMember.MemberState = oriMemberState
		c.state.depositInfo[crMember.Info.CID].Penalty = oriPenalty
		c.state.depositInfo[crMember.Info.CID].Refundable = oriRefundable
		c.state.depositInfo[crMember.Info.CID].DepositAmount = oriDepositAmount
	})
}

func (c *Committee) processImpeachment(height uint32, member []byte,
	votes common.Fixed64, history *utils.History) {
	var crMember *CRMember
	for _, v := range c.Members {
		if bytes.Equal(v.Info.CID.Bytes(), member) &&
			v.MemberState == MemberElected {
			crMember = v
			break
		}
	}
	if crMember == nil {
		return
	}
	history.Append(height, func() {
		crMember.ImpeachmentVotes += votes
	}, func() {
		crMember.ImpeachmentVotes -= votes
	})
	return
}

func (c *Committee) processCRCAppropriation(height uint32, history *utils.History) {
	history.Append(height, func() {
		c.NeedAppropriation = false
	}, func() {
		c.NeedAppropriation = true
	})
}

func (c *Committee) processCRCRealWithdraw(tx *types.Transaction,
	height uint32, history *utils.History) {

	txs := make(map[common.Uint256]types.OutputInfo)
	for k, v := range c.manager.WithdrawableTxInfo {
		txs[k] = v
	}
	withdrawPayload := tx.Payload.(*payload.CRCProposalRealWithdraw)
	history.Append(height, func() {
		for _, hash := range withdrawPayload.WithdrawTransactionHashes {
			delete(c.manager.WithdrawableTxInfo, hash)
		}
	}, func() {
		c.manager.WithdrawableTxInfo = txs
	})
}

func (c *Committee) GetDepositAmountByPublicKey(
	publicKey string) (common.Fixed64, common.Fixed64, error) {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	pubkey, err := common.HexStringToBytes(publicKey)
	if err != nil {
		return 0, 0, errors.New("invalid public key")
	}
	return c.state.getDepositAmountByPublicKey(pubkey)
}

func (c *Committee) GetDepositAmountByID(
	id common.Uint168) (common.Fixed64, common.Fixed64, error) {
	c.mtx.RLock()
	defer c.mtx.RUnlock()

	cid, exist := c.state.getExistCIDByID(id)
	if !exist {
		return 0, 0, errors.New("ID does not exist")
	}
	refundable := c.IsRefundable(*cid)
	return c.state.getDepositAmountByCID(*cid, refundable)
}

func (c *Committee) GetAvailableDepositAmount(cid common.Uint168) common.Fixed64 {
	c.mtx.RLock()
	defer c.mtx.RUnlock()

	return c.state.getAvailableDepositAmount(cid)
}

func (c *Committee) getHistoryMember(code []byte) []*CRMember {
	members := make([]*CRMember, 0)
	for _, v := range c.HistoryMembers {
		for _, m := range v {
			if bytes.Equal(m.Info.Code, code) {
				members = append(members, m)
			}
		}
	}
	return members
}

func (c *Committee) RollbackTo(height uint32) error {
	c.mtx.Lock()
	defer c.mtx.Unlock()
	currentHeight := c.lastHistory.Height()
	for i := currentHeight - 1; i >= height; i-- {
		if err := c.appropriationHistory.RollbackTo(i); err != nil {
			log.Debug("committee appropriationHistory rollback err:", err)
		}
		if err := c.lastHistory.RollbackTo(i); err != nil {
			log.Debug("committee last history rollback err:", err)
		}
		if err := c.manager.history.RollbackTo(i); err != nil {
			log.Debug("manager rollback err:", err)
		}
		if err := c.state.rollbackTo(i); err != nil {
			log.Debug("state rollback err:", err)
		}
		if err := c.firstHistory.RollbackTo(i); err != nil {
			log.Debug("committee first history rollback err:", err)
		}
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

	if c.LastVotingStartHeight == 0 {
		return height == c.LastCommitteeHeight+c.params.CRDutyPeriod
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
	if c.InElectionPeriod == true {
		c.processCurrentMembersDepositInfo(height)
	}

	candidates := c.getActiveAndExistDIDCRCandidatesDesc()
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
		return errors.New("candidates count less than required count height" + strconv.Itoa(int(height)))
	}
	// Process current members.
	newMembers := c.processCurrentMembersHistory(height, candidates)

	// Process current candidates.
	c.processCurrentCandidates(height, candidates, newMembers)

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

func (c *Committee) processCurrentMembersHistory(height uint32,
	activeCandidates []*Candidate) map[common.Uint168]*CRMember {

	oriMembers := copyMembersMap(c.Members)
	if len(c.Members) != 0 {
		if _, ok := c.HistoryMembers[c.state.CurrentSession]; !ok {
			currentSession := c.state.CurrentSession
			c.lastHistory.Append(height, func() {
				c.HistoryMembers[currentSession] =
					make(map[common.Uint168]*CRMember)
			}, func() {
				delete(c.HistoryMembers, currentSession)
			})
		}

		for _, m := range oriMembers {
			member := *m
			c.lastHistory.Append(height, func() {
				c.HistoryMembers[c.state.CurrentSession][member.Info.CID] = &member
			}, func() {
				delete(c.HistoryMembers[c.state.CurrentSession], member.Info.CID)
			})
		}
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
	return newMembers
}

func (c *Committee) processCurrentMembersDepositInfo(height uint32) {
	oriMembers := copyMembersMap(c.Members)
	if len(c.Members) != 0 {
		for _, m := range oriMembers {
			member := *m
			if member.MemberState != MemberElected {
				continue
			}
			oriPenalty := c.state.depositInfo[m.Info.CID].Penalty
			oriRefundable := c.state.depositInfo[m.Info.CID].Refundable
			oriDepositAmount := c.state.depositInfo[m.Info.CID].DepositAmount
			var dpositAmount common.Fixed64
			dpositAmount = MinDepositAmount
			penalty := c.getMemberPenalty(height, &member, false)
			c.lastHistory.Append(height, func() {
				c.state.depositInfo[member.Info.CID].Penalty = penalty
				c.state.depositInfo[member.Info.CID].Refundable = true
				c.state.depositInfo[member.Info.CID].DepositAmount -= dpositAmount
			}, func() {
				c.state.depositInfo[member.Info.CID].Penalty = oriPenalty
				c.state.depositInfo[member.Info.CID].Refundable = oriRefundable
				c.state.depositInfo[member.Info.CID].DepositAmount = oriDepositAmount
			})
		}
	}
}

func (c *Committee) processCurrentCandidates(height uint32,
	activeCandidates []*Candidate, newMembers map[common.Uint168]*CRMember) {
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
	for _, candidate := range oriCandidate {
		ca := *candidate
		// if candidate changed to member, no need to update deposit coin again.
		if _, ok := newMembers[ca.info.DID]; ok {
			continue
		}
		// if canceled enough blocks, no need to update deposit coin again.
		if ca.state == Canceled && height-ca.CancelHeight() >= c.params.CRDepositLockupBlocks {
			continue
		}
		// if CR deposit coin is returned, no need to update deposit coin again.
		if ca.state == Returned {
			continue
		}
		oriRefundable := c.state.depositInfo[ca.info.CID].Refundable
		c.lastHistory.Append(height, func() {
			c.state.depositInfo[ca.info.CID].Refundable = true
			c.state.depositInfo[ca.info.CID].DepositAmount -= MinDepositAmount
		}, func() {
			c.state.depositInfo[ca.info.CID].Refundable = oriRefundable
			c.state.depositInfo[ca.info.CID].DepositAmount += MinDepositAmount
		})
	}
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
		MemberState:      MemberElected,
	}
}

func (c *Committee) getMemberPenalty(height uint32, member *CRMember, impeached bool) common.Fixed64 {
	// Calculate penalty by election block count.
	electionCount := height - c.LastCommitteeHeight
	var electionRate float64
	if impeached {
		electionRate = float64(electionCount) / float64(c.params.CRDutyPeriod)
	} else {
		electionRate = 1
	}

	// Calculate penalty by vote proposal count.
	var voteCount int
	for _, v := range c.manager.ProposalSession[c.state.CurrentSession] {
		proposal := c.manager.Proposals[v]
		for did, _ := range proposal.CRVotes {
			if member.Info.DID.IsEqual(did) {
				voteCount++
				break
			}
		}
	}
	proposalsCount := len(c.manager.ProposalSession[c.state.CurrentSession])
	voteRate := float64(1.0)
	if proposalsCount != 0 {
		voteRate = float64(voteCount) / float64(proposalsCount)
	}

	// Calculate the final penalty.
	penalty := c.state.depositInfo[member.Info.CID].Penalty
	currentPenalty := common.Fixed64(float64(MinDepositAmount) * (1 - electionRate*voteRate))
	finalPenalty := penalty + currentPenalty

	log.Infof("height %d member %s, not election and not vote proposal"+
		" penalty: %s, old penalty: %s, final penalty: %s",
		height, member.Info.NickName, currentPenalty, penalty, finalPenalty)
	log.Info("electionRate:", electionRate, "voteRate:", voteRate,
		"electionCount:", electionCount, "dutyPeriod:", c.params.CRDutyPeriod,
		"voteCount:", voteCount, "proposalsCount:", proposalsCount)

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

func (c *Committee) getActiveAndExistDIDCRCandidatesDesc() []*Candidate {
	emptyDID := common.Uint168{}
	candidates := c.state.getCandidateFromMap(c.state.Candidates,
		func(candidate *Candidate) bool {
			if candidate.info.DID.IsEqual(emptyDID) {
				return false
			}
			return candidate.state == Active
		})

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

func (c *Committee) GetCandidate(cid common.Uint168) *Candidate {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.state.getCandidate(cid)
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

func (c *Committee) ExistCandidateByDepositHash(hash common.Uint168) bool {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.state.existCandidateByDepositHash(hash)
}

func (c *Committee) GetPenalty(cid common.Uint168) common.Fixed64 {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.state.getPenalty(cid)
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

func (c *Committee) Exist(cid common.Uint168) bool {
	c.mtx.RLock()
	defer c.mtx.RUnlock()

	return c.state.exist(cid)
}

func (c *Committee) IsRefundable(cid common.Uint168) bool {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.state.isRefundable(cid)
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

func (c *Committee) GetRealWithdrawTransactions() map[common.Uint256]types.OutputInfo {
	c.mtx.RLock()
	defer c.mtx.RUnlock()

	return c.manager.WithdrawableTxInfo
}

// GetCandidateByID returns candidate with specified cid or did, it will return
// nil if not found.
func (c *Committee) GetCandidateByID(id common.Uint168) *Candidate {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.state.getCandidateByID(id)
}

// GetCandidateByCID returns candidate with specified cid, it will return nil
// if not found.
func (c *Committee) GetCandidateByCID(cid common.Uint168) *Candidate {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	return c.state.getCandidate(cid)
}

// GetCandidateByPublicKey returns candidate with specified public key, it will
// return nil if not found.
func (c *Committee) GetCandidateByPublicKey(publicKey string) *Candidate {
	c.mtx.RLock()
	defer c.mtx.RUnlock()
	pubkey, err := common.HexStringToBytes(publicKey)
	if err != nil {
		return nil
	}
	return c.state.getCandidateByPublicKey(pubkey)
}

type CommitteeFuncsConfig struct {
	GetTxReference func(tx *types.Transaction) (
		map[*types.Input]types.Output, error)
	GetHeight                        func() uint32
	CreateCRAppropriationTransaction func() (*types.Transaction, error)
	CreateCRAssetsRectifyTransaction func() (*types.Transaction, error)
	CreateCRRealWithdrawTransaction  func(withdrawTransactionHashes []common.Uint256,
		outpus []*types.OutputInfo) (*types.Transaction, error)
	IsCurrent      func() bool
	Broadcast      func(msg p2p.Message)
	AppendToTxpool func(transaction *types.Transaction) elaerr.ELAError
	GetUTXO        func(programHash *common.Uint168) ([]*types.UTXO, error)
}

func (c *Committee) RegisterFuncitons(cfg *CommitteeFuncsConfig) {
	c.createCRCAppropriationTx = cfg.CreateCRAppropriationTransaction
	c.createCRAssetsRectifyTransaction = cfg.CreateCRAssetsRectifyTransaction
	c.createCRRealWithdrawTransaction = cfg.CreateCRRealWithdrawTransaction
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

func (c *Committee) Snapshot() *CommitteeKeyFrame {
	keyFrame := &CommitteeKeyFrame{
		KeyFrame:         c.KeyFrame.Snapshot(),
		StateKeyFrame:    c.state.StateKeyFrame.Snapshot(),
		ProposalKeyFrame: c.manager.ProposalKeyFrame.Snapshot(),
	}

	return keyFrame
}

func NewCommittee(params *config.Params) *Committee {
	committee := &Committee{
		state:                NewState(params),
		params:               params,
		KeyFrame:             *NewKeyFrame(),
		manager:              NewProposalManager(params),
		firstHistory:         utils.NewHistory(maxHistoryCapacity),
		lastHistory:          utils.NewHistory(maxHistoryCapacity),
		appropriationHistory: utils.NewHistory(maxHistoryCapacity),
	}
	committee.manager.InitSecretaryGeneralPublicKey(params.SecretaryGeneral)
	committee.state.SetManager(committee.manager)
	params.CkpManager.Register(NewCheckpoint(committee))
	return committee
}
