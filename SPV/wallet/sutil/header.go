package sutil

import (
	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/core"
)

// Ensure Header implement BlockHeader interface.
var _ util.BlockHeader = (*Header)(nil)

type Header struct {
	*core.Header
}

func (h *Header) Previous() common.Uint256 {
	return h.Header.Previous
}

func (h *Header) Bits() uint32 {
	return h.Header.Bits
}

func (h *Header) MerkleRoot() common.Uint256 {
	return h.Header.MerkleRoot
}

func (h *Header) PowHash() common.Uint256 {
	return h.AuxPow.ParBlockHeader.Hash()
}

func NewHeader(orgHeader *core.Header) util.BlockHeader {
	return &Header{Header: orgHeader}
}

func NewEmptyHeader() util.BlockHeader {
	return &Header{&core.Header{}}
}
