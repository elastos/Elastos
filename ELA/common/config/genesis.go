package config

import (
	"time"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"
)

var (
	zeroHash = common.Uint256{}

	// ELA asset
	elaAsset = types.Transaction{
		TxType:         types.RegisterAsset,
		PayloadVersion: 0,
		Payload: &payload.PayloadRegisterAsset{
			Asset: payload.Asset{
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
		Programs:   []*program.Program{},
	}

	attrNonce = types.NewAttribute(types.Nonce,
		[]byte{77, 101, 130, 33, 7, 252, 253, 82})

	timestamp = time.Unix(time.Date(2017, time.December, 22, 10,
		0, 0, 0, time.UTC).Unix(), 0).Unix()

	ELAAssetID = elaAsset.Hash()
)

func GenesisBlock(foundation common.Uint168) *types.Block {
	coinBase := types.Transaction{
		Version:        0,
		TxType:         types.CoinBase,
		PayloadVersion: payload.PayloadCoinBaseVersion,
		Payload:        &payload.PayloadCoinBase{},
		Attributes:     []*types.Attribute{&attrNonce},
		Inputs: []*types.Input{
			{
				Previous: types.OutPoint{
					TxID:  zeroHash,
					Index: 0x0000,
				},
				Sequence: 0x00000000,
			},
		},
		Outputs: []*types.Output{
			{
				AssetID:     ELAAssetID,
				Value:       3300 * 10000 * 100000000,
				ProgramHash: foundation,
			},
		},
		LockTime: 0,
		Programs: []*program.Program{},
	}

	merkleRoot, _ := crypto.ComputeRoot([]common.Uint256{coinBase.Hash(),
		ELAAssetID})

	return &types.Block{
		Header: types.Header{
			Version:    0,
			Previous:   zeroHash,
			MerkleRoot: merkleRoot,
			Timestamp:  uint32(timestamp),
			Bits:       0x1d03ffff,
			Nonce:      2083236893,
			Height:     0,
		},
		Transactions: []*types.Transaction{&coinBase, &elaAsset},
	}
}
