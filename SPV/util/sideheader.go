package util

import (
	"github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA.Utility/common"
)

// Ensure SideHeader implement BlockHeader interface.
var _ BlockHeader = (*SideHeader)(nil)

type SideHeader struct {
	*types.Header
}

func (h *SideHeader) Previous() common.Uint256 {
	return h.Header.Previous
}

func (h *SideHeader) Bits() uint32 {
	return h.Header.Bits
}

func (h *SideHeader) MerkleRoot() common.Uint256 {
	return h.Header.MerkleRoot
}

func (h *SideHeader) PowHash() common.Uint256 {
	return h.SideAuxPow.MainBlockHeader.Hash()
}

func NewSideHeader(orgHeader *types.Header) BlockHeader {
	return &SideHeader{Header: orgHeader}
}
