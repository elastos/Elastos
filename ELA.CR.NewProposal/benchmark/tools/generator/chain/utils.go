// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package chain

import (
	"sort"
	"time"

	"github.com/elastos/Elastos.ELA/account"
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	crstate "github.com/elastos/Elastos.ELA/cr/state"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/dpos/state"
	"github.com/elastos/Elastos.ELA/elanet/pact"
	"github.com/elastos/Elastos.ELA/pow"
	"github.com/elastos/Elastos.ELA/utils/test"
)

func newBlockChain(path string, params *config.Params,
	interrupt <-chan struct{}) (*blockchain.BlockChain, error) {
	log.NewDefault(test.NodeLogPath, 1, 0, 0)

	committee := crstate.NewCommittee(params)
	arbiters, err := state.NewArbitrators(
		params, committee, nil)

	chainStore, err := blockchain.NewChainStore(path, params)
	if err != nil {
		return nil, err
	}
	chain, err := blockchain.New(chainStore, params, arbiters.State, committee)
	if err != nil {
		return nil, err
	}

	if err = chain.Init(interrupt); err != nil {
		return nil, err
	}
	if err = chain.MigrateOldDB(interrupt, func(uint32) {},
		func() {}, path, params); err != nil {
		return nil, err
	}

	arbiters.RegisterFunction(chain.GetHeight,
		func(height uint32) (*types.Block, error) {
			hash, err := chain.GetBlockHash(height)
			if err != nil {
				return nil, err
			}
			block, err := chainStore.GetFFLDB().GetBlock(hash)
			if err != nil {
				return nil, err
			}
			blockchain.CalculateTxsFee(block.Block)
			return block.Block, nil
		}, chain.UTXOCache.GetTxReference)

	initDefaultLedger(chain, chainStore, arbiters, committee)
	return chain, nil
}

func initDefaultLedger(chain *blockchain.BlockChain,
	store blockchain.IChainStore, arbiter state.Arbitrators,
	committee *crstate.Committee) {
	blockchain.DefaultLedger = &blockchain.Ledger{
		Blockchain:  chain,
		Store:       store,
		Arbitrators: arbiter,
		Committee:   committee,
	}
}

func generateChainParams(ac *account.Account) *config.Params {
	proto := config.DefaultParams.RegNet()
	proto.GenesisBlock = newGenesisBlock(ac)
	proto.Foundation = ac.ProgramHash
	blockchain.FoundationAddress = ac.ProgramHash
	return proto.InstantBlock()
}

func newGenesisBlock(ac *account.Account) *types.Block {
	attrNonce := types.NewAttribute(types.Nonce,
		[]byte{77, 101, 130, 33, 7, 252, 253, 82})
	genesisTime, _ := time.Parse(time.RFC3339, "2017-12-22T10:00:00Z")

	coinBase := types.Transaction{
		Version:        0,
		TxType:         types.CoinBase,
		PayloadVersion: payload.CoinBaseVersion,
		Payload:        &payload.CoinBase{},
		Attributes:     []*types.Attribute{&attrNonce},
		Inputs: []*types.Input{
			{
				Previous: types.OutPoint{
					TxID:  common.Uint256{},
					Index: 0x0000,
				},
				Sequence: 0x00000000,
			},
		},
		Outputs: []*types.Output{
			{
				AssetID:     config.ELAAssetID,
				Value:       3300 * 10000 * 100000000,
				ProgramHash: ac.ProgramHash,
			},
		},
		LockTime: 0,
		Programs: []*program.Program{},
	}

	merkleRoot, _ := crypto.ComputeRoot([]common.Uint256{coinBase.Hash(),
		config.ELAAssetID})

	return &types.Block{
		Header: types.Header{
			Version:    0,
			Previous:   common.Uint256{},
			MerkleRoot: merkleRoot,
			Timestamp:  uint32(genesisTime.Unix()),
			Bits:       0x1d03ffff,
			Nonce:      2083236893,
			Height:     0,
		},
		Transactions: []*types.Transaction{
			&coinBase,
			{
				TxType:         types.RegisterAsset,
				PayloadVersion: 0,
				Payload: &payload.RegisterAsset{
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
			}},
	}
}

func quickGenerateBlock(pow *pow.Service, prevHash *common.Uint256,
	txs []*types.Transaction, minerAddr string, params *config.Params,
	height uint32) (*types.Block, error) {
	coinBaseTx, err := pow.CreateCoinbaseTx(minerAddr, height)
	if err != nil {
		return nil, err
	}

	header := types.Header{
		Version:    0,
		Previous:   *prevHash,
		MerkleRoot: common.EmptyHash,
		Timestamp:  uint32(time.Now().Unix()),
		Bits:       params.PowLimitBits,
		Height:     height,
		Nonce:      0,
	}

	msgBlock := &types.Block{
		Header:       header,
		Transactions: []*types.Transaction{},
	}

	msgBlock.Transactions = append(msgBlock.Transactions, coinBaseTx)
	totalTxsSize := coinBaseTx.GetSize()
	txCount := 1
	totalTxFee := common.Fixed64(0)
	isHighPriority := func(tx *types.Transaction) bool {
		if tx.IsIllegalTypeTx() || tx.IsInactiveArbitrators() ||
			tx.IsSideChainPowTx() || tx.IsUpdateVersion() ||
			tx.IsActivateProducerTx() {
			return true
		}
		return false
	}

	sort.Slice(txs, func(i, j int) bool {
		if isHighPriority(txs[i]) {
			return true
		}
		if isHighPriority(txs[j]) {
			return false
		}
		return txs[i].FeePerKB > txs[j].FeePerKB
	})

	for _, tx := range txs {
		size := totalTxsSize + tx.GetSize()
		if size > int(pact.MaxBlockContextSize) {
			continue
		}
		totalTxsSize = size

		if !blockchain.IsFinalizedTransaction(tx, height) {
			continue
		}
		msgBlock.Transactions = append(msgBlock.Transactions, tx)
		totalTxFee += tx.Fee
		txCount++
	}

	totalReward := totalTxFee + params.RewardPerBlock
	pow.AssignCoinbaseTxRewards(msgBlock, totalReward)

	txHash := make([]common.Uint256, 0, len(msgBlock.Transactions))
	for _, tx := range msgBlock.Transactions {
		txHash = append(txHash, tx.Hash())
	}
	txRoot, _ := crypto.ComputeRoot(txHash)
	msgBlock.Header.MerkleRoot = txRoot

	log.Infof("generated block: %d", msgBlock.Height)
	return msgBlock, err
}
