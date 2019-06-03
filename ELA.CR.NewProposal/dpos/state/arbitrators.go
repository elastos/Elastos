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

	// CheckPointInterval defines interval height between two neighbor check
	// points
	CheckPointInterval = uint32(720)

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
	*KeyFrame
	store            IArbitratorsRecord
	chainParams      *config.Params
	bestHeight       func() uint32
	bestBlock        func() (*types.Block, error)
	getBlockByHeight func(uint32) (*types.Block, error)

	mtx               sync.Mutex
	started           bool
	dutyIndex         int
	currentCandidates [][]byte

	CurrentReward RewardData
	NextReward    RewardData

	nextArbitrators             [][]byte
	nextCandidates              [][]byte
	crcArbiters                 [][]byte
	crcArbitratorsProgramHashes map[common.Uint168]interface{}
	crcArbitratorsNodePublicKey map[string]*Producer
	accumulativeReward          common.Fixed64
	finalRoundChange            common.Fixed64
	clearingHeight              uint32
	arbitersRoundReward         map[common.Uint168]common.Fixed64
	illegalBlocksPayloadHashes  map[common.Uint256]interface{}

	snapshots            map[uint32][]*KeyFrame
	snapshotKeysDesc     []uint32
	lastCheckPointHeight uint32
}

func (a *arbitrators) Start() {
	a.mtx.Lock()
	a.started = true
	a.mtx.Unlock()
}

func (a *arbitrators) RecoverFromCheckPoints(height uint32) (uint32, error) {
	if a.store == nil {
		return 0, errors.New("can't find dpos store object")
	}
	point, err := a.store.GetCheckPoint(height)
	if err != nil {
		return 0, err
	}

	a.mtx.Lock()
	a.dutyIndex = point.DutyIndex
	a.CurrentArbitrators = point.CurrentArbitrators
	a.currentCandidates = point.CurrentCandidates
	a.nextArbitrators = point.NextArbitrators
	a.nextCandidates = point.NextCandidates
	a.CurrentReward = point.CurrentReward
	a.NextReward = point.NextReward
	a.StateKeyFrame = &point.StateKeyFrame
	a.mtx.Unlock()

	return point.Height, err
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
	if height > a.history.height {
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
		index = a.dutyIndex % len(a.CurrentArbitrators)
	} else {
		index = int(height) % len(a.CurrentArbitrators)
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

	block, err := a.getBlockByHeight(height)
	if err != nil {
		block, err = a.bestBlock()
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

	a.mtx.Unlock()

	if a.started {
		go events.Notify(events.ETDirectPeersChanged,
			a.GetNeedConnectArbiters())
	}

	a.DumpInfo(height)

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

func (a *arbitrators) NormalChange(height uint32) error {
	a.snapshot(height)

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
		if err := a.NormalChange(block.Height); err != nil {
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
		events.Notify(events.ETDirectPeersChanged, a.GetNeedConnectArbiters())
	}
}

func (a *arbitrators) accumulateReward(block *types.Block) {
	if block.Height < a.State.chainParams.PublicDPOSHeight {
		return
	}

	dposReward := a.getBlockDPOSReward(block)
	a.accumulativeReward += dposReward

	a.arbitersRoundReward = nil
	a.finalRoundChange = 0
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

	if err := a.distributeDPOSReward(a.accumulativeReward); err != nil {
		return err
	}
	a.accumulativeReward = dposReward
	a.clearingHeight = block.Height

	return nil
}

func (a *arbitrators) distributeDPOSReward(reward common.Fixed64) (err error) {
	a.arbitersRoundReward = map[common.Uint168]common.Fixed64{}

	a.arbitersRoundReward[a.chainParams.CRCAddress] = 0
	realDPOSReward, err := a.distributeWithNormalArbitrators(reward)

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

func (a *arbitrators) distributeWithNormalArbitrators(
	reward common.Fixed64) (common.Fixed64, error) {
	ownerHashes := a.CurrentReward.OwnerProgramHashes
	if len(ownerHashes) == 0 {
		return 0, errors.New("not found arbiters when distributeWithNormalArbitrators")
	}

	totalBlockConfirmReward := float64(reward) * 0.25
	totalTopProducersReward := float64(reward) - totalBlockConfirmReward
	individualBlockConfirmReward := common.Fixed64(
		math.Floor(totalBlockConfirmReward / float64(len(ownerHashes))))
	totalVotesInRound := a.CurrentReward.TotalVotesInRound
	if len(a.chainParams.CRCArbiters) == len(a.CurrentArbitrators) {
		a.arbitersRoundReward[a.chainParams.CRCAddress] = reward
		return reward, nil
	}
	rewardPerVote := totalTopProducersReward / float64(totalVotesInRound)

	realDPOSReward := common.Fixed64(0)
	for _, ownerHash := range ownerHashes {
		votes := a.CurrentReward.OwnerVotesInRound[*ownerHash]
		individualProducerReward := common.Fixed64(math.Floor(float64(
			votes) * rewardPerVote))
		r := individualBlockConfirmReward + individualProducerReward
		if _, ok := a.crcArbitratorsProgramHashes[*ownerHash]; ok {
			r = individualBlockConfirmReward
			a.arbitersRoundReward[a.chainParams.CRCAddress] += r
		} else {
			a.arbitersRoundReward[*ownerHash] = r
		}

		realDPOSReward += r
	}
	candidateOwnerHashes := a.CurrentReward.CandidateOwnerProgramHashes
	for _, ownerHash := range candidateOwnerHashes {
		votes := a.CurrentReward.OwnerVotesInRound[*ownerHash]
		individualProducerReward := common.Fixed64(math.Floor(float64(
			votes) * rewardPerVote))
		a.arbitersRoundReward[*ownerHash] = individualProducerReward

		realDPOSReward += individualProducerReward
	}
	return realDPOSReward, nil
}

func (a *arbitrators) DecreaseChainHeight(height uint32) error {
	a.degradation.RollbackTo(height)

	heightOffset := int(a.history.height - height)
	if a.dutyIndex == 0 || a.dutyIndex < heightOffset {
		if err := a.ForceChange(height); err != nil {
			return err
		}

		a.dutyIndex = a.GetArbitersCount() - heightOffset + a.dutyIndex
	} else {
		a.dutyIndex = a.dutyIndex - heightOffset
	}

	return nil
}

func (a *arbitrators) GetNeedConnectArbiters() []peer.PID {
	a.mtx.Lock()
	defer a.mtx.Unlock()

	height := a.history.height + 1
	if height < a.chainParams.CRCOnlyDPOSHeight-a.chainParams.PreConnectOffset {
		return nil
	}

	pids := make(map[string]peer.PID)
	for k, p := range a.crcArbitratorsNodePublicKey {
		var pid peer.PID
		copy(pid[:], p.NodePublicKey())
		pids[k] = pid
	}

	for _, v := range a.CurrentArbitrators {
		key := common.BytesToHexString(v)
		var pid peer.PID
		copy(pid[:], v)
		pids[key] = pid
	}

	for _, v := range a.nextArbitrators {
		key := common.BytesToHexString(v)
		var pid peer.PID
		copy(pid[:], v)
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
	result := a.CurrentArbitrators
	a.mtx.Unlock()

	return result
}

func (a *arbitrators) GetCandidates() [][]byte {
	a.mtx.Lock()
	result := a.currentCandidates
	a.mtx.Unlock()

	return result
}

func (a *arbitrators) GetNextArbitrators() [][]byte {
	a.mtx.Lock()
	result := a.nextArbitrators
	a.mtx.Unlock()

	return result
}

func (a *arbitrators) GetNextCandidates() [][]byte {
	a.mtx.Lock()
	result := a.nextCandidates
	a.mtx.Unlock()

	return result
}

func (a *arbitrators) GetCRCArbiters() [][]byte {
	a.mtx.Lock()
	result := a.crcArbiters
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
	// there is no need to lock because crc related variable is read only and
	// initialized at the very first
	_, ok := a.crcArbitratorsNodePublicKey[hex.EncodeToString(pk)]
	return ok
}

func (a *arbitrators) IsActiveProducer(pk []byte) bool {
	return a.State.IsActiveProducer(pk)
}

func (a *arbitrators) IsDisabledProducer(pk []byte) bool {
	return a.State.IsInactiveProducer(pk) || a.State.IsIllegalProducer(pk) || a.State.IsCanceledProducer(pk)
}

func (a *arbitrators) GetCRCProducer(publicKey []byte) *Producer {
	a.mtx.Lock()
	defer a.mtx.Unlock()

	key := hex.EncodeToString(publicKey)
	if producer, ok := a.crcArbitratorsNodePublicKey[key]; ok {
		return producer
	}
	return nil
}

func (a *arbitrators) GetCRCArbitrators() map[string]*Producer {
	return a.crcArbitratorsNodePublicKey
}

func (a *arbitrators) GetOnDutyArbitrator() []byte {
	return a.GetNextOnDutyArbitratorV(a.bestHeight()+1, 0)
}

func (a *arbitrators) GetNextOnDutyArbitrator(offset uint32) []byte {
	return a.GetNextOnDutyArbitratorV(a.bestHeight()+1, offset)
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

func (a *arbitrators) GetNextOnDutyArbitratorV(height, offset uint32) []byte {
	// main version is >= H1
	if height >= a.State.chainParams.CRCOnlyDPOSHeight {
		arbitrators := a.CurrentArbitrators
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
	result := len(a.CurrentArbitrators)
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
	minSignCount := int(float64(len(a.CurrentArbitrators)) *
		MajoritySignRatioNumerator / MajoritySignRatioDenominator)
	a.mtx.Unlock()
	return minSignCount
}

func (a *arbitrators) HasArbitersMajorityCount(num int) bool {
	return num > a.GetArbitersMajorityCount()
}

func (a *arbitrators) HasArbitersMinorityCount(num int) bool {
	a.mtx.Lock()
	count := len(a.CurrentArbitrators)
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
		a.dutyIndex == len(a.CurrentArbitrators)-1 {
		return normalChange, height
	}

	return none, height
}

func (a *arbitrators) changeCurrentArbitrators() error {
	a.CurrentArbitrators = a.nextArbitrators
	a.currentCandidates = a.nextCandidates
	a.CurrentReward = a.NextReward

	sort.Slice(a.CurrentArbitrators, func(i, j int) bool {
		return bytes.Compare(a.CurrentArbitrators[i], a.CurrentArbitrators[j]) < 0
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

	a.nextArbitrators = make([][]byte, 0)
	for _, v := range a.crcArbitratorsNodePublicKey {
		a.nextArbitrators = append(a.nextArbitrators, v.info.NodePublicKey)
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
			a.nextCandidates = make([][]byte, 0)
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
		a.nextCandidates = make([][]byte, 0)
	}

	if err := a.snapshotVotesStates(); err != nil {
		return err
	}
	if err := a.updateNextOwnerProgramHashes(); err != nil {
		return err
	}

	return nil
}

func (a *arbitrators) GetCandidatesDesc(height uint32, startIndex int,
	producers []*Producer) ([][]byte, error) {
	// main version >= H2
	if height >= a.State.chainParams.PublicDPOSHeight {
		if len(producers) < startIndex {
			return make([][]byte, 0), nil
		}

		result := make([][]byte, 0)
		for i := startIndex; i < len(producers) && i < startIndex+a.
			chainParams.CandidateArbiters; i++ {
			result = append(result, producers[i].NodePublicKey())
		}
		return result, nil
	}

	// old version [0, H2)
	return nil, nil
}

func (a *arbitrators) GetNormalArbitratorsDesc(height uint32,
	arbitratorsCount int, producers []*Producer) ([][]byte, error) {
	// main version >= H2
	if height >= a.State.chainParams.PublicDPOSHeight {
		if len(producers) < arbitratorsCount {
			return nil, ErrInsufficientProducer
		}

		result := make([][]byte, 0)
		for i := 0; i < arbitratorsCount && i < len(producers); i++ {
			result = append(result, producers[i].NodePublicKey())
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
	for _, nodePublicKey := range a.nextArbitrators {
		if !a.IsCRCArbitrator(nodePublicKey) {
			producer := a.GetProducer(nodePublicKey)
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

	for _, nodePublicKey := range a.nextCandidates {
		if a.IsCRCArbitrator(nodePublicKey) {
			continue
		}
		producer := a.GetProducer(nodePublicKey)
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

func (a *arbitrators) updateNextOwnerProgramHashes() error {
	a.NextReward.OwnerProgramHashes = make([]*common.Uint168, 0)
	for _, nodePublicKey := range a.nextArbitrators {
		if a.IsCRCArbitrator(nodePublicKey) {
			ownerPublicKey := nodePublicKey // crc node public key is its owner public key for now
			programHash, err := contract.PublicKeyToStandardProgramHash(ownerPublicKey)
			if err != nil {
				return err
			}
			a.NextReward.OwnerProgramHashes = append(
				a.NextReward.OwnerProgramHashes, programHash)
		} else {
			producer := a.GetProducer(nodePublicKey)
			if producer == nil {
				return errors.New("get producer by node public key failed")
			}
			ownerPublicKey := producer.OwnerPublicKey()
			programHash, err := contract.PublicKeyToStandardProgramHash(ownerPublicKey)
			if err != nil {
				return err
			}
			a.NextReward.OwnerProgramHashes = append(
				a.NextReward.OwnerProgramHashes, programHash)
		}
	}

	a.NextReward.CandidateOwnerProgramHashes = make([]*common.Uint168, 0)
	for _, nodePublicKey := range a.nextCandidates {
		if a.IsCRCArbitrator(nodePublicKey) {
			continue
		}
		producer := a.GetProducer(nodePublicKey)
		if producer == nil {
			return errors.New("get producer by node public key failed")
		}
		programHash, err := contract.PublicKeyToStandardProgramHash(producer.OwnerPublicKey())
		if err != nil {
			return err
		}
		a.NextReward.CandidateOwnerProgramHashes = append(
			a.NextReward.CandidateOwnerProgramHashes, programHash)
	}

	return nil
}

func (a *arbitrators) DumpInfo(height uint32) {
	a.mtx.Lock()
	defer a.mtx.Unlock()

	var printer func(string, ...interface{})
	changeType, _ := a.getChangeType(height)
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
	if len(a.CurrentArbitrators) != 0 {
		crInfo, crParams = getArbitersInfoWithOnduty("CURRENT ARBITERS",
			a.CurrentArbitrators, a.dutyIndex, a.GetOnDutyArbitrator())
	} else {
		crInfo, crParams = getArbitersInfoWithoutOnduty("CURRENT ARBITERS", a.CurrentArbitrators)
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

func (a *arbitrators) trySaveCheckPoint(height uint32) error {
	if a.store == nil || height < a.lastCheckPointHeight+CheckPointInterval {
		return nil
	}

	point := &CheckPoint{
		Height:            height,
		DutyIndex:         a.dutyIndex,
		CurrentCandidates: make([][]byte, 0),
		NextArbitrators:   make([][]byte, 0),
		NextCandidates:    make([][]byte, 0),
		CurrentReward:     *NewRewardData(),
		NextReward:        *NewRewardData(),
		KeyFrame: KeyFrame{
			CurrentArbitrators: a.CurrentArbitrators,
		},
		StateKeyFrame: *a.State.snapshot(),
	}
	point.CurrentArbitrators = copyByteList(a.CurrentArbitrators)
	point.CurrentCandidates = copyByteList(a.currentCandidates)
	point.NextArbitrators = copyByteList(a.nextArbitrators)
	point.NextCandidates = copyByteList(a.nextCandidates)
	point.CurrentReward = *copyReward(&a.CurrentReward)
	point.NextReward = *copyReward(&a.NextReward)

	if err := a.store.SaveArbitersState(point); err != nil {
		return err
	}

	a.lastCheckPointHeight = height
	return nil
}

func (a *arbitrators) snapshot(height uint32) {
	var frames []*KeyFrame
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
	f := &KeyFrame{CurrentArbitrators: make([][]byte, 0)}
	f.CurrentArbitrators = copyByteList(a.CurrentArbitrators)
	frames = append(frames, f)
	a.snapshots[height] = frames

	if err := a.trySaveCheckPoint(height); err != nil {
		log.Warn("[snapshot] save check point err: ", err)
	}
}

func (a *arbitrators) GetSnapshot(height uint32) (result []*KeyFrame) {
	a.mtx.Lock()
	if height > a.bestHeight() {
		// if height is larger than first snapshot then return current key frame
		result = append(result, a.KeyFrame)
	} else if height >= a.snapshotKeysDesc[len(a.snapshotKeysDesc)-1] {
		// if height is in range of snapshotKeysDesc, get the key with the same
		// election as height
		key := a.snapshotKeysDesc[0]
		for i := 1; i < len(a.snapshotKeysDesc); i++ {
			if height >= a.snapshotKeysDesc[i] &&
				height < a.snapshotKeysDesc[i-1] {
				key = a.snapshotKeysDesc[i]
			}
		}

		result = a.snapshots[key]
	}
	a.mtx.Unlock()

	return result
}

func getArbitersInfoWithOnduty(title string, arbiters [][]byte,
	dutyIndex int, ondutyArbiter []byte) (string, []interface{}) {
	info := "\n" + title + "\nDUTYINDEX: %d\n%5s %66s %6s \n----- " +
		strings.Repeat("-", 66) + " ------\n"
	params := make([]interface{}, 0)
	params = append(params, (dutyIndex+1)%len(arbiters))
	params = append(params, "INDEX", "PUBLICKEY", "ONDUTY")
	for i, arbiter := range arbiters {
		info += "%-5d %-66s %6t\n"
		publicKey := common.BytesToHexString(arbiter)
		params = append(params, i+1, publicKey, bytes.Equal(arbiter, ondutyArbiter))
	}
	info += "----- " + strings.Repeat("-", 66) + " ------"
	return info, params
}

func getArbitersInfoWithoutOnduty(title string, arbiters [][]byte) (string, []interface{}) {

	info := "\n" + title + "\n%5s %66s\n----- " + strings.Repeat("-", 66) + "\n"
	params := make([]interface{}, 0)
	params = append(params, "INDEX", "PUBLICKEY")
	for i, arbiter := range arbiters {
		info += "%-5d %-66s\n"
		publicKey := common.BytesToHexString(arbiter)
		params = append(params, i+1, publicKey)
	}
	info += "----- " + strings.Repeat("-", 66)
	return info, params
}

func NewArbitrators(chainParams *config.Params,
	store IArbitratorsRecord,
	bestHeight func() uint32,
	bestBlock func() (*types.Block, error),
	getBlockByHeight func(uint32) (*types.Block, error)) (*arbitrators, error) {

	originArbiters := make([][]byte, len(chainParams.OriginArbiters))
	originArbitersProgramHashes := make([]*common.Uint168, len(chainParams.OriginArbiters))
	for i, arbiter := range chainParams.OriginArbiters {
		a, err := common.HexStringToBytes(arbiter)
		if err != nil {
			return nil, err
		}
		originArbiters[i] = a

		hash, err := contract.PublicKeyToStandardProgramHash(a)
		if err != nil {
			return nil, err
		}
		originArbitersProgramHashes[i] = hash
	}

	crcNodeMap := make(map[string]*Producer)
	crcArbitratorsProgramHashes := make(map[common.Uint168]interface{})
	crcArbiters := make([][]byte, 0, len(chainParams.CRCArbiters))
	for _, pk := range chainParams.CRCArbiters {
		pubKey, err := hex.DecodeString(pk)
		if err != nil {
			return nil, err
		}
		hash, err := contract.PublicKeyToStandardProgramHash(pubKey)
		if err != nil {
			return nil, err
		}
		crcArbiters = append(crcArbiters, pubKey)
		crcArbitratorsProgramHashes[*hash] = nil
		crcNodeMap[pk] = &Producer{ // here need crc NODE public key
			info: payload.ProducerInfo{
				OwnerPublicKey: pubKey,
				NodePublicKey:  pubKey,
			},
			activateRequestHeight: math.MaxUint32,
		}
	}

	a := &arbitrators{
		chainParams:                 chainParams,
		store:                       store,
		bestHeight:                  bestHeight,
		bestBlock:                   bestBlock,
		getBlockByHeight:            getBlockByHeight,
		nextArbitrators:             originArbiters,
		nextCandidates:              make([][]byte, 0),
		crcArbiters:                 crcArbiters,
		crcArbitratorsNodePublicKey: crcNodeMap,
		crcArbitratorsProgramHashes: crcArbitratorsProgramHashes,
		accumulativeReward:          common.Fixed64(0),
		finalRoundChange:            common.Fixed64(0),
		arbitersRoundReward:         nil,
		illegalBlocksPayloadHashes:  make(map[common.Uint256]interface{}),
		snapshots:                   make(map[uint32][]*KeyFrame),
		snapshotKeysDesc:            make([]uint32, 0),
		KeyFrame: &KeyFrame{
			CurrentArbitrators: originArbiters,
		},
		CurrentReward: RewardData{
			OwnerProgramHashes:          originArbitersProgramHashes,
			CandidateOwnerProgramHashes: make([]*common.Uint168, 0),
			OwnerVotesInRound:           make(map[common.Uint168]common.Fixed64),
			TotalVotesInRound:           0,
		},
		NextReward: RewardData{
			OwnerProgramHashes:          originArbitersProgramHashes,
			CandidateOwnerProgramHashes: make([]*common.Uint168, 0),
			OwnerVotesInRound:           make(map[common.Uint168]common.Fixed64),
			TotalVotesInRound:           0,
		},
		degradation: &degradation{
			inactiveTxs:       make(map[common.Uint256]interface{}),
			inactivateHeight:  0,
			understaffedSince: 0,
			state:             DSNormal,
		},
	}
	a.State = NewState(chainParams, a.GetArbitrators)

	if store != nil {
		checkedHeights, err := store.GetHeightsDesc()
		if err == nil && len(checkedHeights) > 0 {
			a.lastCheckPointHeight = checkedHeights[0]
		}
	}

	return a, nil
}
