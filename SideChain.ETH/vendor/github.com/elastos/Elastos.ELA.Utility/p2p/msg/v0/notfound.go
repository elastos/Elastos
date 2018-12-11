package v0

import (
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

// Ensure NotFound implement p2p.Message interface.
var _ p2p.Message = (*NotFound)(nil)

type NotFound struct {
	Hash common.Uint256
}

func NewNotFound(hash common.Uint256) *NotFound {
	return &NotFound{Hash: hash}
}

func (msg *NotFound) CMD() string {
	return p2p.CmdNotFound
}

func (msg *NotFound) MaxLength() uint32 {
	return common.UINT256SIZE
}

func (msg *NotFound) Serialize(w io.Writer) error {
	return msg.Hash.Serialize(w)
}

func (msg *NotFound) Deserialize(r io.Reader) error {
	return msg.Hash.Deserialize(r)
}
