package state

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/dpos/p2p"
)

type Arbitrators interface {
	ProcessBlock(block *types.Block, confirm *payload.Confirm)
	ProcessSpecialTxPayload(p types.Payload, height uint32) error
	RollbackTo(height uint32) error

	IsArbitrator(pk []byte) bool
	GetArbitrators() [][]byte
	GetCandidates() [][]byte
	GetNextArbitrators() [][]byte
	GetNextCandidates() [][]byte
	GetNeedConnectArbiters(height uint32) map[string]*p2p.PeerAddr
	GetDutyIndex() uint32

	GetCRCArbitrators() []config.CRCArbiter
	IsCRCArbitrator(pk []byte) bool
	IsCRCArbitratorProgramHash(hash *common.Uint168) bool
	IsCRCArbitratorNodePublicKey(nodePublicKeyHex string) bool

	GetCurrentOwnerProgramHashes() []*common.Uint168
	GetCandidateOwnerProgramHashes() []*common.Uint168
	GetOwnerVotes(programHash *common.Uint168) common.Fixed64
	GetTotalVotesInRound() common.Fixed64

	GetOnDutyArbitrator() []byte
	GetNextOnDutyArbitrator(offset uint32) []byte

	GetArbitersCount() uint32
	GetArbitersMajorityCount() uint32
	HasArbitersMajorityCount(num uint32) bool
	HasArbitersMinorityCount(num uint32) bool
}
