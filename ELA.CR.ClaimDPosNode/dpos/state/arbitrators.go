package state

import (
	"bytes"
	"encoding/hex"
	"errors"
	"sort"
	"strconv"
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
	// majoritySignRatioNumerator defines the ratio numerator to achieve
	// majority signatures.
	majoritySignRatioNumerator = float64(2)

	// majoritySignRatioDenominator defines the ratio denominator to achieve
	// majority signatures.
	majoritySignRatioDenominator = float64(3)

	none         = ChangeType(0x00)
	updateNext   = ChangeType(0x01)
	normalChange = ChangeType(0x02)
)

type arbitrators struct {
	*State
	chainParams   *config.Params
	bestHeight    func() uint32
	arbitersCount int

	mtx                sync.Mutex
	dutyIndex          int
	currentArbitrators [][]byte
	currentCandidates  [][]byte

	currentOwnerProgramHashes   []*common.Uint168
	candidateOwnerProgramHashes []*common.Uint168
	ownerVotesInRound           map[common.Uint168]common.Fixed64
	totalVotesInRound           common.Fixed64

	nextArbitrators             [][]byte
	nextCandidates              [][]byte
	crcArbitratorsProgramHashes map[common.Uint168]interface{}
	crcArbitratorsNodePublicKey map[string]interface{}
}

func (a *arbitrators) ProcessBlock(block *types.Block, confirm *payload.Confirm) {
	a.State.ProcessBlock(block, confirm)
	a.IncreaseChainHeight(block.Height)
}

func (a *arbitrators) ProcessSpecialTxPayload(p types.Payload,
	height uint32) error {
	switch p.(type) {
	case *payload.DPOSIllegalBlocks, *payload.InactiveArbitrators:
		a.State.ProcessSpecialTxPayload(p)
		return a.ForceChange(height)
	default:
		return errors.New("[ProcessSpecialTxPayload] invalid payload type")
	}
}

func (a *arbitrators) RollbackTo(height uint32) error {
	if err := a.State.RollbackTo(height); err != nil {
		return err
	}
	a.DecreaseChainHeight(height)
	return nil
}

func (a *arbitrators) GetDutyIndex() int {
	a.mtx.Lock()
	index := a.dutyIndex
	a.mtx.Unlock()

	return index
}

func (a *arbitrators) ForceChange(height uint32) error {
	a.mtx.Lock()
	if err := a.updateNextArbitrators(height + 1); err != nil {
		return err
	}

	if err := a.changeCurrentArbitrators(); err != nil {
		return err
	}

	a.showArbitersInfo(true)
	a.mtx.Unlock()

	events.Notify(events.ETDirectPeersChanged, a.GetNeedConnectArbiters(height))

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

	a.showArbitersInfo(true)

	return nil
}

func (a *arbitrators) IncreaseChainHeight(height uint32) {
	trace := true
	notify := true

	a.mtx.Lock()

	changeType, versionHeight := a.getChangeType(height + 1)
	switch changeType {
	case updateNext:
		if err := a.updateNextArbitrators(versionHeight); err != nil {
			log.Error("[IncreaseChainHeight] update next arbiters error: ", err)
		}
	case normalChange:
		if err := a.NormalChange(height); err != nil {
			panic("normal change fail when finding an inactive arbitrators" +
				" transaction, height: " + strconv.FormatUint(uint64(height), 10))
		}
		trace = false
	case none:
		a.dutyIndex++
		notify = false
	}

	if trace {
		a.showArbitersInfo(false)
	}

	a.mtx.Unlock()

	if notify {
		events.Notify(events.ETDirectPeersChanged,
			a.GetNeedConnectArbiters(versionHeight))
	}
}

func (a *arbitrators) DecreaseChainHeight(height uint32) {
	a.mtx.Lock()
	defer a.mtx.Unlock()

	if a.dutyIndex == 0 {
		//todo complete me
	} else {
		a.dutyIndex--
	}
}

func (a *arbitrators) GetNeedConnectArbiters(height uint32) map[string]*p2p.PeerAddr {
	arbiters := make(map[string]*p2p.PeerAddr)

	if height >= a.chainParams.CRCOnlyDPOSHeight {
		a.mtx.Lock()
		for _, v := range a.chainParams.CRCArbiters {
			str := common.BytesToHexString(v.PublicKey)
			arbiters[str] = a.generatePeerAddr(v.PublicKey, v.NetAddress)
		}

		for _, v := range a.currentArbitrators {
			str := common.BytesToHexString(v)
			if _, exist := arbiters[str]; exist {
				continue
			}
			arbiters[str] = a.getArbiterPeerAddr(v)
		}

		for _, v := range a.nextArbitrators {
			str := common.BytesToHexString(v)
			if _, exist := arbiters[str]; exist {
				continue
			}
			arbiters[str] = a.getArbiterPeerAddr(v)
		}
		a.mtx.Unlock()
	}

	return arbiters
}

func (a *arbitrators) getArbiterPeerAddr(pk []byte) *p2p.PeerAddr {
	producer := a.GetProducer(pk)
	if producer == nil {
		return nil
	}

	return a.generatePeerAddr(pk, producer.info.NetAddress)
}

func (a *arbitrators) generatePeerAddr(pk []byte, ip string) *p2p.PeerAddr {
	pid := peer.PID{}
	copy(pid[:], pk)

	return &p2p.PeerAddr{
		PID:  pid,
		Addr: ip,
	}
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

func (a *arbitrators) IsCRCArbitratorProgramHash(hash *common.Uint168) bool {
	_, ok := a.crcArbitratorsProgramHashes[*hash]
	return ok
}

func (a *arbitrators) IsCRCArbitratorNodePublicKey(nodePublicKeyHex string) bool {
	_, ok := a.crcArbitratorsNodePublicKey[nodePublicKeyHex]
	return ok
}

func (a *arbitrators) IsCRCArbitrator(pk []byte) bool {
	for _, v := range a.chainParams.CRCArbiters {
		if bytes.Equal(v.PublicKey, pk) {
			return true
		}
	}
	return false
}

func (a *arbitrators) GetCRCArbitrators() []config.CRCArbiter {
	return a.chainParams.CRCArbiters
}

func (a *arbitrators) GetCurrentOwnerProgramHashes() []*common.Uint168 {
	a.mtx.Lock()
	result := a.currentOwnerProgramHashes
	a.mtx.Unlock()

	return result
}

func (a *arbitrators) GetCandidateOwnerProgramHashes() []*common.Uint168 {
	a.mtx.Lock()
	result := a.candidateOwnerProgramHashes
	a.mtx.Unlock()

	return result
}

func (a *arbitrators) GetOwnerVotes(programHash *common.Uint168) common.Fixed64 {
	a.mtx.Lock()
	result := a.ownerVotesInRound[*programHash]
	a.mtx.Unlock()

	return result
}

func (a *arbitrators) GetTotalVotesInRound() common.Fixed64 {
	a.mtx.Lock()
	result := a.totalVotesInRound
	a.mtx.Unlock()

	return result
}

func (a *arbitrators) GetOnDutyArbitrator() []byte {
	return a.GetNextOnDutyArbitratorV(a.bestHeight()+1, 0)
}

func (a *arbitrators) GetNextOnDutyArbitrator(offset uint32) []byte {
	return a.GetNextOnDutyArbitratorV(a.bestHeight()+1, offset)
}

func (a *arbitrators) GetNextOnDutyArbitratorV(height, offset uint32) []byte {
	// main version is >= H1
	if height >= a.State.chainParams.HeightVersions[2] {
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
		majoritySignRatioNumerator / majoritySignRatioDenominator)
	a.mtx.Unlock()
	return minSignCount
}

func (a *arbitrators) HasArbitersMajorityCount(num int) bool {
	return num > a.GetArbitersMajorityCount()
}

func (a *arbitrators) HasArbitersMinorityCount(num int) bool {
	return num >= a.arbitersCount-a.GetArbitersMajorityCount()
}

func (a *arbitrators) getChangeType(height uint32) (ChangeType, uint32) {

	// special change points:
	//		H1 - PreConnectHeight -> 	[updateNext, H1]: update next arbiters and let CRC arbiters prepare to connect
	//		H1 -> 						[normalChange, H1]: should change to new election (that only have CRC arbiters)
	//		H2 - PreConnectHeight -> 	[updateNext, H2]: update next arbiters and let normal arbiters prepare to connect
	//		H2 -> 						[normalChange, H2]: should change to new election (arbiters will have both CRC and normal arbiters)
	if height == a.State.chainParams.CRCOnlyDPOSHeight-
		config.Parameters.ArbiterConfiguration.PreConnectHeight {
		return updateNext, a.State.chainParams.CRCOnlyDPOSHeight
	} else if height == a.State.chainParams.CRCOnlyDPOSHeight {
		return normalChange, a.State.chainParams.CRCOnlyDPOSHeight
	} else if height == a.State.chainParams.PublicDPOSHeight-
		config.Parameters.ArbiterConfiguration.PreConnectHeight {
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
	for _, v := range a.chainParams.CRCArbiters {
		if !a.isInactiveProducer(v.PublicKey) {
			a.nextArbitrators = append(a.nextArbitrators, v.PublicKey)
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
	if height >= a.State.chainParams.HeightVersions[3] {
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
	if height >= a.State.chainParams.HeightVersions[3] {
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
	if height >= a.State.chainParams.HeightVersions[2] {
		return a.getNormalArbitratorsDescV1()
	}

	// version [0, H1)
	return a.getNormalArbitratorsDescV0()
}

func (a *arbitrators) updateOwnerProgramHashes() error {
	a.currentOwnerProgramHashes = make([]*common.Uint168, 0)
	a.ownerVotesInRound = make(map[common.Uint168]common.Fixed64, 0)
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

func (a *arbitrators) showArbitersInfo(isInfo bool) {
	show := log.Debugf
	if isInfo {
		show = log.Infof
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
	show(crInfo+nrInfo+ccInfo+ncInfo, append(append(append(crParams, nrParams...), ccParams...), ncParams...)...)
}

func (a *arbitrators) getProducersConnectionInfo() (result map[string]p2p.PeerAddr) {
	result = make(map[string]p2p.PeerAddr)
	crcs := a.chainParams.CRCArbiters
	for _, c := range crcs {
		if len(c.PublicKey) != 33 {
			log.Warn("[getProducersConnectionInfo] invalid public key")
			continue
		}
		pid := peer.PID{}
		copy(pid[:], c.PublicKey)
		result[hex.EncodeToString(c.PublicKey)] =
			p2p.PeerAddr{PID: pid, Addr: c.NetAddress}
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

func NewArbitrators(chainParams *config.Params, bestHeight func() uint32) (*arbitrators, error) {

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

	crcNodeMap := make(map[string]interface{})
	crcArbitratorsProgramHashes := make(map[common.Uint168]interface{})
	for _, v := range chainParams.CRCArbiters {
		crcNodeMap[common.BytesToHexString(v.PublicKey)] = nil

		hash, err := contract.PublicKeyToStandardProgramHash(v.PublicKey)
		if err != nil {
			return nil, err
		}
		crcArbitratorsProgramHashes[*hash] = nil
	}

	arbitersCount := chainParams.GeneralArbiters + len(chainParams.CRCArbiters)
	a := &arbitrators{
		chainParams:                 chainParams,
		bestHeight:                  bestHeight,
		arbitersCount:               arbitersCount,
		currentArbitrators:          originArbiters,
		currentOwnerProgramHashes:   originArbitersProgramHashes,
		nextArbitrators:             originArbiters,
		nextCandidates:              make([][]byte, 0),
		crcArbitratorsNodePublicKey: crcNodeMap,
		crcArbitratorsProgramHashes: crcArbitratorsProgramHashes,
	}
	a.State = NewState(chainParams, a.GetArbitrators)

	return a, nil
}
