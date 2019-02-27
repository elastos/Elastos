package interfaces

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types/payload"
)

type Arbitrators interface {
	ForceChange() error
	IncreaseChainHeight(height uint32)
	DecreaseChainHeight(height uint32)

	IsArbitrator(pk []byte) bool
	GetArbitrators() [][]byte
	GetCandidates() [][]byte
	GetNextArbitrators() [][]byte
	GetNextCandidates() [][]byte

	GetCRCArbitrators() []config.CRCArbitratorParams
	IsCRCArbitrator(pk []byte) bool
	IsCRCArbitratorProgramHash(hash *common.Uint168) bool

	GetArbitratorsProgramHashes() []*common.Uint168
	GetCandidatesProgramHashes() []*common.Uint168

	GetPreviousOnDutyArbitrator() []byte
	GetOnDutyArbitrator() []byte
	GetNextOnDutyArbitrator(offset uint32) []byte

	GetArbitersCount() uint32
	GetArbitersMajorityCount() uint32
	HasArbitersMajorityCount(num uint32) bool
	HasArbitersMinorityCount(num uint32) bool

	GetInactiveArbitrators(confirm *payload.Confirm,
		onDutyArbitrator []byte) (result []string)
}
