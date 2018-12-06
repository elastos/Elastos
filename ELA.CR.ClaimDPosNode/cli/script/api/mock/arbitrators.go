package mock

import (
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/core/types"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type ArbitratorsMock interface {
	blockchain.Arbitrators

	GetDutyChangeCount() uint32
	SetDutyChangeCount(count uint32)
}

func NewArbitratorsMock(arbitersByte [][]byte, changeCount, majorityCount uint32) ArbitratorsMock {
	return &arbitrators{arbitersByte, 0, majorityCount}
}

//mock object of arbitrators
type arbitrators struct {
	currentArbitrators [][]byte
	dutyChangedCount   uint32
	majorityCount      uint32
}

func (a *arbitrators) GetDutyChangeCount() uint32 {
	return a.dutyChangedCount
}

func (a *arbitrators) SetDutyChangeCount(count uint32) {
	a.dutyChangedCount = count
}

func (a *arbitrators) OnBlockReceived(b *types.Block, confirmed bool) {
	panic("implement me")
}

func (a *arbitrators) OnConfirmReceived(p *types.DPosProposalVoteSlot) {
	panic("implement me")
}

func (a *arbitrators) StartUp() error {
	panic("implement me")
}

func (a *arbitrators) ForceChange() error {
	panic("implement me")
}

func (a *arbitrators) GetArbitrators() [][]byte {
	return a.currentArbitrators
}

func (a *arbitrators) GetCandidates() [][]byte {
	panic("implement me")
}

func (a *arbitrators) GetNextArbitrators() [][]byte {
	panic("implement me")
}

func (a *arbitrators) GetNextCandidates() [][]byte {
	panic("implement me")
}

func (a *arbitrators) GetArbitratorsProgramHashes() []*common.Uint168 {
	panic("implement me")
}

func (a *arbitrators) GetCandidatesProgramHashes() []*common.Uint168 {
	panic("implement me")
}

func (a *arbitrators) GetOnDutyArbitrator() []byte {
	return a.GetNextOnDutyArbitrator(0)
}

func (a *arbitrators) GetNextOnDutyArbitrator(offset uint32) []byte {
	index := (a.dutyChangedCount + offset) % uint32(len(a.currentArbitrators))
	return a.currentArbitrators[index]
}

func (a *arbitrators) HasArbitersMajorityCount(num uint32) bool {
	//note "num > majorityCount" in real logic
	return num >= a.majorityCount
}

func (a *arbitrators) HasArbitersMinorityCount(num uint32) bool {
	//note "num >= uint32(len(arbitratorsPublicKeys))-majorityCount" in real logic
	return num > uint32(len(a.currentArbitrators))-a.majorityCount
}

func (a *arbitrators) RegisterListener(listener blockchain.ArbitratorsListener) {
	panic("implement me")
}

func (a *arbitrators) UnregisterListener(listener blockchain.ArbitratorsListener) {
	panic("implement me")
}
