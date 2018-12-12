package types

import (
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

type IllegalDataType byte

const (
	IllegalBlock    IllegalDataType = 0x00
	IllegalProposal IllegalDataType = 0x01
	IllegalVote     IllegalDataType = 0x02
)

type DposIllegalData interface {
	Type() IllegalDataType
	GetBlockHeight() uint32
	Serialize(w io.Writer) error
	Deserialize(r io.Reader) error
	Hash() common.Uint256
}
