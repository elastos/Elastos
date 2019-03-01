package auxpow

import (
	"time"

	"github.com/elastos/Elastos.ELA/auxpow"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	ela "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
)

func getSideChainPowTx(msgBlockHash common.Uint256, genesisHash common.Uint256) *ela.Transaction {

	txPayload := &payload.SideChainPow{
		SideBlockHash:   msgBlockHash,
		SideGenesisHash: genesisHash,
	}

	sideChainPowTx := NewSideChainPowTx(txPayload, 0)

	return sideChainPowTx
}

func GenerateSideAuxPow(msgBlockHash common.Uint256, genesisHash common.Uint256) *SideAuxPow {
	sideAuxMerkleBranch := make([]common.Uint256, 0)
	sideAuxMerkleIndex := 0
	sideAuxBlockTx := getSideChainPowTx(msgBlockHash, genesisHash)
	elaBlockHeader := ela.Header{
		Version:    0x7fffffff,
		Previous:   common.EmptyHash,
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

func NewSideChainPowTx(payload *payload.SideChainPow, currentHeight uint32) *ela.Transaction {
	return &ela.Transaction{
		TxType:  ela.SideChainPow,
		Payload: payload,
		Inputs: []*ela.Input{
			{
				Previous: ela.OutPoint{
					TxID:  common.EmptyHash,
					Index: 0x0000,
				},
				Sequence: 0x00000000,
			},
		},
		Attributes: []*ela.Attribute{},
		LockTime:   currentHeight,
		Programs:   []*program.Program{},
	}
}
