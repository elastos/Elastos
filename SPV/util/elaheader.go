package util

import (
	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/core"
)

// Ensure SideHeader implement BlockHeader interface.
var _ BlockHeader = (*ElaHeader)(nil)

type ElaHeader struct {
	*core.Header
}

func (h *ElaHeader) Previous() common.Uint256 {
	return h.Header.Previous
}

func (h *ElaHeader) Bits() uint32 {
	return h.Header.Bits
}

func (h *ElaHeader) MerkleRoot() common.Uint256 {
	return h.Header.MerkleRoot
}

func (h *ElaHeader) PowHash() common.Uint256 {
	return h.AuxPow.ParBlockHeader.Hash()
}

func NewElaHeader(orgHeader *core.Header) BlockHeader {
	return &ElaHeader{Header: orgHeader}
}
