// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package state

import (
	"bytes"
	"encoding/hex"
	"errors"
	"fmt"
	"math"
	"sort"
	"strings"
	"sync"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/cr/state"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"
	"github.com/elastos/Elastos.ELA/events"
)

type ChangeType byte

const (
	// MajoritySignRatioNumerator defines the ratio numerator to achieve
	// majority signatures.
	MajoritySignRatioNumerator = float64(2)

	// MajoritySignRatioDenominator defines the ratio denominator to achieve
	// majority signatures.
	MajoritySignRatioDenominator = float64(3)

	// MaxNormalInactiveChangesCount defines the max count arbitrators can
	// change when more than 1/3 arbiters don't sign cause to confirm fail
	MaxNormalInactiveChangesCount = 3

	// MaxSnapshotLength defines the max length the snapshot map should take
	MaxSnapshotLength = 20

	none         = ChangeType(0x00)
	updateNext   = ChangeType(0x01)
	normalChange = ChangeType(0x02)
)

var (
	ErrInsufficientProducer = errors.New("producers count less than min arbitrators count")
)

type arbitrators struct {
	*State
	*degradation
	chainParams      *config.Params
	crCommittee      *state.Committee
	bestHeight       func() uint32
	getBlockByHeight func(uint32) (*types.Block, error)

	mtx       sync.Mutex
	started   bool
	dutyIndex int

	CurrentReward RewardData
	NextReward    RewardData

	currentArbitrators         []ArbiterMember
	currentCandidates          []ArbiterMember
	nextArbitrators            []ArbiterMember
	nextCandidates             []ArbiterMember
	crcArbiters                map[common.Uint168]ArbiterMember
	crcChangedHeight           uint32
	accumulativeReward         common.Fixed64
	finalRoundChange           common.Fixed64
	clearingHeight             uint32
	arbitersRoundReward        map[common.Uint168]common.Fixed64
	illegalBlocksPayloadHashes map[common.Uint256]interface{}

	snapshots            map[uint32][]*CheckPoint
	snapshotKeysDesc     []uint32
	lastCheckPointHeight uint32

	forceChanged bool
}

func (a *arbitrators) Start() {
	a.mtx.Lock()
	a.started = true
	a.mtx.Unlock()
}

func (a *arbitrators) RegisterFunction(bestHeight func() uint32,
	getBlockByHeight func(uint32) (*types.Block, error),
	getTxReference func(tx *types.Transaction) (
		map[*types.Input]*types.Output, error)) {
	a.bestHeight = bestHeight
	a.getBlockByHeight = getBlockByHeight
	a.getTxReference = getTxReference
}

func (a *arbitrators) RecoverFromCheckPoints(point *CheckPoint) {
	a.mtx.Lock()
	a.recoverFromCheckPoints(point)
	a.mtx.Unlock()
}

func (a *arbitrators) recoverFromCheckPoints(point *CheckPoint) {
	a.dutyIndex = point.DutyIndex
	a.currentArbitrators = point.CurrentArbitrators
	a.currentCandidates = point.CurrentCandidates
	a.nextArbitrators = point.NextArbitrators
	a.nextCandidates = point.NextCandidates
	a.CurrentReward = point.CurrentReward
	a.NextReward = point.NextReward
	a.StateKeyFrame = &point.StateKeyFrame
	a.accumulativeReward = point.accumulativeReward
	a.finalRoundChange = point.finalRoundChange
	a.clearingHeight = point.clearingHeight
	a.arbitersRoundReward = point.arbitersRoundReward
	a.illegalBlocksPayloadHashes = point.illegalBlocksPayloadHashes
}

func (a *arbitrators) ProcessBlock(block *types.Block, confirm *payload.Confirm) {
	a.State.ProcessBlock(block, confirm)
	a.IncreaseChainHeight(block)
}

func (a *arbitrators) CheckDPOSIllegalTx(block *types.Block) error {

	a.mtx.Lock()
	hashes := a.illegalBlocksPayloadHashes
	a.mtx.Unlock()

	if hashes == nil || len(hashes) == 0 {
		return nil
	}

	foundMap := make(map[common.Uint256]bool)
	for k := range hashes {
		foundMap[k] = false
	}

	for _, tx := range block.Transactions {
		if tx.IsIllegalBlockTx() {
			foundMap[tx.Payload.(*payload.DPOSIllegalBlocks).Hash()] = true
		}
	}

	for _, found := range foundMap {
		if !found {
			return errors.New("expect an illegal blocks transaction in this block")
		}
	}
	return nil
}

func (a *arbitrators) CheckCRCAppropriationTx(block *types.Block) error {
	a.mtx.Lock()
	needAppropriation := a.crCommittee.NeedAppropriation
	a.mtx.Unlock()

	var appropriationCount uint32
	for _, tx := range block.Transactions {
		if tx.IsCRCAppropriationTx() {
			appropriationCount++
		}
	}

	var needAppropriationCount uint32
	if needAppropriation {
		needAppropriationCount = 1
	}

	if appropriationCount != needAppropriationCount {
		return fmt.Errorf("current block height %d, appropriation "+
			"transaction count should be %d, current block contains %d",
			block.Height, needAppropriationCount, appropriationCount)
	}

	return nil
}

func (a *arbitrators) ProcessSpecialTxPayload(p types.Payload,
	height uint32) error {
	switch obj := p.(type) {
	case *payload.DPOSIllegalBlocks:
		a.mtx.Lock()
		a.illegalBlocksPayloadHashes[obj.Hash()] = nil
		a.mtx.Unlock()
	case *payload.InactiveArbitrators:
		if !a.AddInactivePayload(obj) {
			log.Debug("[ProcessSpecialTxPayload] duplicated payload")
			return nil
		}
	default:
		return errors.New("[ProcessSpecialTxPayload] invalid payload type")
	}

	a.State.ProcessSpecialTxPayload(p, height)
	return a.ForceChange(height)
}

func (a *arbitrators) RollbackTo(height uint32) error {
	if height > a.history.Height() {
		return fmt.Errorf("can't rollback to height: %d", height)
	}

	if err := a.State.RollbackTo(height); err != nil {
		return err
	}
	return a.DecreaseChainHeight(height)
}

func (a *arbitrators) GetDutyIndexByHeight(height uint32) (index int) {
	a.mtx.Lock()
	if height >= a.chainParams.CRCOnlyDPOSHeight-1 {
		index = a.dutyIndex % len(a.currentArbitrators)
	} else {
		index = int(height) % len(a.currentArbitrators)
	}
	a.mtx.Unlock()
	return index
}

func (a *arbitrators) GetDutyIndex() int {
	a.mtx.Lock()
	index := a.dutyIndex
	a.mtx.Unlock()

	return index
}

func (a *arbitrators) GetArbitersRoundReward() map[common.Uint168]common.Fixed64 {
	a.mtx.Lock()
	result := a.arbitersRoundReward
	a.mtx.Unlock()

	return result
}

func (a *arbitrators) GetFinalRoundChange() common.Fixed64 {
	a.mtx.Lock()
	result := a.finalRoundChange
	a.mtx.Unlock()

	return result
}

func (a *arbitrators) ForceChange(height uint32) error {
	a.mtx.Lock()
	defer a.mtx.Unlock()

	block, err := a.getBlockByHeight(height)
	if err != nil {
		block, err = a.getBlockByHeight(a.bestHeight())
		if err != nil {
			return err
		}
	}
	a.snapshot(height)

	if err := a.clearingDPOSReward(block, false); err != nil {
		return err
	}

	if err := a.updateNextArbitrators(height + 1); err != nil {
		return err
	}

	if err := a.changeCurrentArbitrators(); err != nil {
		return err
	}

	if a.started {
		go events.Notify(events.ETDirectPeersChanged,
			a.getNeedConnectArbiters())
	}

	a.forceChanged = true

	a.dumpInfo(height)
	return nil
}

func (a *arbitrators) tryHandleError(height uint32, err error) error {
	if err == ErrInsufficientProducer {
		log.Warn("found error: ", err, ", degrade to CRC only state")
		a.TrySetUnderstaffed(height)
		return nil
	} else {
		return err
	}
}

func (a *arbitrators) normalChange(height uint32) error {
	if err := a.changeCurrentArbitrators(); err != nil {
		log.Warn("[NormalChange] change current arbiters error: ", err)
		return err
	}

	if err := a.updateNextArbitrators(height + 1); err != nil {
		log.Warn("[NormalChange] update next arbiters error: ", err)
		return err
	}

	return nil
}

func (a *arbitrators) IncreaseChainHeight(block *types.Block) {
	var notify = true

	a.mtx.Lock()

	if block.Height > a.bestHeight()-MaxSnapshotLength {
		a.snapshot(block.Height)
	}

	changeType, versionHeight := a.getChangeType(block.Height + 1)
	switch changeType {
	case updateNext:
		if err := a.updateNextArbitrators(versionHeight); err != nil {
			panic(fmt.Sprintf("[IncreaseChainHeight] update next arbiters at height: %d, error: %s", block.Height, err))
		}
	case normalChange:
		if err := a.clearingDPOSReward(block, true); err != nil {
			panic(fmt.Sprintf("normal change fail when clear DPOS reward: "+
				" transaction, height: %d, error: %s", block.Height, err))
		}
		if err := a.normalChange(block.Height); err != nil {
			panic(fmt.Sprintf("normal change fail at height: %d, error: %s",
				block.Height, err))
		}
	case none:
		a.accumulateReward(block)
		a.dutyIndex++
		notify = false
	}
	a.illegalBlocksPayloadHashes = make(map[common.Uint256]interface{})

	a.mtx.Unlock()

	if a.started && notify {
		go events.Notify(events.ETDirectPeersChanged, a.GetNeedConnectArbiters())
	}
}

func (a *arbitrators) accumulateReward(block *types.Block) {
	if block.Height < a.State.chainParams.PublicDPOSHeight {
		return
	}

	if block.Height < a.chainParams.CRVotingStartHeight || !a.forceChanged {
		dposReward := a.getBlockDPOSReward(block)
		a.accumulativeReward += dposReward
	}

	a.arbitersRoundReward = nil
	a.finalRoundChange = 0
	a.forceChanged = false
}

func (a *arbitrators) clearingDPOSReward(block *types.Block,
	smoothClearing bool) error {
	if block.Height < a.chainParams.PublicDPOSHeight ||
		block.Height == a.clearingHeight {
		return nil
	}

	dposReward := a.getBlockDPOSReward(block)
	if smoothClearing {
		a.accumulativeReward += dposReward
		dposReward = 0
	}

	if err := a.distributeDPOSReward(block.Height,
		a.accumulativeReward); err != nil {
		return err
	}
	a.accumulativeReward = dposReward
	a.clearingHeight = block.Height

	return nil
}

func (a *arbitrators) distributeDPOSReward(height uint32,
	reward common.Fixed64) (err error) {
	a.arbitersRoundReward = map[common.Uint168]common.Fixed64{}

	var realDPOSReward common.Fixed64
	if height >= a.chainParams.CRCommitteeStartHeight &&
		a.crcChangedHeight > 0 {
		realDPOSReward, err = a.distributeWithNormalArbitratorsV(reward)
	} else {
		realDPOSReward, err = a.distributeWithNormalArbitratorsV0(reward)
	}

	if err != nil {
		return err
	}

	change := reward - realDPOSReward
	if change < 0 {
		return errors.New("real dpos reward more than reward limit")
	}

	a.finalRoundChange = change
	return nil
}

func (a *arbitrators) distributeWithNormalArbitratorsV(
	reward common.Fixed64) (common.Fixed64, error) {
	if len(a.currentArbitrators) == 0 {
		return 0, errors.New("not found arbiters when distributeWithNormalArbitratorsV")
	}

	totalBlockConfirmReward := float64(reward) * 0.25
	totalTopProducersReward := float64(reward) - totalBlockConfirmReward
	individualBlockConfirmReward := common.Fixed64(
		math.Floor(totalBlockConfirmReward / float64(len(a.currentArbitrators))))
	totalVotesInRound := a.CurrentReward.TotalVotesInRound
	if len(a.chainParams.CRCArbiters) == len(a.currentArbitrators) {
		a.arbitersRoundReward[a.chainParams.CRCAddress] = reward
		return reward, nil
	}
	rewardPerVote := totalTopProducersReward / float64(totalVotesInRound)

	realDPOSReward := common.Fixed64(0)
	for _, arbiter := range a.currentArbitrators {
		ownerHash := arbiter.GetOwnerProgramHash()
		rewardHash := ownerHash
		var r common.Fixed64
		if member, ok := a.crcArbiters[ownerHash]; ok {
			r = individualBlockConfirmReward
			if member.(*crcArbiter).crMember.MemberState != state.MemberElected {
				rewardHash = a.chainParams.DestroyELAAddress
			}
		} else {
			votes := a.CurrentReward.OwnerVotesInRound[ownerHash]
			individualProducerReward := common.Fixed64(math.Floor(float64(
				votes) * rewardPerVote))
			r = individualBlockConfirmReward + individualProducerReward
		}
		a.arbitersRoundReward[rewardHash] = r
		realDPOSReward += r
	}
	for _, candidate := range a.currentCandidates {
		ownerHash := candidate.GetOwnerProgramHash()
		votes := a.CurrentReward.OwnerVotesInRound[ownerHash]
		individualProducerReward := common.Fixed64(math.Floor(float64(
			votes) * rewardPerVote))
		a.arbitersRoundReward[ownerHash] = individualProducerReward

		realDPOSReward += individualProducerReward
	}
	return realDPOSReward, nil
}

func (a *arbitrators) DecreaseChainHeight(height uint32) error {
	a.mtx.Lock()
	a.degradation.RollbackTo(height)

	checkpoints := a.getSnapshot(height + 1)
	if checkpoints != nil {
		for _, v := range checkpoints {
			a.recoverFromCheckPoints(v)
			// since we don't support force change at now, length of checkpoints
			// should be 1, so we break the loop here
			break
		}
	}
	a.mtx.Unlock()

	return nil
}

func (a *arbitrators) GetNeedConnectArbiters() []peer.PID {
	a.mtx.Lock()
	defer a.mtx.Unlock()

	return a.getNeedConnectArbiters()
}

func (a *arbitrators) getNeedConnectArbiters() []peer.PID {
	height := a.history.Height() + 1
	if height < a.chainParams.CRCOnlyDPOSHeight-a.chainParams.PreConnectOffset {
		return nil
	}

	pids := make(map[string]peer.PID)
	for _, p := range a.crcArbiters {
		var pid peer.PID
		copy(pid[:], p.GetNodePublicKey())
		pids[common.BytesToHexString(p.GetNodePublicKey())] = pid
	}

	for _, v := range a.currentArbitrators {
		key := common.BytesToHexString(v.GetNodePublicKey())
		var pid peer.PID
		copy(pid[:], v.GetNodePublicKey())
		pids[key] = pid
	}

	for _, v := range a.nextArbitrators {
		key := common.BytesToHexString(v.GetNodePublicKey())
		var pid peer.PID
		copy(pid[:], v.GetNodePublicKey())
		pids[key] = pid
	}

	peers := make([]peer.PID, 0, len(pids))
	for _, pid := range pids {
		peers = append(peers, pid)
	}

	return peers
}

func (a *arbitrators) IsArbitrator(pk []byte) bool {
	arbitrators := a.GetArbitrators()

	for _, v := range arbitrators {
		if bytes.Equal(pk, v) {
			return true
		}
	}
	return false
}

func (a *arbitrators) GetArbitrators() [][]byte {
	a.mtx.Lock()
	result := make([][]byte, 0, len(a.currentArbitrators))
	for _, v := range a.currentArbitrators {
		result = append(result, v.GetNodePublicKey())
	}
	a.mtx.Unlock()

	return result
}

func (a *arbitrators) GetCandidates() [][]byte {
	a.mtx.Lock()
	result := make([][]byte, 0, len(a.currentCandidates))
	for _, v := range a.currentCandidates {
		result = append(result, v.GetNodePublicKey())
	}
	a.mtx.Unlock()

	return result
}

func (a *arbitrators) GetNextArbitrators() [][]byte {
	a.mtx.Lock()
	result := make([][]byte, 0, len(a.nextArbitrators))
	for _, v := range a.nextArbitrators {
		result = append(result, v.GetNodePublicKey())
	}
	a.mtx.Unlock()

	return result
}

func (a *arbitrators) GetNextCandidates() [][]byte {
	a.mtx.Lock()
	result := make([][]byte, 0, len(a.nextCandidates))
	for _, v := range a.nextCandidates {
		result = append(result, v.GetNodePublicKey())
	}
	a.mtx.Unlock()

	return result
}

func (a *arbitrators) GetCRCArbiters() [][]byte {
	a.mtx.Lock()
	result := make([][]byte, 0, len(a.crcArbiters))
	for _, v := range a.crcArbiters {
		result = append(result, v.GetNodePublicKey())
	}
	a.mtx.Unlock()

	return result
}

func (a *arbitrators) GetCurrentRewardData() RewardData {
	a.mtx.Lock()
	result := a.CurrentReward
	a.mtx.Unlock()

	return result
}

func (a *arbitrators) GetNextRewardData() RewardData {
	a.mtx.Lock()
	result := a.NextReward
	a.mtx.Unlock()

	return result
}

func (a *arbitrators) IsCRCArbitrator(pk []byte) bool {
	a.mtx.Lock()
	defer a.mtx.Unlock()

	return a.isCRCArbitrator(pk)
}

func (a *arbitrators) isCRCArbitrator(pk []byte) bool {
	for _, v := range a.crcArbiters {
		if bytes.Equal(v.GetNodePublicKey(), pk) {
			return true
		}
	}
	return false
}

func (a *arbitrators) IsActiveProducer(pk []byte) bool {
	return a.State.IsActiveProducer(pk)
}

func (a *arbitrators) IsDisabledProducer(pk []byte) bool {
	return a.State.IsInactiveProducer(pk) || a.State.IsIllegalProducer(pk) || a.State.IsCanceledProducer(pk)
}

func (a *arbitrators) GetConnectedProducer(publicKey []byte) ArbiterMember {
	a.mtx.Lock()
	defer a.mtx.Unlock()

	for _, v := range a.crcArbiters {
		if bytes.Equal(v.GetNodePublicKey(), publicKey) {
			return v
		}
	}

	findByPk := func(arbiters []ArbiterMember) ArbiterMember {
		for _, v := range arbiters {
			if bytes.Equal(v.GetNodePublicKey(), publicKey) {
				return v
			}
		}
		return nil
	}
	if ar := findByPk(a.currentArbitrators); ar != nil {
		return ar
	}
	if ar := findByPk(a.currentCandidates); ar != nil {
		return ar
	}
	if ar := findByPk(a.nextArbitrators); ar != nil {
		return ar
	}
	if ar := findByPk(a.nextCandidates); ar != nil {
		return ar
	}

	return nil
}

func (a *arbitrators) CRCProducerCount() int {
	a.mtx.Lock()
	defer a.mtx.Unlock()
	return len(a.crcArbiters)
}

func (a *arbitrators) GetOnDutyArbitrator() []byte {
	return a.GetNextOnDutyArbitratorV(a.bestHeight()+1, 0).GetNodePublicKey()
}

func (a *arbitrators) GetNextOnDutyArbitrator(offset uint32) []byte {
	return a.GetNextOnDutyArbitratorV(a.bestHeight()+1, offset).GetNodePublicKey()
}

func (a *arbitrators) GetOnDutyCrossChainArbitrator() []byte {
	var arbiter []byte
	height := a.bestHeight()
	if height < a.chainParams.CRCOnlyDPOSHeight-1 {
		arbiter = a.GetOnDutyArbitrator()
	} else {
		crcArbiters := a.GetCRCArbiters()
		sort.Slice(crcArbiters, func(i, j int) bool {
			return bytes.Compare(crcArbiters[i], crcArbiters[j]) < 0
		})
		ondutyIndex := int(height-a.chainParams.CRCOnlyDPOSHeight+1) % len(crcArbiters)
		arbiter = crcArbiters[ondutyIndex]
	}

	return arbiter
}

func (a *arbitrators) GetCrossChainArbiters() [][]byte {
	if a.bestHeight() < a.chainParams.CRCOnlyDPOSHeight-1 {
		return a.GetArbitrators()
	}
	return a.GetCRCArbiters()
}

func (a *arbitrators) GetCrossChainArbitersCount() int {
	if a.bestHeight() < a.chainParams.CRCOnlyDPOSHeight-1 {
		return len(a.chainParams.OriginArbiters)
	}

	return len(a.chainParams.CRCArbiters)
}

func (a *arbitrators) GetCrossChainArbitersMajorityCount() int {
	minSignCount := int(float64(a.GetCrossChainArbitersCount()) *
		MajoritySignRatioNumerator / MajoritySignRatioDenominator)
	return minSignCount
}

func (a *arbitrators) GetNextOnDutyArbitratorV(height, offset uint32) ArbiterMember {
	// main version is >= H1
	if height >= a.State.chainParams.CRCOnlyDPOSHeight {
		arbitrators := a.currentArbitrators
		if len(arbitrators) == 0 {
			return nil
		}
		index := (a.dutyIndex + int(offset)) % len(arbitrators)
		arbiter := arbitrators[index]

		return arbiter
	}

	// old version
	return a.getNextOnDutyArbitratorV0(height, offset)
}

func (a *arbitrators) GetArbitersCount() int {
	a.mtx.Lock()
	result := len(a.currentArbitrators)
	a.mtx.Unlock()
	return result
}

func (a *arbitrators) GetCRCArbitersCount() int {
	a.mtx.Lock()
	result := len(a.crcArbiters)
	a.mtx.Unlock()
	return result
}

func (a *arbitrators) GetArbitersMajorityCount() int {
	a.mtx.Lock()
	minSignCount := int(float64(len(a.currentArbitrators)) *
		MajoritySignRatioNumerator / MajoritySignRatioDenominator)
	a.mtx.Unlock()
	return minSignCount
}

func (a *arbitrators) HasArbitersMajorityCount(num int) bool {
	return num > a.GetArbitersMajorityCount()
}

func (a *arbitrators) HasArbitersMinorityCount(num int) bool {
	a.mtx.Lock()
	count := len(a.currentArbitrators)
	a.mtx.Unlock()
	return num >= count-a.GetArbitersMajorityCount()
}

func (a *arbitrators) getChangeType(height uint32) (ChangeType, uint32) {

	// special change points:
	//		H1 - PreConnectOffset -> 	[updateNext, H1]: update next arbiters and let CRC arbiters prepare to connect
	//		H1 -> 						[normalChange, H1]: should change to new election (that only have CRC arbiters)
	//		H2 - PreConnectOffset -> 	[updateNext, H2]: update next arbiters and let normal arbiters prepare to connect
	//		H2 -> 						[normalChange, H2]: should change to new election (arbiters will have both CRC and normal arbiters)
	if height == a.State.chainParams.CRCOnlyDPOSHeight-
		a.State.chainParams.PreConnectOffset {
		return updateNext, a.State.chainParams.CRCOnlyDPOSHeight
	} else if height == a.State.chainParams.CRCOnlyDPOSHeight {
		return normalChange, a.State.chainParams.CRCOnlyDPOSHeight
	} else if height == a.State.chainParams.PublicDPOSHeight-
		a.State.chainParams.PreConnectOffset {
		return updateNext, a.State.chainParams.PublicDPOSHeight
	} else if height == a.State.chainParams.PublicDPOSHeight {
		return normalChange, a.State.chainParams.PublicDPOSHeight
	}

	// main version >= H2
	if height > a.State.chainParams.PublicDPOSHeight &&
		a.dutyIndex == len(a.currentArbitrators)-1 {
		return normalChange, height
	}

	return none, height
}

func (a *arbitrators) changeCurrentArbitrators() error {
	a.currentArbitrators = a.nextArbitrators
	a.currentCandidates = a.nextCandidates
	a.CurrentReward = a.NextReward

	sort.Slice(a.currentArbitrators, func(i, j int) bool {
		return bytes.Compare(a.currentArbitrators[i].GetNodePublicKey(),
			a.currentArbitrators[j].GetNodePublicKey()) < 0
	})

	a.dutyIndex = 0
	return nil
}

func (a *arbitrators) updateNextArbitrators(height uint32) error {
	_, recover := a.InactiveModeSwitch(height, a.IsAbleToRecoverFromInactiveMode)
	if recover {
		a.LeaveEmergency()
	} else {
		a.TryLeaveUnderStaffed(a.IsAbleToRecoverFromUnderstaffedState)
	}

	err := a.resetNextArbiterByCRC(height)
	if err != nil {
		return err
	}

	if !a.IsInactiveMode() && !a.IsUnderstaffedMode() {
		count := a.chainParams.GeneralArbiters
		votedProducers := a.State.GetVotedProducers()
		sort.Slice(votedProducers, func(i, j int) bool {
			if votedProducers[i].votes == votedProducers[j].votes {
				return bytes.Compare(votedProducers[i].info.NodePublicKey,
					votedProducers[j].NodePublicKey()) < 0
			}
			return votedProducers[i].Votes() > votedProducers[j].Votes()
		})

		producers, err := a.GetNormalArbitratorsDesc(height, count,
			votedProducers)
		if err != nil {
			if err := a.tryHandleError(height, err); err != nil {
				return err
			}
			a.nextCandidates = make([]ArbiterMember, 0)
		} else {
			a.nextArbitrators = append(a.nextArbitrators, producers...)

			candidates, err := a.GetCandidatesDesc(height, count,
				votedProducers)
			if err != nil {
				return err
			}
			a.nextCandidates = candidates
		}
	} else {
		a.nextCandidates = make([]ArbiterMember, 0)
	}

	if err := a.snapshotVotesStates(); err != nil {
		return err
	}

	return nil
}

func (a *arbitrators) resetNextArbiterByCRC(height uint32) error {
	a.nextArbitrators = make([]ArbiterMember, 0)

	if height >= a.chainParams.CRCommitteeStartHeight {
		if a.crCommittee.LastCommitteeHeight > a.crcChangedHeight {
			crMembers := a.crCommittee.GetAllMembers()
			if len(crMembers) != len(a.chainParams.CRCArbiters) {
				return errors.New("CRC members count mismatch with CRC arbiters")
			}

			a.crcArbiters = map[common.Uint168]ArbiterMember{}
			for i, v := range a.chainParams.CRCArbiters {
				pk, err := common.HexStringToBytes(v)
				if err != nil {
					return err
				}
				ar, err := NewCRCArbiter(pk, crMembers[i])
				if err != nil {
					return err
				}
				a.crcArbiters[ar.GetOwnerProgramHash()] = ar
			}
			a.crcChangedHeight = a.crCommittee.LastCommitteeHeight
		}
	}

	for _, v := range a.crcArbiters {
		a.nextArbitrators = append(a.nextArbitrators, v)
	}

	return nil
}

func (a *arbitrators) GetCandidatesDesc(height uint32, startIndex int,
	producers []*Producer) ([]ArbiterMember, error) {
	// main version >= H2
	if height >= a.State.chainParams.PublicDPOSHeight {
		if len(producers) < startIndex {
			return make([]ArbiterMember, 0), nil
		}

		result := make([]ArbiterMember, 0)
		for i := startIndex; i < len(producers) && i < startIndex+a.
			chainParams.CandidateArbiters; i++ {
			ar, err := NewDPoSArbiter(DPoS, producers[i])
			if err != nil {
				return nil, err
			}
			result = append(result, ar)
		}
		return result, nil
	}

	// old version [0, H2)
	return nil, nil
}

func (a *arbitrators) GetNormalArbitratorsDesc(height uint32,
	arbitratorsCount int, producers []*Producer) ([]ArbiterMember, error) {
	// main version >= H2
	if height >= a.State.chainParams.PublicDPOSHeight {
		if len(producers) < arbitratorsCount {
			return nil, ErrInsufficientProducer
		}

		result := make([]ArbiterMember, 0)
		for i := 0; i < arbitratorsCount && i < len(producers); i++ {
			ar, err := NewDPoSArbiter(DPoS, producers[i])
			if err != nil {
				return nil, err
			}
			result = append(result, ar)
		}
		return result, nil
	}

	// version [H1, H2)
	if height >= a.State.chainParams.CRCOnlyDPOSHeight {
		return a.getNormalArbitratorsDescV1()
	}

	// version [0, H1)
	return a.getNormalArbitratorsDescV0()
}

func (a *arbitrators) snapshotVotesStates() error {
	a.NextReward.OwnerVotesInRound = make(map[common.Uint168]common.Fixed64, 0)
	a.NextReward.TotalVotesInRound = 0
	for _, ar := range a.nextArbitrators {
		if !a.isCRCArbitrator(ar.GetNodePublicKey()) {
			producer := a.GetProducer(ar.GetNodePublicKey())
			if producer == nil {
				return errors.New("get producer by node public key failed")
			}
			programHash, err := contract.PublicKeyToStandardProgramHash(
				producer.OwnerPublicKey())
			if err != nil {
				return err
			}
			a.NextReward.OwnerVotesInRound[*programHash] = producer.Votes()
			a.NextReward.TotalVotesInRound += producer.Votes()
		}
	}

	for _, ar := range a.nextCandidates {
		if a.isCRCArbitrator(ar.GetNodePublicKey()) {
			continue
		}
		producer := a.GetProducer(ar.GetNodePublicKey())
		if producer == nil {
			return errors.New("get producer by node public key failed")
		}
		programHash, err := contract.PublicKeyToStandardProgramHash(producer.OwnerPublicKey())
		if err != nil {
			return err
		}
		a.NextReward.OwnerVotesInRound[*programHash] = producer.Votes()
		a.NextReward.TotalVotesInRound += producer.Votes()
	}

	return nil
}

func (a *arbitrators) DumpInfo(height uint32) {
	a.mtx.Lock()
	defer a.mtx.Unlock()

	a.dumpInfo(height)
}

func (a *arbitrators) dumpInfo(height uint32) {
	var printer func(string, ...interface{})
	changeType, _ := a.getChangeType(height + 1)
	switch changeType {
	case updateNext:
		fallthrough
	case normalChange:
		printer = log.Infof
	case none:
		printer = log.Debugf
	}

	var crInfo string
	crParams := make([]interface{}, 0)
	if len(a.currentArbitrators) != 0 {
		crInfo, crParams = getArbitersInfoWithOnduty("CURRENT ARBITERS",
			a.currentArbitrators, a.dutyIndex, a.GetOnDutyArbitrator())
	} else {
		crInfo, crParams = getArbitersInfoWithoutOnduty("CURRENT ARBITERS", a.currentArbitrators)
	}
	nrInfo, nrParams := getArbitersInfoWithoutOnduty("NEXT ARBITERS", a.nextArbitrators)
	ccInfo, ccParams := getArbitersInfoWithoutOnduty("CURRENT CANDIDATES", a.currentCandidates)
	ncInfo, ncParams := getArbitersInfoWithoutOnduty("NEXT CANDIDATES", a.nextCandidates)
	printer(crInfo+nrInfo+ccInfo+ncInfo, append(append(append(crParams, nrParams...), ccParams...), ncParams...)...)
}

func (a *arbitrators) getBlockDPOSReward(block *types.Block) common.Fixed64 {
	totalTxFx := common.Fixed64(0)
	for _, tx := range block.Transactions {
		totalTxFx += tx.Fee
	}

	return common.Fixed64(math.Ceil(float64(totalTxFx+
		a.chainParams.RewardPerBlock) * 0.35))
}

func (a *arbitrators) newCheckPoint(height uint32) *CheckPoint {
	point := &CheckPoint{
		Height:                     height,
		DutyIndex:                  a.dutyIndex,
		CurrentCandidates:          make([]ArbiterMember, 0),
		NextArbitrators:            make([]ArbiterMember, 0),
		NextCandidates:             make([]ArbiterMember, 0),
		CurrentReward:              *NewRewardData(),
		NextReward:                 *NewRewardData(),
		accumulativeReward:         a.accumulativeReward,
		finalRoundChange:           a.finalRoundChange,
		clearingHeight:             a.clearingHeight,
		arbitersRoundReward:        make(map[common.Uint168]common.Fixed64),
		illegalBlocksPayloadHashes: make(map[common.Uint256]interface{}),
		CurrentArbitrators:         a.currentArbitrators,
		StateKeyFrame:              *a.State.snapshot(),
	}
	point.CurrentArbitrators = copyByteList(a.currentArbitrators)
	point.CurrentCandidates = copyByteList(a.currentCandidates)
	point.NextArbitrators = copyByteList(a.nextArbitrators)
	point.NextCandidates = copyByteList(a.nextCandidates)
	point.CurrentReward = *copyReward(&a.CurrentReward)
	point.NextReward = *copyReward(&a.NextReward)
	for k, v := range a.arbitersRoundReward {
		point.arbitersRoundReward[k] = v
	}
	for k := range a.illegalBlocksPayloadHashes {
		point.illegalBlocksPayloadHashes[k] = nil
	}

	return point
}

func (a *arbitrators) snapshot(height uint32) {
	var frames []*CheckPoint
	if v, ok := a.snapshots[height]; ok {
		frames = v
	} else {
		// remove the oldest keys if snapshot capacity is over
		if len(a.snapshotKeysDesc) >= MaxSnapshotLength {
			for i := MaxSnapshotLength - 1; i < len(a.snapshotKeysDesc); i++ {
				delete(a.snapshots, a.snapshotKeysDesc[i])
			}
			a.snapshotKeysDesc = a.snapshotKeysDesc[0 : MaxSnapshotLength-1]
		}

		a.snapshotKeysDesc = append(a.snapshotKeysDesc, height)
		sort.Slice(a.snapshotKeysDesc, func(i, j int) bool {
			return a.snapshotKeysDesc[i] > a.snapshotKeysDesc[j]
		})
	}
	checkpoint := a.newCheckPoint(height)
	frames = append(frames, checkpoint)
	a.snapshots[height] = frames
}

func (a *arbitrators) GetSnapshot(height uint32) []*CheckPoint {
	a.mtx.Lock()
	defer a.mtx.Unlock()

	if height > a.bestHeight() {
		return []*CheckPoint{
			{
				CurrentArbitrators: a.currentArbitrators,
			},
		}
	} else {
		return a.getSnapshot(height)
	}
}

func (a *arbitrators) getSnapshot(height uint32) []*CheckPoint {
	result := make([]*CheckPoint, 0)
	if height >= a.snapshotKeysDesc[len(a.snapshotKeysDesc)-1] {
		// if height is in range of snapshotKeysDesc, get the key with the same
		// election as height
		key := a.snapshotKeysDesc[0]
		for i := 1; i < len(a.snapshotKeysDesc); i++ {
			if height >= a.snapshotKeysDesc[i] &&
				height < a.snapshotKeysDesc[i-1] {
				key = a.snapshotKeysDesc[i]
			}
		}

		return a.snapshots[key]
	}
	return result
}

func getArbitersInfoWithOnduty(title string, arbiters []ArbiterMember,
	dutyIndex int, ondutyArbiter []byte) (string, []interface{}) {
	info := "\n" + title + "\nDUTYINDEX: %d\n%5s %66s %6s \n----- " +
		strings.Repeat("-", 66) + " ------\n"
	params := make([]interface{}, 0)
	params = append(params, (dutyIndex+1)%len(arbiters))
	params = append(params, "INDEX", "PUBLICKEY", "ONDUTY")
	for i, arbiter := range arbiters {
		info += "%-5d %-66s %6t\n"
		publicKey := common.BytesToHexString(arbiter.GetNodePublicKey())
		params = append(params, i+1, publicKey, bytes.Equal(
			arbiter.GetNodePublicKey(), ondutyArbiter))
	}
	info += "----- " + strings.Repeat("-", 66) + " ------"
	return info, params
}

func getArbitersInfoWithoutOnduty(title string,
	arbiters []ArbiterMember) (string, []interface{}) {

	info := "\n" + title + "\n%5s %66s\n----- " + strings.Repeat("-", 66) + "\n"
	params := make([]interface{}, 0)
	params = append(params, "INDEX", "PUBLICKEY")
	for i, arbiter := range arbiters {
		info += "%-5d %-66s\n"
		publicKey := common.BytesToHexString(arbiter.GetNodePublicKey())
		params = append(params, i+1, publicKey)
	}
	info += "----- " + strings.Repeat("-", 66)
	return info, params
}

func (a *arbitrators) initArbitrators(chainParams *config.Params) error {
	originArbiters := make([]ArbiterMember, len(chainParams.OriginArbiters))
	for i, arbiter := range chainParams.OriginArbiters {
		b, err := common.HexStringToBytes(arbiter)
		if err != nil {
			return err
		}
		ar, err := NewOriginArbiter(Origin, b)
		if err != nil {
			return err
		}

		originArbiters[i] = ar
	}

	crcArbiters := make(map[common.Uint168]ArbiterMember)
	for _, pk := range chainParams.CRCArbiters {
		pubKey, err := hex.DecodeString(pk)
		if err != nil {
			return err
		}
		producer := &Producer{ // here need crc NODE public key
			info: payload.ProducerInfo{
				OwnerPublicKey: pubKey,
				NodePublicKey:  pubKey,
			},
			activateRequestHeight: math.MaxUint32,
		}
		ar, err := NewDPoSArbiter(CROrigin, producer)
		if err != nil {
			return err
		}
		crcArbiters[ar.GetOwnerProgramHash()] = ar
	}

	a.currentArbitrators = originArbiters
	a.nextArbitrators = originArbiters
	a.crcArbiters = crcArbiters
	a.CurrentReward = RewardData{
		OwnerVotesInRound: make(map[common.Uint168]common.Fixed64),
		TotalVotesInRound: 0,
	}
	a.NextReward = RewardData{
		OwnerVotesInRound: make(map[common.Uint168]common.Fixed64),
		TotalVotesInRound: 0,
	}
	return nil
}

func NewArbitrators(chainParams *config.Params, committee *state.Committee,
	getProducerDepositAmount func(common.Uint168) (common.Fixed64, error)) (
	*arbitrators, error) {
	a := &arbitrators{
		chainParams:                chainParams,
		crCommittee:                committee,
		nextCandidates:             make([]ArbiterMember, 0),
		accumulativeReward:         common.Fixed64(0),
		finalRoundChange:           common.Fixed64(0),
		arbitersRoundReward:        nil,
		illegalBlocksPayloadHashes: make(map[common.Uint256]interface{}),
		snapshots:                  make(map[uint32][]*CheckPoint),
		snapshotKeysDesc:           make([]uint32, 0),
		crcChangedHeight:           0,
		degradation: &degradation{
			inactiveTxs:       make(map[common.Uint256]interface{}),
			inactivateHeight:  0,
			understaffedSince: 0,
			state:             DSNormal,
		},
	}
	if err := a.initArbitrators(chainParams); err != nil {
		return nil, err
	}
	a.State = NewState(chainParams, a.GetArbitrators,
		getProducerDepositAmount)

	chainParams.CkpManager.Register(NewCheckpoint(a))
	return a, nil
}
