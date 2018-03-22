package pow

import (
	"time"

	. "Elastos.ELA.SideChain/common"
	"Elastos.ELA.SideChain/core/auxpow"
)

func getSideMiningTx(msgBlockHash Uint256) *auxpow.ElaTx {
	s := auxpow.SideMiningPayload{
		SideBlockHash: msgBlockHash,
	}
	sideMiningTx := auxpow.NewSideMiningTx(s, 0)

	return sideMiningTx
}

func generateSideAuxPow(msgBlockHash Uint256) *auxpow.SideAuxPow {
	sideAuxMerkleBranch := make([]Uint256, 0)
	sideAuxMerkleIndex := uint32(0)
	sideAuxBlockTx := getSideMiningTx(msgBlockHash)
	elaBlockHeader := auxpow.ElaBlockHeader{
		Version:          0x7fffffff,
		PrevBlockHash:    Uint256{},
		TransactionsRoot: sideAuxBlockTx.Hash(),
		Timestamp:        uint32(time.Now().Unix()),
		Bits:             0, // do not care about parent block diff
		Nonce:            0, // to be solved
		Height:           0,
		AuxPow:           auxpow.AuxPow{},
	}

	elahash := elaBlockHeader.Hash()
	newAuxPow := generateAuxPow(elahash)
	elaBlockHeader.AuxPow = *newAuxPow

	sideAuxPow := auxpow.NewSideAuxPow(
		sideAuxMerkleBranch,
		sideAuxMerkleIndex,
		*sideAuxBlockTx,
		elaBlockHeader,
	)

	return sideAuxPow
}
