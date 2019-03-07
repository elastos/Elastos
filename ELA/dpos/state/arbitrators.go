package state

import (
	"bytes"
	"encoding/hex"
	"errors"
	"sort"
	"strings"
	"sync"

	"github.com/elastos/Elastos.ELA/blockchain/interfaces"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/dpos/p2p"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"
	"github.com/elastos/Elastos.ELA/events"
)

const (
	// majoritySignRatioNumerator defines the ratio numerator to achieve
	// majority signatures.
	majoritySignRatioNumerator = float64(2)

	// majoritySignRatioDenominator defines the ratio denominator to achieve
	// majority signatures.
	majoritySignRatioDenominator = float64(3)
)

type Arbitrators struct {
	*State
	chainParams   *config.Params
	versions      interfaces.HeightVersions
	bestHeight    func() uint32
	arbitersCount uint32

	mtx                             sync.Mutex
	dutyIndex                       uint32
	currentArbitrators              [][]byte
	currentCandidates               [][]byte
	currentArbitratorsProgramHashes []*common.Uint168
	currentCandidatesProgramHashes  []*common.Uint168
	nextArbitrators                 [][]byte
	nextCandidates                  [][]byte
	crcArbitratorsProgramHashes     map[common.Uint168]interface{}
}

func (a *Arbitrators) ForceChange(height uint32) error {
	if err := a.updateNextArbitrators(height + 1); err != nil {
		return err
	}

	if err := a.changeCurrentArbitrators(); err != nil {
		return err
	}

	a.showArbitersInfo(true)
	events.Notify(events.ETNewArbiterElection, a.nextArbitrators)

	return nil
}

func (a *Arbitrators) NormalChange(height uint32) error {
	if err := a.changeCurrentArbitrators(); err != nil {
		return err
	}

	if err := a.updateNextArbitrators(height + 1); err != nil {
		return err
	}

	a.showArbitersInfo(true)
	events.Notify(events.ETNewArbiterElection, a.nextArbitrators)

	return nil
}

func (a *Arbitrators) IncreaseChainHeight(height uint32) {
	forceChange, normalChange := a.isNewElection(height + 1)
	if forceChange {
		if err := a.ForceChange(height); err != nil {
			panic("force change fail when finding an inactive arbitrators" +
				" transaction")
		}
	} else if normalChange {
		if err := a.NormalChange(height); err != nil {
			panic("normal change fail when finding an inactive arbitrators" +
				" transaction")
		}
	} else {
		a.dutyIndex++
		a.showArbitersInfo(false)
	}
}

func (a *Arbitrators) DecreaseChainHeight(height uint32) {

	if a.dutyIndex == 0 {
		//todo complete me
	} else {
		a.dutyIndex--
	}
}

func (a *Arbitrators) IsArbitrator(pk []byte) bool {
	arbitrators := a.GetArbitrators()

	for _, v := range arbitrators {
		if bytes.Equal(pk, v) {
			return true
		}
	}
	return false
}

func (a *Arbitrators) GetArbitrators() [][]byte {
	a.mtx.Lock()
	result := a.currentArbitrators
	a.mtx.Unlock()

	return result
}

func (a *Arbitrators) GetCandidates() [][]byte {
	a.mtx.Lock()
	result := a.currentCandidates
	a.mtx.Unlock()

	return result
}

func (a *Arbitrators) GetNextArbitrators() [][]byte {
	a.mtx.Lock()
	result := a.nextArbitrators
	a.mtx.Unlock()

	return result
}

func (a *Arbitrators) GetNextCandidates() [][]byte {
	a.mtx.Lock()
	result := a.nextCandidates
	a.mtx.Unlock()

	return result
}

func (a *Arbitrators) IsCRCArbitratorProgramHash(hash *common.Uint168) bool {
	_, ok := a.crcArbitratorsProgramHashes[*hash]
	return ok
}

func (a *Arbitrators) IsCRCArbitrator(pk []byte) bool {
	for _, v := range a.chainParams.CRCArbiters {
		if bytes.Equal(v.PublicKey, pk) {
			return true
		}
	}
	return false
}

func (a *Arbitrators) GetCRCArbitrators() []config.CRCArbiter {
	return a.chainParams.CRCArbiters
}

func (a *Arbitrators) GetArbitratorsProgramHashes() []*common.Uint168 {
	a.mtx.Lock()
	result := a.currentArbitratorsProgramHashes
	a.mtx.Unlock()

	return result
}

func (a *Arbitrators) GetCandidatesProgramHashes() []*common.Uint168 {
	a.mtx.Lock()
	result := a.currentCandidatesProgramHashes
	a.mtx.Unlock()

	return result
}

func (a *Arbitrators) GetOnDutyArbitratorByHeight(height uint32) []byte {
	return a.GetNextOnDutyArbitratorV(height, 0)
}

func (a *Arbitrators) GetOnDutyArbitrator() []byte {
	return a.GetNextOnDutyArbitratorV(a.bestHeight()+1, 0)
}

func (a *Arbitrators) GetNextOnDutyArbitrator(offset uint32) []byte {
	return a.GetNextOnDutyArbitratorV(a.bestHeight()+1, offset)
}

func (a *Arbitrators) GetNextOnDutyArbitratorV(height, offset uint32) []byte {
	// main version is >= H1
	if height >= a.State.chainParams.HeightVersions[2] {
		arbitrators := a.currentArbitrators
		if len(arbitrators) == 0 {
			return nil
		}
		index := (a.dutyIndex + offset) % uint32(len(arbitrators))
		arbiter := arbitrators[index]

		return arbiter
	}

	// old version
	return a.GetNextOnDutyArbitratorV0(height, offset)
}

func (a *Arbitrators) GetArbitersCount() uint32 {
	a.mtx.Lock()
	result := a.getArbitersCount()
	a.mtx.Unlock()
	return result
}

func (a *Arbitrators) GetArbitersMajorityCount() uint32 {
	a.mtx.Lock()
	minSignCount := uint32(float64(a.getArbitersCount()) *
		majoritySignRatioNumerator / majoritySignRatioDenominator)
	a.mtx.Unlock()
	return minSignCount
}

func (a *Arbitrators) HasArbitersMajorityCount(num uint32) bool {
	return num > a.GetArbitersMajorityCount()
}

func (a *Arbitrators) HasArbitersMinorityCount(num uint32) bool {
	return num >= a.arbitersCount-a.GetArbitersMajorityCount()
}

func (a *Arbitrators) isNewElection(height uint32) (forceChange bool, normalChange bool) {
	// main version >= H1
	if height >= a.State.chainParams.HeightVersions[2] {
		// when change to "H1" or "H2" height should fire new election immediately
		if height == a.chainParams.HeightVersions[2] || height == a.chainParams.HeightVersions[3] {
			return true, false
		}
		return false, a.dutyIndex == a.arbitersCount-1
	}

	return false, false
}

func (a *Arbitrators) changeCurrentArbitrators() error {
	a.currentArbitrators = a.nextArbitrators
	a.currentCandidates = a.nextCandidates

	if err := a.sortArbitrators(); err != nil {
		return err
	}

	if err := a.updateArbitratorsProgramHashes(); err != nil {
		return err
	}

	a.dutyIndex = 1
	return nil
}

func (a *Arbitrators) updateNextArbitrators(height uint32) error {
	crcCount := uint32(0)
	a.nextArbitrators = make([][]byte, 0)
	for _, v := range a.chainParams.CRCArbiters {
		if !a.isInactiveProducer(v.PublicKey) {
			a.nextArbitrators = append(a.nextArbitrators, v.PublicKey)
		} else {
			crcCount++
		}
	}
	count := config.Parameters.ArbiterConfiguration.NormalArbitratorsCount + crcCount
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

func (a *Arbitrators) GetCandidatesDesc(height, startIndex uint32, producers []*Producer) ([][]byte, error) {
	// main version >= H2
	if height >= a.State.chainParams.HeightVersions[3] {
		if uint32(len(producers)) < startIndex {
			return make([][]byte, 0), nil
		}

		sort.Slice(producers, func(i, j int) bool {
			return producers[i].Votes() > producers[j].Votes()
		})

		result := make([][]byte, 0)
		for i := startIndex; i < uint32(len(producers)) && i < startIndex+config.
			Parameters.ArbiterConfiguration.CandidatesCount; i++ {
			result = append(result, producers[i].NodePublicKey())
		}
		return result, nil
	}

	// old version [0, H2)
	return [][]byte{}, nil
}

func (a *Arbitrators) GetNormalArbitratorsDesc(height uint32,
	arbitratorsCount uint32, producers []*Producer) ([][]byte, error) {
	// main version >= H2
	if height >= a.State.chainParams.HeightVersions[3] {
		if uint32(len(producers)) < arbitratorsCount/2+1 {
			return nil, errors.New("producers count less than min arbitrators count")
		}

		sort.Slice(producers, func(i, j int) bool {
			return producers[i].Votes() > producers[j].Votes()
		})

		result := make([][]byte, 0)
		for i := uint32(0); i < arbitratorsCount && i < uint32(len(producers)); i++ {
			result = append(result, producers[i].NodePublicKey())
		}
		return result, nil
	}

	// version [H1, H2)
	if height >= a.State.chainParams.HeightVersions[2] {
		return a.GetNormalArbitratorsDescV1()
	}

	// version [0, H1)
	return a.GetNormalArbitratorsDescV0()
}

func (a *Arbitrators) sortArbitrators() error {

	strArbitrators := make([]string, len(a.currentArbitrators))
	for i := 0; i < len(strArbitrators); i++ {
		strArbitrators[i] = common.BytesToHexString(a.currentArbitrators[i])
	}
	sort.Strings(strArbitrators)

	a.currentArbitrators = make([][]byte, len(strArbitrators))
	for i := 0; i < len(strArbitrators); i++ {
		value, err := common.HexStringToBytes(strArbitrators[i])
		if err != nil {
			return err
		}
		a.currentArbitrators[i] = value
	}

	return nil
}

func (a *Arbitrators) getArbitersCount() uint32 {
	return uint32(len(a.currentArbitrators))
}

func (a *Arbitrators) updateArbitratorsProgramHashes() error {
	a.currentArbitratorsProgramHashes = make([]*common.Uint168, len(a.currentArbitrators))
	for index, v := range a.currentArbitrators {
		hash, err := contract.PublicKeyToStandardProgramHash(v)
		if err != nil {
			return err
		}
		a.currentArbitratorsProgramHashes[index] = hash
	}

	a.currentCandidatesProgramHashes = make([]*common.Uint168, len(a.currentCandidates))
	for index, v := range a.currentCandidates {
		hash, err := contract.PublicKeyToStandardProgramHash(v)
		if err != nil {
			return err
		}
		a.currentCandidatesProgramHashes[index] = hash
	}

	return nil
}

func (a *Arbitrators) showArbitersInfo(isInfo bool) {
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

func (a *Arbitrators) getProducersConnectionInfo() (result map[string]p2p.PeerAddr) {
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

func (a *Arbitrators) getArbitersInfoWithOnduty(title string, arbiters [][]byte,
	connectionInfoMap map[string]p2p.PeerAddr) (string, []interface{}) {
	info := "\n" + title + "\nDUTYINDEX: %d\n%5s %66s %21s %6s\n----- " + strings.Repeat("-", 66) +
		" " + strings.Repeat("-", 21) + " ------\n"
	params := make([]interface{}, 0)
	params = append(params, (a.dutyIndex+1)%uint32(len(arbiters)))
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

func (a *Arbitrators) getArbitersInfoWithoutOnduty(title string, arbiters [][]byte,
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

func NewArbitrators(chainParams *config.Params, versions interfaces.
	HeightVersions, bestHeight func() uint32) (*Arbitrators, error) {

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

	arbitersCount := config.Parameters.ArbiterConfiguration.
		NormalArbitratorsCount + uint32(len(chainParams.CRCArbiters))
	a := &Arbitrators{
		chainParams:                     chainParams,
		versions:                        versions,
		bestHeight:                      bestHeight,
		arbitersCount:                   arbitersCount,
		currentArbitrators:              originArbiters,
		currentArbitratorsProgramHashes: originArbitersProgramHashes,
		nextArbitrators:                 originArbiters,
		nextCandidates:                  make([][]byte, 0),
	}
	a.State = NewState(a, chainParams)

	a.crcArbitratorsProgramHashes = make(map[common.Uint168]interface{})
	for _, v := range a.chainParams.CRCArbiters {
		//a.nextArbitrators = append(a.nextArbitrators, v.PublicKey)

		hash, err := contract.PublicKeyToStandardProgramHash(v.PublicKey)
		if err != nil {
			return nil, err
		}
		a.crcArbitratorsProgramHashes[*hash] = nil
		//a.activityProducers[a.getProducerKey(v.PublicKey)] = &Producer{
		//	info: payload.ProducerInfo{
		//		OwnerPublicKey: v.PublicKey,
		//		NodePublicKey:  v.PublicKey,
		//		NetAddress:     v.NetAddress,
		//	},
		//	registerHeight:        0,
		//	votes:                 0,
		//	inactiveSince:         0,
		//	penalty:               common.Fixed64(0),
		//	activateRequestHeight: math.MaxUint32,
		//}
	}

	return a, nil
}
