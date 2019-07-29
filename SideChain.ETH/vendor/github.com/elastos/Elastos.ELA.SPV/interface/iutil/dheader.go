package iutil

import (
	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
)

// Ensure Header implement BlockHeader interface.
var _ util.BlockHeader = (*DHeader)(nil)

// DHeader represents a DPOS header.
type DHeader struct {
	*types.DPOSHeader
}

func (h *DHeader) Previous() common.Uint256 {
	return h.Header.Previous
}

func (h *DHeader) Bits() uint32 {
	return h.Header.Bits
}

func (h *DHeader) MerkleRoot() common.Uint256 {
	return h.Header.MerkleRoot
}

func (h *DHeader) PowHash() common.Uint256 {
	return h.AuxPow.ParBlockHeader.Hash()
}

func NewDHeader(orgHeader *types.DPOSHeader) util.BlockHeader {
	return &DHeader{orgHeader}
}
