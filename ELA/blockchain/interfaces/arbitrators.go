package interfaces

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
)

type Arbitrators interface {
	Start() error
	ForceChange() error

	GetArbitrators() [][]byte
	GetCandidates() [][]byte
	GetNextArbitrators() [][]byte
	GetNextCandidates() [][]byte

	GetCRCArbitrators() []config.CRCArbitratorParams
	IsCRCArbitrator(pk []byte) bool
	IsCRCArbitratorProgramHash(hash *common.Uint168) bool

	GetArbitratorsProgramHashes() []*common.Uint168
	GetCandidatesProgramHashes() []*common.Uint168

	GetOnDutyArbitrator() []byte
	GetNextOnDutyArbitrator(offset uint32) []byte

	GetArbitersCount() uint32
	GetArbitersMajorityCount() uint32
	HasArbitersMajorityCount(num uint32) bool
	HasArbitersMinorityCount(num uint32) bool

	GetActiveDposPeers() map[string]string
	TryEnterEmergency(blockTime uint32) bool
	GetLastConfirmedBlockTimeStamp() uint32
}
