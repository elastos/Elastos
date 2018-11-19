package blockchain

import (
	"errors"

	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/log"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type ArbitratorsConfig struct {
	ArbitratorsCount uint32
	CandidatesCount  uint32
	MajorityCount    uint32
}

type Arbitrators interface {
	NewBlocksListener

	GetArbitrators() [][]byte
	GetCandidates() [][]byte

	GetOnDutyArbitrator() []byte
	GetNextOnDutyArbitrator(offset uint32) []byte

	HasArbitersMajorityCount(num uint32) bool
	HasArbitersMinorityCount(num uint32) bool
}

type arbitrators struct {
	config ArbitratorsConfig

	currentArbitrators [][]byte
	candidates         [][]byte
}

func (a *arbitrators) OnBlockReceived(b *core.Block, confirmed bool) {
	if err := a.updateArbitrators(); err != nil {
		log.Error("Update arbitrators error: ", err)
	}
}

func (a *arbitrators) OnConfirmReceived(p *core.DPosProposalVoteSlot) {
	if err := a.updateArbitrators(); err != nil {
		log.Error("Update arbitrators error: ", err)
	}
}

func (a *arbitrators) GetArbitrators() [][]byte {
	return a.currentArbitrators
}

func (a *arbitrators) GetCandidates() [][]byte {
	return a.candidates
}

func (a *arbitrators) GetOnDutyArbitrator() []byte {
	return a.GetNextOnDutyArbitrator(uint32(0))
}

func (a *arbitrators) GetNextOnDutyArbitrator(offset uint32) []byte {
	arbitrators := a.GetArbitrators()
	height := DefaultLedger.Store.GetHeight()
	index := (height + offset) % uint32(len(arbitrators))
	arbitrator := arbitrators[index]

	return arbitrator
}

func (a *arbitrators) HasArbitersMajorityCount(num uint32) bool {
	return num >= a.config.MajorityCount
}

func (a *arbitrators) HasArbitersMinorityCount(num uint32) bool {
	return num > a.config.ArbitratorsCount-a.config.MajorityCount
}

func (a *arbitrators) updateArbitrators() error {
	producers, err := a.parseProducersDesc()
	if err == nil {
		return err
	}

	if uint32(len(producers)) < a.config.ArbitratorsCount {
		return errors.New("Producers count less than arbitrators count.")
	}

	a.currentArbitrators = producers[:a.config.ArbitratorsCount]
	a.sortCurrentArbitrators()

	if uint32(len(producers)) < a.config.ArbitratorsCount+a.config.CandidatesCount {
		a.candidates = producers[a.config.ArbitratorsCount:]
	} else {
		a.candidates = producers[a.config.ArbitratorsCount : a.config.ArbitratorsCount+a.config.CandidatesCount]
	}

	return nil
}

func (a *arbitrators) sortCurrentArbitrators() {
	//todo sort arbitrators
}

func (a *arbitrators) parseProducersDesc() ([][]byte, error) {
	//todo parse by get vote results instead
	if len(config.Parameters.Arbiters) == 0 {
		return nil, errors.New("arbiters not configured")
	}

	var arbitersByte [][]byte
	for _, arbiter := range config.Parameters.Arbiters {
		arbiterByte, err := common.HexStringToBytes(arbiter)
		if err != nil {
			return nil, err
		}
		arbitersByte = append(arbitersByte, arbiterByte)
	}

	return arbitersByte, nil
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
