package state

import (
	"bytes"
	"sort"

	"github.com/elastos/Elastos.ELA/blockchain/interfaces"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/events"
)

const (
	// Numerator of dpos majority ratio
	DPOSMajorityRatioNumerator = float64(2)

	// Denominator of dpos majority ratio
	DPOSMajorityRatioDenominator = float64(3)
)

type ArbitratorsConfig struct {
	ArbitratorsCount uint32
	CandidatesCount  uint32
	CRCArbitrators   []config.CRCArbitratorParams
	Versions         interfaces.HeightVersions
	OriginArbiters   []string

	GetCurrentHeader func() (*types.Header, error)
	GetBestHeight    func() uint32
}

type Arbitrators struct {
	cfg   ArbitratorsConfig
	State *State

	dutyChangedCount uint32

	currentArbitrators [][]byte
	currentCandidates  [][]byte

	currentArbitratorsProgramHashes []*common.Uint168
	currentCandidatesProgramHashes  []*common.Uint168

	nextArbitrators [][]byte
	nextCandidates  [][]byte

	crcArbitratorsProgramHashes map[common.Uint168]interface{}
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
		a.ForceChange(height)
	} else if normalChange {
		a.NormalChange(height)
	} else {
		a.dutyChangedCount++
	}
}

func (a *Arbitrators) DecreaseChainHeight(height uint32) {

	if a.dutyChangedCount == 0 {
		//todo complete me
	} else {
		a.dutyChangedCount--
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
	a.State.mtx.RLock()
	result := a.currentArbitrators
	a.State.mtx.RUnlock()

	return result
}

func (a *Arbitrators) GetCandidates() [][]byte {
	a.State.mtx.RLock()
	result := a.currentCandidates
	a.State.mtx.RUnlock()

	return result
}

func (a *Arbitrators) GetNextArbitrators() [][]byte {
	a.State.mtx.RLock()
	result := a.nextArbitrators
	a.State.mtx.RUnlock()

	return result
}

func (a *Arbitrators) GetNextCandidates() [][]byte {
	a.State.mtx.RLock()
	result := a.nextCandidates
	a.State.mtx.RUnlock()

	return result
}

func (a *Arbitrators) IsCRCArbitratorProgramHash(hash *common.Uint168) bool {
	_, ok := a.crcArbitratorsProgramHashes[*hash]
	return ok
}

func (a *Arbitrators) IsCRCArbitrator(pk []byte) bool {
	for _, v := range a.cfg.CRCArbitrators {
		if bytes.Equal(v.PublicKey, pk) {
			return true
		}
	}
	return false
}

func (a *Arbitrators) GetCRCArbitrators() []config.CRCArbitratorParams {
	return a.cfg.CRCArbitrators
}

func (a *Arbitrators) GetArbitratorsProgramHashes() []*common.Uint168 {
	a.State.mtx.RLock()
	result := a.currentArbitratorsProgramHashes
	a.State.mtx.RUnlock()

	return result
}

func (a *Arbitrators) GetCandidatesProgramHashes() []*common.Uint168 {
	a.State.mtx.RLock()
	result := a.currentCandidatesProgramHashes
	a.State.mtx.RUnlock()

	return result
}

func (a *Arbitrators) GetOnDutyArbitratorByHeight(height uint32) []byte {
	return a.cfg.Versions.GetNextOnDutyArbitrator(height, a.dutyChangedCount, 0)
}

func (a *Arbitrators) GetOnDutyArbitrator() []byte {
	return a.GetNextOnDutyArbitrator(0)
}

func (a *Arbitrators) GetNextOnDutyArbitrator(offset uint32) []byte {
	return a.cfg.Versions.GetNextOnDutyArbitrator(a.cfg.GetBestHeight()+1,
		a.dutyChangedCount, offset)
}

func (a *Arbitrators) GetArbitersCount() uint32 {
	a.State.mtx.RLock()
	result := a.getArbitersCount()
	a.State.mtx.RUnlock()

	return result
}

func (a *Arbitrators) GetArbitersMajorityCount() uint32 {
	a.State.mtx.RLock()
	minSignCount := float64(a.getArbitersCount()) *
		DPOSMajorityRatioNumerator / DPOSMajorityRatioDenominator
	a.State.mtx.RUnlock()

	return uint32(minSignCount)
}

func (a *Arbitrators) HasArbitersMajorityCount(num uint32) bool {
	return num > a.GetArbitersMajorityCount()
}

func (a *Arbitrators) HasArbitersMinorityCount(num uint32) bool {
	return num >= a.cfg.ArbitratorsCount-a.GetArbitersMajorityCount()
}

func (a *Arbitrators) isNewElection(height uint32) (forceChange bool, normalChange bool) {
	if a.cfg.Versions.GetDefaultBlockVersion(height) >= 1 {

		// when change to "H1" or "H2" height should fire new election immediately
		if height == a.State.chainParams.HeightVersions[2] || height == a.State.chainParams.HeightVersions[3] {
			return true, false
		}
		return false, a.dutyChangedCount == a.cfg.ArbitratorsCount-1
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

	a.dutyChangedCount = 1
	return nil
}

func (a *Arbitrators) updateNextArbitrators(height uint32) error {
	crcCount := uint32(0)
	a.nextArbitrators = make([][]byte, 0)
	for _, v := range a.cfg.CRCArbitrators {
		if !a.State.isInactiveProducer(v.PublicKey) {
			a.nextArbitrators = append(a.nextArbitrators, v.PublicKey)
		} else {
			crcCount++
		}
	}
	count := config.Parameters.ArbiterConfiguration.NormalArbitratorsCount + crcCount
	producers, err := a.cfg.Versions.GetNormalArbitratorsDesc(height, count, a.State.getInterfaceProducers())
	if err != nil {
		return err
	}
	for _, v := range producers {
		a.nextArbitrators = append(a.nextArbitrators, v)
	}

	candidates, err := a.cfg.Versions.GetCandidatesDesc(height, count, a.State.getInterfaceProducers())
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

func NewArbitrators(cfg *ArbitratorsConfig) (*Arbitrators, error) {

	originArbiters := make([][]byte, len(cfg.OriginArbiters))
	originArbitersProgramHashes := make([]*common.Uint168, len(cfg.OriginArbiters))
	for i, arbiter := range cfg.OriginArbiters {
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

	a := &Arbitrators{
		cfg:                             *cfg,
		currentArbitrators:              originArbiters,
		currentArbitratorsProgramHashes: originArbitersProgramHashes,
		nextArbitrators:                 originArbiters,
		nextCandidates:                  make([][]byte, 0),
	}

	a.crcArbitratorsProgramHashes = make(map[common.Uint168]interface{})
	for _, v := range a.cfg.CRCArbitrators {
		a.nextArbitrators = append(a.nextArbitrators, v.PublicKey)

		hash, err := contract.PublicKeyToStandardProgramHash(v.PublicKey)
		if err != nil {
			return nil, err
		}
		a.crcArbitratorsProgramHashes[*hash] = nil
	}

	return a, nil
}
