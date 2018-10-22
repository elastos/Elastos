package blockchain

import (
	"errors"
	"time"

	"github.com/elastos/Elastos.ELA.SideChain/auxpow"
	"github.com/elastos/Elastos.ELA.SideChain/types"

	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/crypto"
	"github.com/elastos/Elastos.ELA/core"
)

func GenesisBlock() (*types.Block, error) {
	// ELA coin
	elaCoin := types.Transaction{
		TxType:         types.RegisterAsset,
		PayloadVersion: 0,
		Payload: &types.PayloadRegisterAsset{
			Asset: types.Asset{
				Name:      "ELA",
				Precision: 0x08,
				AssetType: 0x00,
			},
			Amount:     0 * 100000000,
			Controller: Uint168{},
		},
		Attributes: []*types.Attribute{},
		Inputs:     []*types.Input{},
		Outputs:    []*types.Output{},
		Programs:   []*types.Program{},
	}

	// header
	header := types.Header{
		Version:    types.BlockVersion,
		Previous:   EmptyHash,
		MerkleRoot: EmptyHash,
		Timestamp:  uint32(time.Unix(time.Date(2018, time.June, 30, 12, 0, 0, 0, time.UTC).Unix(), 0).Unix()),
		Bits:       0x1d03ffff,
		Nonce:      types.GenesisNonce,
		Height:     uint32(0),
		SideAuxPow: auxpow.SideAuxPow{
			SideAuxBlockTx: core.Transaction{
				TxType:         core.SideChainPow,
				PayloadVersion: core.SideChainPowPayloadVersion,
				Payload:        new(core.PayloadSideChainPow),
			},
		},
	}

	//block
	block := &types.Block{
		Header:       header,
		Transactions: []*types.Transaction{&elaCoin},
	}
	hashes := make([]Uint256, 0, len(block.Transactions))
	for _, tx := range block.Transactions {
		hashes = append(hashes, tx.Hash())
	}
	var err error
	block.Header.MerkleRoot, err = crypto.ComputeRoot(hashes)
	if err != nil {
		return nil, errors.New("[GenesisBlock] ,BuildMerkleRoot failed.")
	}

	return block, nil
}
