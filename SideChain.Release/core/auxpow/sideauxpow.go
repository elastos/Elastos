package auxpow

import (
	. "Elastos.ELA.SideChain/common"
)

type SideAuxPow struct {
	SideAuxBlockTx      Uint256
	SideAuxMerkleBranch []Uint256
	SideAuxMerkleIndex  uint32
	MainBlockHeader     ElaBlockHeader
}
