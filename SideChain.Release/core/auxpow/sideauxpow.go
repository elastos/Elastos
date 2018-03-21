package auxpow

import (
	. "Elastos.ELA.SideChain/common"
)

type SideAuxPow struct {
	SideAuxMerkleBranch []Uint256
	SideAuxMerkleIndex  uint32
	SideAuxBlockTx      ElaTx
	ElaBlockHeader      ElaBlockHeader
}

func NewSideAuxPow(sideAuxMerkleBranch []Uint256,
	sideAuxMerkleIndex uint32,
	sideAuxBlockTx ElaTx,
	elaBlockHeader ElaBlockHeader) *SideAuxPow {

	return &SideAuxPow{
		SideAuxMerkleBranch: sideAuxMerkleBranch,
		SideAuxMerkleIndex:  sideAuxMerkleIndex,
		SideAuxBlockTx:      sideAuxBlockTx,
		ElaBlockHeader:      elaBlockHeader,
	}
}
