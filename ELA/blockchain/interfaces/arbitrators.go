package interfaces

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
)

type ArbitratorsListener interface {
	OnNewElection(arbiters [][]byte)
}

type NewBlocksListener interface {
	OnBlockReceived(b *types.Block, confirmed bool)
	OnConfirmReceived(p *types.DPosProposalVoteSlot)
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
