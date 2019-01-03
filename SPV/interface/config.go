package _interface

import (
	"time"

	"github.com/elastos/Elastos.ELA.SPV/interface/iutil"
	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"
)

func newBlockHeader() util.BlockHeader {
	return iutil.NewHeader(&types.Header{})
}

func newTransaction() util.Transaction {
	return iutil.NewTx(&types.Transaction{})
}

// GenesisHeader creates a specific genesis header by the given
// foundation address.
func GenesisHeader(foundation *common.Uint168) util.BlockHeader {
	// Genesis time
	genesisTime := time.Date(2017, time.December, 22, 10, 0, 0, 0, time.UTC)

	// header
	header := types.Header{
		Version:    0,
		Previous:   common.EmptyHash,
		MerkleRoot: common.EmptyHash,
		Timestamp:  uint32(genesisTime.Unix()),
		Bits:       0x1d03ffff,
		Nonce:      types.GenesisNonce,
		Height:     uint32(0),
	}

	// ELA coin
	elaCoin := &types.Transaction{
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

	coinBase := &types.Transaction{
		TxType:         types.CoinBase,
		PayloadVersion: payload.PayloadCoinBaseVersion,
		Payload:        new(payload.PayloadCoinBase),
		Inputs: []*types.Input{
			{
				Previous: types.OutPoint{
					TxID:  common.EmptyHash,
					Index: 0x0000,
				},
				Sequence: 0x00000000,
			},
		},
		Attributes: []*types.Attribute{},
		LockTime:   0,
		Programs:   []*program.Program{},
	}

	coinBase.Outputs = []*types.Output{
		{
			AssetID:     elaCoin.Hash(),
			Value:       3300 * 10000 * 100000000,
			ProgramHash: *foundation,
		},
	}

	nonce := []byte{0x4d, 0x65, 0x82, 0x21, 0x07, 0xfc, 0xfd, 0x52}
	txAttr := types.NewAttribute(types.Nonce, nonce)
	coinBase.Attributes = append(coinBase.Attributes, &txAttr)

	transactions := []*types.Transaction{coinBase, elaCoin}
	hashes := make([]common.Uint256, 0, len(transactions))
	for _, tx := range transactions {
		hashes = append(hashes, tx.Hash())
	}
	header.MerkleRoot, _ = crypto.ComputeRoot(hashes)

	return iutil.NewHeader(&header)
}
