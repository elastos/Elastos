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
	"github.com/elastos/Elastos.ELA/dpos/p2p"
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

	none         = ChangeType(0x00)
	updateNext   = ChangeType(0x01)
	normalChange = ChangeType(0x02)
)

type arbitrators struct {
	*State
	chainParams   *config.Params
	bestHeight    func() uint32
	bestBlock     func() (*types.Block, error)
	arbitersCount int

	mtx                sync.Mutex
	started            bool
	dutyIndex          int
	currentArbitrators [][]byte
	currentCandidates  [][]byte

	currentOwnerProgramHashes   []*common.Uint168
	candidateOwnerProgramHashes []*common.Uint168
	ownerVotesInRound           map[common.Uint168]common.Fixed64
	totalVotesInRound           common.Fixed64

	nextArbitrators             [][]byte
	nextCandidates              [][]byte
	crcArbiters                 [][]byte
	crcArbitratorsProgramHashes map[common.Uint168]interface{}
	crcArbitratorsNodePublicKey map[string]*Producer
	accumulativeReward          common.Fixed64
	finalRoundChange            common.Fixed64
	arbitersRoundReward         map[common.Uint168]common.Fixed64
	illegalBlocksPayloadHashes  map[common.Uint256]interface{}
}

func (a *arbitrators) Start() {
	a.mtx.Lock()
	a.started = true
	a.mtx.Unlock()
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
	default:
		return errors.New("[ProcessSpecialTxPayload] invalid payload type")
	}

	a.State.ProcessSpecialTxPayload(p, height)
	return a.ForceChange(height)
}

func (a *arbitrators) RollbackTo(height uint32) error {
	if err := a.State.RollbackTo(height); err != nil {
		return err
	}
	a.DecreaseChainHeight(height)
	return nil
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

	block, err := a.bestBlock()
	if err != nil {
		return err
	}

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

	return nil
}

func (a *arbitrators) NormalChange(height uint32) error {
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
			log.Error("[IncreaseChainHeight] update next arbiters error: ", err)
		}
	case normalChange:
		if err := a.clearingDPOSReward(block, true); err != nil {
			panic(fmt.Sprintf("normal change fail when clear DPOS reward: "+
				" transaction, height: %d", block.Height))
		}
		if err := a.NormalChange(block.Height); err != nil {
			panic(fmt.Sprintf("normal change fail when finding an"+
				" inactive arbitrators transaction, height: %d", block.Height))
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
	if block.Height <= a.State.chainParams.PublicDPOSHeight {
		return
	}

	dposReward := a.getBlockDPOSReward(block)
	a.accumulativeReward += dposReward

	a.arbitersRoundReward = nil
	a.finalRoundChange = 0
}

func (a *arbitrators) clearingDPOSReward(block *types.Block,
	smoothClearing bool) error {

	dposReward := a.getBlockDPOSReward(block)
	if block.Height+1 <= a.State.chainParams.PublicDPOSHeight {
		a.accumulativeReward = dposReward
		return nil
	}

	if smoothClearing {
		a.accumulativeReward += dposReward
		dposReward = 0
	}

	if err := a.distributeDPOSReward(a.accumulativeReward); err != nil {
		return err
	}
	a.accumulativeReward = dposReward

	return nil
}

func (a *arbitrators) distributeDPOSReward(reward common.Fixed64) error {
	a.arbitersRoundReward = map[common.Uint168]common.Fixed64{}

	ownerHashes := a.currentOwnerProgramHashes
	if len(ownerHashes) == 0 {
		return errors.New("not found arbiters when distributeDposReward")
	}
	candidateOwnerHashes := a.candidateOwnerProgramHashes

	totalBlockConfirmReward := float64(reward) * 0.25
	totalTopProducersReward := float64(reward) - totalBlockConfirmReward
	individualBlockConfirmReward := common.Fixed64(math.Floor(totalBlockConfirmReward / float64(len(ownerHashes))))
	totalVotesInRound := a.totalVotesInRound
	if totalVotesInRound == common.Fixed64(0) {
		panic("total votes in round equal 0")
	}
	rewardPerVote := totalTopProducersReward / float64(totalVotesInRound)

	a.arbitersRoundReward[a.chainParams.CRCAddress] = 0
	realDPOSReward := common.Fixed64(0)
	for _, ownerHash := range ownerHashes {
		votes := a.ownerVotesInRound[*ownerHash]
		individualProducerReward := common.Fixed64(float64(votes) * rewardPerVote)
		r := individualBlockConfirmReward + individualProducerReward
		if _, ok := a.crcArbitratorsProgramHashes[*ownerHash]; ok {
			r = individualBlockConfirmReward
			a.arbitersRoundReward[a.chainParams.CRCAddress] += r
		} else {
			a.arbitersRoundReward[*ownerHash] = r
		}

		realDPOSReward += r
	}

	for _, ownerHash := range candidateOwnerHashes {
		votes := a.ownerVotesInRound[*ownerHash]
		individualProducerReward := common.Fixed64(float64(votes) * rewardPerVote)
		a.arbitersRoundReward[*ownerHash] = individualProducerReward

		realDPOSReward += individualProducerReward
	}

	change := reward - realDPOSReward
	if change < 0 {
		return errors.New("real dpos reward more than reward limit")
	}

	a.finalRoundChange = change
	return nil
}

func (a *arbitrators) DecreaseChainHeight(height uint32) {
	a.mtx.Lock()
	defer a.mtx.Unlock()

	if a.dutyIndex == 0 {
		a.dutyIndex = len(a.currentArbitrators) - 1
	} else {
		a.dutyIndex--
	}
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

	for _, v := range a.currentArbitrators {
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
	result := a.currentArbitrators
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

func (a *arbitrators) IsCRCArbitratorNodePublicKey(nodePublicKeyHex string) bool {
	_, ok := a.crcArbitratorsNodePublicKey[nodePublicKeyHex]
	return ok
}

func (a *arbitrators) IsCRCArbitrator(pk []byte) bool {
	_, ok := a.crcArbitratorsNodePublicKey[hex.EncodeToString(pk)]
	return ok
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

func (a *arbitrators) GetNextOnDutyArbitratorV(height, offset uint32) []byte {
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
		a.dutyIndex == a.arbitersCount-1 {
		return normalChange, height
	}

	return none, height
}

func (a *arbitrators) changeCurrentArbitrators() error {
	a.currentArbitrators = a.nextArbitrators
	a.currentCandidates = a.nextCandidates

	sort.Slice(a.currentArbitrators, func(i, j int) bool {
		return bytes.Compare(a.currentArbitrators[i], a.currentArbitrators[j]) < 0
	})

	if err := a.updateOwnerProgramHashes(); err != nil {
		return err
	}

	a.dutyIndex = 0
	return nil
}

func (a *arbitrators) updateNextArbitrators(height uint32) error {
	var crcCount int
	a.nextArbitrators = make([][]byte, 0)
	for _, v := range a.crcArbitratorsNodePublicKey {
		if !a.isInactiveProducer(v.info.NodePublicKey) {
			a.nextArbitrators = append(a.nextArbitrators, v.info.NodePublicKey)
		} else {
			crcCount++
		}
	}
	count := a.chainParams.GeneralArbiters + crcCount
	producers, err := a.GetNormalArbitratorsDesc(height, count, a.State.getProducers())
	if err != nil {
		return err
	}
	for _, v := range producers {
		a.nextArbitrators = append(a.nextArbitrators, v)
	}

	candidates, err := a.GetCandidatesDesc(height, count, a.State.getProducers())
	if err != nil {
		return err
	}
	a.nextCandidates = candidates

	return nil
}

func (a *arbitrators) GetCandidatesDesc(height uint32, startIndex int,
	producers []*Producer) ([][]byte, error) {
	// main version >= H2
	if height >= a.State.chainParams.PublicDPOSHeight {
		if len(producers) < startIndex {
			return make([][]byte, 0), nil
		}

		sort.Slice(producers, func(i, j int) bool {
			return producers[i].Votes() > producers[j].Votes()
		})

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
		if len(producers) < arbitratorsCount/2+1 {
			return nil, errors.New("producers count less than min arbitrators count")
		}

		sort.Slice(producers, func(i, j int) bool {
			if producers[i].votes == producers[j].votes {
				return bytes.Compare(producers[i].info.NodePublicKey,
					producers[j].NodePublicKey()) < 0
			}
			return producers[i].Votes() > producers[j].Votes()
		})

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

func (a *arbitrators) updateOwnerProgramHashes() error {
	a.currentOwnerProgramHashes = make([]*common.Uint168, 0)
	a.ownerVotesInRound = make(map[common.Uint168]common.Fixed64, 0)
	a.totalVotesInRound = 0
	for _, nodePublicKey := range a.currentArbitrators {
		if a.IsCRCArbitratorNodePublicKey(common.BytesToHexString(nodePublicKey)) {
			ownerPublicKey := nodePublicKey // crc node public key is its owner public key for now
			programHash, err := contract.PublicKeyToStandardProgramHash(ownerPublicKey)
			if err != nil {
				return err
			}
			a.currentOwnerProgramHashes = append(a.currentOwnerProgramHashes, programHash)
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
			a.currentOwnerProgramHashes = append(a.currentOwnerProgramHashes, programHash)
			a.ownerVotesInRound[*programHash] = producer.Votes()
			a.totalVotesInRound += producer.Votes()
		}
	}

	a.candidateOwnerProgramHashes = make([]*common.Uint168, 0)
	for _, nodePublicKey := range a.currentCandidates {
		if a.IsCRCArbitratorNodePublicKey(common.BytesToHexString(nodePublicKey)) {
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
		a.candidateOwnerProgramHashes = append(a.candidateOwnerProgramHashes, programHash)
		a.ownerVotesInRound[*programHash] = producer.Votes()
		a.totalVotesInRound += producer.Votes()
	}

	return nil
}

func (a *arbitrators) DumpInfo() {
	a.mtx.Lock()
	defer a.mtx.Unlock()

	var printer func(string, ...interface{})
	changeType, _ := a.getChangeType(a.State.history.height + 1)
	switch changeType {
	case updateNext:
		fallthrough
	case normalChange:
		printer = log.Debugf
	case none:
		printer = log.Infof
	}

	connectionInfoMap := a.getProducersConnectionInfo()
	var crInfo string
	crParams := make([]interface{}, 0)
	if len(a.currentArbitrators) != 0 {
		crInfo, crParams = a.getArbitersInfoWithOnduty("CURRENT ARBITERS", a.currentArbitrators, connectionInfoMap)
	} else {
		crInfo, crParams = a.getArbitersInfoWithoutOnduty("CURRENT ARBITERS", a.currentArbitrators, connectionInfoMap)
	}
	nrInfo, nrParams := a.getArbitersInfoWithoutOnduty("NEXT ARBITERS", a.nextArbitrators, connectionInfoMap)
	ccInfo, ccParams := a.getArbitersInfoWithoutOnduty("CURRENT CANDIDATES", a.currentCandidates, connectionInfoMap)
	ncInfo, ncParams := a.getArbitersInfoWithoutOnduty("NEXT CANDIDATES", a.nextCandidates, connectionInfoMap)
	printer(crInfo+nrInfo+ccInfo+ncInfo, append(append(append(crParams, nrParams...), ccParams...), ncParams...)...)
}

func (a *arbitrators) getProducersConnectionInfo() (result map[string]p2p.PeerAddr) {
	result = make(map[string]p2p.PeerAddr)
	for k, v := range a.crcArbitratorsNodePublicKey {
		pid := peer.PID{}
		copy(pid[:], v.info.NodePublicKey)
		result[k] = p2p.PeerAddr{PID: pid, Addr: v.info.NetAddress}
	}
	for _, p := range a.State.activityProducers {
		if len(p.Info().NodePublicKey) != 33 {
			log.Warn("[getProducersConnectionInfo] invalid public key")
			continue
		}
		pid := peer.PID{}
		copy(pid[:], p.Info().NodePublicKey)
		result[hex.EncodeToString(p.Info().NodePublicKey)] =
			p2p.PeerAddr{PID: pid, Addr: p.Info().NetAddress}
	}

	return result
}

func (a *arbitrators) getBlockDPOSReward(block *types.Block) common.Fixed64 {
	totalTxFx := common.Fixed64(0)
	for _, tx := range block.Transactions {
		totalTxFx += tx.Fee
	}

	return common.Fixed64(math.Ceil(float64(totalTxFx +
		a.chainParams.RewardPerBlock) * 0.35))
}

func (a *arbitrators) getArbitersInfoWithOnduty(title string, arbiters [][]byte,
	connectionInfoMap map[string]p2p.PeerAddr) (string, []interface{}) {
	info := "\n" + title + "\nDUTYINDEX: %d\n%5s %66s %21s %6s\n----- " + strings.Repeat("-", 66) +
		" " + strings.Repeat("-", 21) + " ------\n"
	params := make([]interface{}, 0)
	params = append(params, (a.dutyIndex+1)%len(arbiters))
	params = append(params, "INDEX", "PUBLICKEY", "NETADDRESS", "ONDUTY")
	for i, arbiter := range arbiters {
		publicKey := common.BytesToHexString(arbiter)
		info += "%-5d %-66s %21s %6t\n"
		params = append(params, i+1, publicKey,
			connectionInfoMap[publicKey].Addr, bytes.Equal(arbiter, a.GetOnDutyArbitrator()))
	}
	info += "----- " + strings.Repeat("-", 66) + " " + strings.Repeat("-", 21) + " ------"
	return info, params
}

func (a *arbitrators) getArbitersInfoWithoutOnduty(title string, arbiters [][]byte,
	connectionInfoMap map[string]p2p.PeerAddr) (string, []interface{}) {

	info := "\n" + title + "\n%5s %66s %21s\n----- " + strings.Repeat("-", 66) +
		" " + strings.Repeat("-", 21) + "\n"
	params := make([]interface{}, 0)
	params = append(params, "INDEX", "PUBLICKEY", "NETADDRESS")
	for i, arbiter := range arbiters {
		publicKey := common.BytesToHexString(arbiter)
		info += "%-5d %-66s %21s\n"
		params = append(params, i+1, publicKey, connectionInfoMap[publicKey].Addr)
	}
	info += "----- " + strings.Repeat("-", 66) + " " + strings.Repeat("-", 21)
	return info, params
}

func NewArbitrators(chainParams *config.Params, bestHeight func() uint32,
	bestBlock func() (*types.Block, error)) (*arbitrators, error) {

	originArbiters := make([][]byte, len(chainParams.OriginArbiters))
	originArbitersProgramHashes := make([]*common.Uint168, len(chainParams.OriginArbiters))
	for i, arbiter := range chainParams.OriginArbiters {
		a, err := common.HexStringToBytes(arbiter)
		if err != nil {
			return nil, err
		}
		originArbiters[i] = a

		publicKey, err := common.HexStringToBytes(arbiter)
		if err != nil {
			return nil, err
		}
		hash, err := contract.PublicKeyToStandardProgramHash(publicKey)
		if err != nil {
			return nil, err
		}
		originArbitersProgramHashes[i] = hash
	}

	crcNodeMap := make(map[string]*Producer)
	crcArbitratorsProgramHashes := make(map[common.Uint168]interface{})
	crcArbiters := make([][]byte, len(chainParams.CRCArbiters))
	for _, v := range chainParams.CRCArbiters {
		pubKey, err := hex.DecodeString(v.PublicKey)
		if err != nil {
			return nil, err
		}
		hash, err := contract.PublicKeyToStandardProgramHash(pubKey)
		if err != nil {
			return nil, err
		}
		crcArbiters = append(crcArbiters, pubKey)
		crcArbitratorsProgramHashes[*hash] = nil
		crcNodeMap[v.PublicKey] = &Producer{ // here need crc NODE public key
			info: payload.ProducerInfo{
				OwnerPublicKey: pubKey,
				NodePublicKey:  pubKey,
				NetAddress:     v.NetAddress,
			},
			activateRequestHeight: math.MaxUint32,
		}
	}

	arbitersCount := chainParams.GeneralArbiters + len(chainParams.CRCArbiters)
	a := &arbitrators{
		chainParams:                 chainParams,
		bestHeight:                  bestHeight,
		bestBlock:                   bestBlock,
		arbitersCount:               arbitersCount,
		currentArbitrators:          originArbiters,
		currentOwnerProgramHashes:   originArbitersProgramHashes,
		nextArbitrators:             originArbiters,
		nextCandidates:              make([][]byte, 0),
		crcArbiters:                 crcArbiters,
		crcArbitratorsNodePublicKey: crcNodeMap,
		crcArbitratorsProgramHashes: crcArbitratorsProgramHashes,
		accumulativeReward:          common.Fixed64(0),
		finalRoundChange:            common.Fixed64(0),
		arbitersRoundReward:         nil,
		illegalBlocksPayloadHashes:  make(map[common.Uint256]interface{}),
	}
	a.State = NewState(chainParams, a.GetArbitrators)

	return a, nil
}
