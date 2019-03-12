package interfaces

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
)

type Arbitrators interface {
	ForceChange(height uint32) error
	ProcessBlock(block *types.Block, confirm *payload.Confirm)
	RollbackTo(height uint32) error

	IsArbitrator(pk []byte) bool
	GetArbitrators() [][]byte
	GetCandidates() [][]byte
	GetNextArbitrators() [][]byte
	GetNextCandidates() [][]byte
	GetNeedConnectArbiters() map[string]struct{}
	GetDutyIndex() uint32

	GetCRCArbitrators() []config.CRCArbiter
	IsCRCArbitrator(pk []byte) bool
	IsCRCArbitratorProgramHash(hash *common.Uint168) bool

	GetArbitratorsProgramHashes() []*common.Uint168
	GetCandidatesProgramHashes() []*common.Uint168

	GetOnDutyArbitratorByHeight(height uint32) []byte
	GetOnDutyArbitrator() []byte
	GetNextOnDutyArbitrator(offset uint32) []byte

	GetArbitersCount() uint32
	GetArbitersMajorityCount() uint32
	HasArbitersMajorityCount(num uint32) bool
	HasArbitersMinorityCount(num uint32) bool
}
