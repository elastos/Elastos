package auxpow

import (
	"time"

	"github.com/elastos/Elastos.ELA/auxpow"

	. "github.com/elastos/Elastos.ELA/core"
	. "github.com/elastos/Elastos.ELA.Utility/common"
)

func getSideMiningTx(msgBlockHash Uint256, genesisHash Uint256) *Transaction {

	txPayload := &PayloadSideMining{
		SideBlockHash:   msgBlockHash,
		SideGenesisHash: genesisHash,
	}

	sideMiningTx := NewSideMiningTx(txPayload, 0)

	return sideMiningTx
}

func GenerateSideAuxPow(msgBlockHash Uint256, genesisHash Uint256) *SideAuxPow {
	sideAuxMerkleBranch := make([]Uint256, 0)
	sideAuxMerkleIndex := 0
	sideAuxBlockTx := getSideMiningTx(msgBlockHash, genesisHash)
	elaBlockHeader := Header{
		Version:    0x7fffffff,
		Previous:   EmptyHash,
		MerkleRoot: sideAuxBlockTx.Hash(),
		Timestamp:  uint32(time.Now().Unix()),
		Bits:       0,
		Nonce:      0,
		Height:     0,
	}

	elahash := elaBlockHeader.Hash()
	// fake a btc blockheader and coinbase
	newAuxPow := auxpow.GenerateAuxPow(elahash)
	elaBlockHeader.AuxPow = *newAuxPow

	sideAuxPow := NewSideAuxPow(
		sideAuxMerkleBranch,
		sideAuxMerkleIndex,
		*sideAuxBlockTx,
		elaBlockHeader,
	)

	return sideAuxPow
}

func NewSideMiningTx(payload *PayloadSideMining, currentHeight uint32) *Transaction {
	return &Transaction{
		TxType:  SideMining,
		Payload: payload,
		Inputs: []*Input{
			{
				Previous: OutPoint{
					TxID:  EmptyHash,
					Index: 0x0000,
				},
				Sequence: 0x00000000,
			},
		},
		Attributes: []*Attribute{},
		LockTime:   currentHeight,
		Programs:   []*Program{},
	}
}
