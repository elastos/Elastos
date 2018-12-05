package blockchain

import (
	"errors"
	"sort"
	"sync"

	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/types"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type ArbitratorsConfig struct {
	ArbitratorsCount uint32
	CandidatesCount  uint32
	MajorityCount    uint32
}

type ArbitratorsListener interface {
	OnNewElection(arbiters [][]byte)
}

type Arbitrators interface {
	NewBlocksListener

	StartUp() error
	ForceChange() error

	GetArbitrators() [][]byte
	GetCandidates() [][]byte
	GetNextArbitrators() [][]byte
	GetNextCandidates() [][]byte
	GetArbitratorsProgramHashes() []*common.Uint168
	GetCandidatesProgramHashes() []*common.Uint168

	GetOnDutyArbitrator() []byte
	GetNextOnDutyArbitrator(offset uint32) []byte

	HasArbitersMajorityCount(num uint32) bool
	HasArbitersMinorityCount(num uint32) bool

	RegisterListener(listener ArbitratorsListener)
	UnregisterListener(listener ArbitratorsListener)
}

type arbitrators struct {
	config           ArbitratorsConfig
	dutyChangedCount uint32

	currentArbitrators [][]byte
	currentCandidates  [][]byte

	currentArbitratorsProgramHashes []*common.Uint168
	currentCandidatesProgramHashes  []*common.Uint168

	nextArbitrators [][]byte
	nextCandidates  [][]byte

	listener ArbitratorsListener
	lock     sync.Mutex
}

func (a *arbitrators) StartUp() error {
	a.lock.Lock()
	defer a.lock.Unlock()

	if err := DefaultLedger.Store.GetArbitrators(a); err != nil {
		return err
	}
	if err := a.updateArbitratorsProgramHashes(); err != nil {
		return err
	}

	return nil
}

func (a *arbitrators) ForceChange() error {
	block, err := DefaultLedger.GetBlockWithHeight(DefaultLedger.Blockchain.BlockHeight)
	if err != nil {
		return err
	}

	if err = a.updateNextArbitrators(block); err != nil {
		return err
	}

	if err = a.changeCurrentArbitrators(); err != nil {
		return err
	}

	if a.listener != nil {
		a.listener.OnNewElection(a.nextArbitrators)
	}

	return nil
}

func (a *arbitrators) OnBlockReceived(b *types.Block, confirmed bool) {
	if confirmed {
		a.lock.Lock()
		a.onChainHeightIncreased(b)
		a.lock.Unlock()
	}
}

func (a *arbitrators) OnConfirmReceived(p *types.DPosProposalVoteSlot) {
	block, err := DefaultLedger.GetBlockWithHash(p.Hash)
	if err != nil {
		log.Error("Error occurred when changing arbitrators, details: ", err)
		return
	}

	a.lock.Lock()
	defer a.lock.Unlock()

	a.onChainHeightIncreased(block)
}

func (a *arbitrators) GetArbitrators() [][]byte {
	a.lock.Lock()
	defer a.lock.Unlock()

	return a.currentArbitrators
}

func (a *arbitrators) GetCandidates() [][]byte {
	a.lock.Lock()
	defer a.lock.Unlock()

	return a.currentCandidates
}

func (a *arbitrators) GetNextArbitrators() [][]byte {
	a.lock.Lock()
	defer a.lock.Unlock()

	return a.nextArbitrators
}

func (a *arbitrators) GetNextCandidates() [][]byte {
	a.lock.Lock()
	defer a.lock.Unlock()

	return a.nextCandidates
}

func (a *arbitrators) GetArbitratorsProgramHashes() []*common.Uint168 {
	a.lock.Lock()
	defer a.lock.Unlock()

	return a.currentArbitratorsProgramHashes
}

func (a *arbitrators) GetCandidatesProgramHashes() []*common.Uint168 {
	a.lock.Lock()
	defer a.lock.Unlock()

	return a.currentCandidatesProgramHashes
}

func (a *arbitrators) GetOnDutyArbitrator() []byte {
	return a.GetNextOnDutyArbitrator(uint32(0))
}

func (a *arbitrators) GetNextOnDutyArbitrator(offset uint32) []byte {
	return DefaultLedger.HeightVersions.GetNextOnDutyArbitrator(DefaultLedger.Blockchain.BlockHeight, a.dutyChangedCount, offset)
}

func (a *arbitrators) HasArbitersMajorityCount(num uint32) bool {
	return num > a.config.MajorityCount
}

func (a *arbitrators) HasArbitersMinorityCount(num uint32) bool {
	return num >= a.config.ArbitratorsCount-a.config.MajorityCount
}

func (a *arbitrators) RegisterListener(listener ArbitratorsListener) {
	a.listener = listener
}

func (a *arbitrators) UnregisterListener(listener ArbitratorsListener) {
	a.listener = nil
}

func (a *arbitrators) onChainHeightIncreased(block *types.Block) {
	if a.isNewElection() {
		if err := a.changeCurrentArbitrators(); err != nil {
			log.Error("Change current arbitrators error: ", err)
			return
		}

		if err := a.updateNextArbitrators(block); err != nil {
			log.Error("Update arbitrators error: ", err)
			return
		}

		if a.listener != nil {
			a.listener.OnNewElection(a.nextArbitrators)
		}
	} else {
		a.dutyChangedCount++
		DefaultLedger.Store.SaveDposDutyChangedCount(a.dutyChangedCount)
	}
}

func (a *arbitrators) isNewElection() bool {
	return a.dutyChangedCount == a.config.ArbitratorsCount-1
}

func (a *arbitrators) changeCurrentArbitrators() error {
	a.currentArbitrators = a.nextArbitrators
	a.currentCandidates = a.nextCandidates

	DefaultLedger.Store.SaveCurrentArbitrators(a)

	if err := a.sortArbitrators(); err != nil {
		return err
	}

	if err := a.updateArbitratorsProgramHashes(); err != nil {
		return err
	}

	a.dutyChangedCount = 0
	DefaultLedger.Store.SaveDposDutyChangedCount(a.dutyChangedCount)

	return nil
}

func (a *arbitrators) updateNextArbitrators(block *types.Block) error {
	producers, err := DefaultLedger.HeightVersions.GetProducersDesc(block)
	if err != nil {
		return err
	}

	if uint32(len(producers)) < a.config.ArbitratorsCount {
		return errors.New("Producers count less than arbitrators count.")
	}

	a.nextArbitrators = producers[:a.config.ArbitratorsCount]

	if uint32(len(producers)) < a.config.ArbitratorsCount+a.config.CandidatesCount {
		a.nextCandidates = producers[a.config.ArbitratorsCount:]
	} else {
		a.nextCandidates = producers[a.config.ArbitratorsCount : a.config.ArbitratorsCount+a.config.CandidatesCount]
	}

	DefaultLedger.Store.SaveNextArbitrators(a)
	return nil
}

func (a *arbitrators) sortArbitrators() error {

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

func (a *arbitrators) updateArbitratorsProgramHashes() error {
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

func NewArbitrators(arbitratorsConfig ArbitratorsConfig) Arbitrators {
	if arbitratorsConfig.MajorityCount > arbitratorsConfig.ArbitratorsCount {
		log.Error("Majority count should less or equal than arbitrators count.")
		return nil
	}

	return &arbitrators{
		config: arbitratorsConfig,
	}
}
