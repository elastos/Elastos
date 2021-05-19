package params

import (
	"time"

	"github.com/elastos/Elastos.ELA.SideChain/auxpow"
	"github.com/elastos/Elastos.ELA.SideChain/types"

	"github.com/elastos/Elastos.ELA/common"
	ela "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"

	ntypes "github.com/elastos/Elastos.ELA.SideChain.NeoVM/types"
)

var (
	// ELA coin
	elaAsset = types.Transaction{
		TxType:         types.RegisterAsset,
		PayloadVersion: 0,
		Payload: &types.PayloadRegisterAsset{
			Asset: types.Asset{
				Name:      "ELA",
				Precision: 0x08,
				AssetType: 0x00,
			},
			Amount:     0 * 100000000,
			Controller: common.Uint168{},
		},
		Attributes: []*types.Attribute{},
		Inputs:     []*types.Input{},
		Outputs:    []*types.Output{},
		Programs:   []*types.Program{},
	}

	// The main chain asset ID
	elaAssetId = elaAsset.Hash()

	// genesisHeader
	genesisHeader = types.Header{
		Version:    types.BlockVersion,
		Previous:   common.EmptyHash,
		MerkleRoot: elaAssetId,
		Timestamp: uint32(time.Unix(time.Date(2018, time.December, 24,
			12, 0, 0, 0, time.UTC).Unix(), 0).Unix()),
		Bits:   0x1d03ffff,
		Nonce:  types.GenesisNonce,
		Height: uint32(0),
		SideAuxPow: auxpow.SideAuxPow{
			SideAuxBlockTx: ela.Transaction{
				TxType:         ela.SideChainPow,
				PayloadVersion: payload.SideChainPowVersion,
				Payload:        new(payload.SideChainPow),
			},
		},
	}

	header = ntypes.Header{
		Header:      &genesisHeader,
		ReceiptHash: common.EmptyHash,
		Bloom:       []byte{},
	}
	// genesis block
	genesisBlock = &types.Block{
		Header:       &header,
		Transactions: []*types.Transaction{&elaAsset},
	}
)
