package payload

import (
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

type IllegalDataType byte

const (
	IllegalBlock             IllegalDataType = 0x00
	IllegalProposal          IllegalDataType = 0x01
	IllegalVote              IllegalDataType = 0x02
	SidechainIllegalProposal IllegalDataType = 0x03
	SidechainIllegalVote     IllegalDataType = 0x04
	InactiveArbitrator       IllegalDataType = 0x05
)

type DPOSIllegalData interface {
	Type() IllegalDataType
	GetBlockHeight() uint32
	Serialize(w io.Writer, version byte) error
	Deserialize(r io.Reader, version byte) error
	Hash() common.Uint256
}
