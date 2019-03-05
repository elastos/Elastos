package state

import (
	"bytes"
	"math"
	"sort"
	"sync"

	"github.com/elastos/Elastos.ELA/blockchain/interfaces"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/types/payload"
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
	if err := a.updateNextArbitrators(height); err != nil {
		return err
	}

	if err := a.changeCurrentArbitrators(); err != nil {
		return err
	}

	events.Notify(events.ETNewArbiterElection, a.nextArbitrators)

	return nil
}

func (a *Arbitrators) NormalChange(height uint32) error {
	if err := a.changeCurrentArbitrators(); err != nil {
		return err
	}

	if err := a.updateNextArbitrators(height); err != nil {
		return err
	}

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

func (a *Arbitrators) GetCRCArbitrators() []config.CRCArbitratorParams {
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
	return a.versions.GetNextOnDutyArbitrator(height, a.dutyIndex, 0)
}

func (a *Arbitrators) GetOnDutyArbitrator() []byte {
	return a.GetNextOnDutyArbitrator(0)
}

func (a *Arbitrators) GetNextOnDutyArbitrator(offset uint32) []byte {
	return a.versions.GetNextOnDutyArbitrator(a.bestHeight()+1,
		a.dutyIndex, offset)
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
	if a.versions.GetDefaultBlockVersion(height) >= 1 {

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
	producers, err := a.versions.GetNormalArbitratorsDesc(height, count, a.getInterfaceProducers())
	if err != nil {
		return err
	}
	for _, v := range producers {
		a.nextArbitrators = append(a.nextArbitrators, v)
	}

	candidates, err := a.versions.GetCandidatesDesc(height, count, a.getInterfaceProducers())
	if err != nil {
		return err
	}
	a.nextCandidates = candidates

	return nil
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
		a.nextArbitrators = append(a.nextArbitrators, v.PublicKey)

		hash, err := contract.PublicKeyToStandardProgramHash(v.PublicKey)
		if err != nil {
			return nil, err
		}
		a.crcArbitratorsProgramHashes[*hash] = nil
		a.activityProducers[a.getProducerKey(v.PublicKey)] = &Producer{
			info: payload.ProducerInfo{
				OwnerPublicKey: v.PublicKey,
				NodePublicKey:  v.PublicKey,
				NetAddress:     v.NetAddress,
			},
			registerHeight:        0,
			votes:                 0,
			inactiveSince:         0,
			penalty:               common.Fixed64(0),
			activateRequestHeight: math.MaxUint32,
		}
	}

	return a, nil
}
