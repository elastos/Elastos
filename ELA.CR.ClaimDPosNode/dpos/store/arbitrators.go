package store

import (
	"errors"
	"fmt"
	"sort"
	"sync"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/blockchain/interfaces"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/events"
)

type ArbitratorsConfig struct {
	ArbitratorsCount uint32
	CandidatesCount  uint32
	MajorityCount    uint32
	CRCArbitrators   [][]byte
	Versions         interfaces.HeightVersions
	Store            interfaces.IDposStore
	ChainStore       blockchain.IChainStore
}

type Arbitrators struct {
	cfg              ArbitratorsConfig
	DutyChangedCount uint32

	currentArbitrators [][]byte
	currentCandidates  [][]byte

	currentArbitratorsProgramHashes []*common.Uint168
	currentCandidatesProgramHashes  []*common.Uint168

	nextArbitrators [][]byte
	nextCandidates  [][]byte

	lock sync.Mutex
}

func (a *Arbitrators) Start() error {
	a.lock.Lock()
	defer a.lock.Unlock()

	block, err := a.cfg.ChainStore.GetBlock(a.cfg.ChainStore.GetCurrentBlockHash())
	if err != nil {
		return err
	}
	if a.cfg.Versions.GetDefaultBlockVersion(block.Height) == 0 {
		if a.currentArbitrators, err = a.cfg.Versions.GetProducersDesc(block); err != nil {
			return err
		}
	} else {
		if err := a.cfg.Store.GetArbitrators(a); err != nil {
			return err
		}
	}

	if err := a.updateArbitratorsProgramHashes(); err != nil {
		return err
	}

	return nil
}

func (a *Arbitrators) ForceChange() error {
	block, err := a.cfg.ChainStore.GetBlock(a.cfg.ChainStore.GetCurrentBlockHash())
	if err != nil {
		return err
	}

	if err = a.updateNextArbitrators(block); err != nil {
		return err
	}

	if err = a.changeCurrentArbitrators(); err != nil {
		return err
	}

	events.Notify(events.ETNewArbiterElection, a.nextArbitrators)

	return nil
}

func (a *Arbitrators) OnBlockReceived(b *types.Block, confirmed bool) {
	if confirmed {
		a.lock.Lock()
		a.onChainHeightIncreased(b)
		a.lock.Unlock()
	}
}

func (a *Arbitrators) OnConfirmReceived(p *types.DPosProposalVoteSlot) {
	a.lock.Lock()
	defer a.lock.Unlock()
	block, err := a.cfg.ChainStore.GetBlock(p.Hash)
	if err != nil {
		log.Warn("Error occurred when changing arbitrators, details: ", err)
		return
	}

	a.onChainHeightIncreased(block)
}

func (a *Arbitrators) GetArbitrators() [][]byte {
	a.lock.Lock()
	defer a.lock.Unlock()

	return a.currentArbitrators
}

func (a *Arbitrators) GetCandidates() [][]byte {
	a.lock.Lock()
	defer a.lock.Unlock()

	return a.currentCandidates
}

func (a *Arbitrators) GetNextArbitrators() [][]byte {
	a.lock.Lock()
	defer a.lock.Unlock()

	return a.nextArbitrators
}

func (a *Arbitrators) GetNextCandidates() [][]byte {
	a.lock.Lock()
	defer a.lock.Unlock()

	return a.nextCandidates
}

func (a *Arbitrators) GetArbitratorsProgramHashes() []*common.Uint168 {
	a.lock.Lock()
	defer a.lock.Unlock()

	return a.currentArbitratorsProgramHashes
}

func (a *Arbitrators) GetCandidatesProgramHashes() []*common.Uint168 {
	a.lock.Lock()
	defer a.lock.Unlock()

	return a.currentCandidatesProgramHashes
}

func (a *Arbitrators) GetOnDutyArbitrator() []byte {
	return a.GetNextOnDutyArbitrator(uint32(0))
}

func (a *Arbitrators) GetNextOnDutyArbitrator(offset uint32) []byte {
	return a.cfg.Versions.GetNextOnDutyArbitrator(a.cfg.ChainStore.GetHeight(),
		a.DutyChangedCount, offset)
}

func (a *Arbitrators) HasArbitersMajorityCount(num uint32) bool {
	return num > a.cfg.MajorityCount
}

func (a *Arbitrators) HasArbitersMinorityCount(num uint32) bool {
	return num >= a.cfg.ArbitratorsCount-a.cfg.MajorityCount
}

func (a *Arbitrators) onChainHeightIncreased(block *types.Block) {
	if a.isNewElection() {
		if err := a.changeCurrentArbitrators(); err != nil {
			log.Error("Change current arbitrators error: ", err)
			return
		}

		if err := a.updateNextArbitrators(block); err != nil {
			log.Error("Update arbitrators error: ", err)
			return
		}

		events.Notify(events.ETNewArbiterElection, a.nextArbitrators)

	} else {
		a.DutyChangedCount++
		a.cfg.Store.SaveDposDutyChangedCount(a.DutyChangedCount)
	}
}

func (a *Arbitrators) isNewElection() bool {
	return a.DutyChangedCount == a.cfg.ArbitratorsCount-1
}

func (a *Arbitrators) changeCurrentArbitrators() error {
	a.currentArbitrators = a.nextArbitrators
	a.currentCandidates = a.nextCandidates

	a.cfg.Store.SaveCurrentArbitrators(a)

	if err := a.sortArbitrators(); err != nil {
		return err
	}

	if err := a.updateArbitratorsProgramHashes(); err != nil {
		return err
	}

	a.DutyChangedCount = 0
	a.cfg.Store.SaveDposDutyChangedCount(a.DutyChangedCount)

	return nil
}

func (a *Arbitrators) updateNextArbitrators(block *types.Block) error {
	producers, err := a.cfg.Versions.GetProducersDesc(block)
	if err != nil {
		return err
	}

	normalArbitratorsCount := a.cfg.ArbitratorsCount - uint32(len(a.cfg.CRCArbitrators))
	if uint32(len(producers)) < normalArbitratorsCount {
		return errors.New("producers count less than arbitrators count")
	}

	a.nextArbitrators = producers[:normalArbitratorsCount]
	a.nextCandidates = append(a.nextCandidates, a.cfg.CRCArbitrators...)

	if uint32(len(producers)) < normalArbitratorsCount+a.cfg.CandidatesCount {
		a.nextCandidates = producers[normalArbitratorsCount:]
	} else {
		a.nextCandidates = producers[normalArbitratorsCount : normalArbitratorsCount+a.cfg.CandidatesCount]
	}

	a.cfg.Store.SaveNextArbitrators(a)
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
	if cfg.MajorityCount > cfg.ArbitratorsCount {
		return nil, fmt.Errorf("Majority count should less or equal than arbitrators count.")
	}
	return &Arbitrators{cfg: *cfg}, nil
}
