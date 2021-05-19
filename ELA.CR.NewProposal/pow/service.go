// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package pow

import (
	"encoding/binary"
	"errors"
	"fmt"
	"math"
	"math/rand"
	"sort"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA/auxpow"
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/dpos/state"
	"github.com/elastos/Elastos.ELA/elanet/pact"
	"github.com/elastos/Elastos.ELA/mempool"
)

const (
	maxNonce       = ^uint32(0) // 2^32 - 1
	updateInterval = 30 * time.Second
	maxTxPerBlock  = 100
)

type Config struct {
	PayToAddr      string
	MinerInfo      string
	Chain          *blockchain.BlockChain
	ChainParams    *config.Params
	TxMemPool      *mempool.TxPool
	BlkMemPool     *mempool.BlockPool
	BroadcastBlock func(block *types.Block)
	Arbitrators    state.Arbitrators
}

type AuxBlockPool struct {
	mutex       sync.RWMutex
	mapNewBlock map[common.Uint256]*types.Block
}

func (p *AuxBlockPool) AppendBlock(block *types.Block) {
	p.mutex.Lock()
	defer p.mutex.Unlock()

	p.mapNewBlock[block.Hash()] = block
}

func (p *AuxBlockPool) ClearBlock() {
	p.mutex.Lock()
	defer p.mutex.Unlock()

	for key := range p.mapNewBlock {
		delete(p.mapNewBlock, key)
	}
}

func (p *AuxBlockPool) GetBlock(hash common.Uint256) (*types.Block, bool) {
	p.mutex.RLock()
	defer p.mutex.RUnlock()

	block, ok := p.mapNewBlock[hash]
	return block, ok
}

type Service struct {
	PayToAddr   string
	MinerInfo   string
	chain       *blockchain.BlockChain
	chainParams *config.Params
	txMemPool   *mempool.TxPool
	blkMemPool  *mempool.BlockPool
	broadcast   func(block *types.Block)
	arbiters    state.Arbitrators

	mutex           sync.Mutex
	started         bool
	discreteMining  bool
	auxBlockPool    AuxBlockPool
	preChainHeight  uint32
	preTime         time.Time
	currentAuxBlock *types.Block

	wg   sync.WaitGroup
	quit chan struct{}

	lock      sync.Mutex
	lastBlock *types.Block
}

func (pow *Service) GetDefaultTxVersion(height uint32) types.TransactionVersion {
	var v types.TransactionVersion = 0
	// when block height greater than H2 use the version TxVersion09
	if height >= pow.chainParams.PublicDPOSHeight {
		v = types.TxVersion09
	}
	return v
}

func (pow *Service) CreateCoinbaseTx(minerAddr string, height uint32) (*types.Transaction, error) {
	crRewardAddr := pow.chainParams.Foundation
	if height >= pow.chainParams.CRCommitteeStartHeight {
		crRewardAddr = pow.chainParams.CRAssetsAddress
	}

	minerProgramHash, err := common.Uint168FromAddress(minerAddr)
	if err != nil {
		return nil, err
	}

	tx := &types.Transaction{
		Version:        pow.GetDefaultTxVersion(height),
		TxType:         types.CoinBase,
		PayloadVersion: payload.CoinBaseVersion,
		Payload: &payload.CoinBase{
			Content: []byte(pow.MinerInfo),
		},
		Inputs: []*types.Input{
			{
				Previous: types.OutPoint{
					TxID:  common.EmptyHash,
					Index: math.MaxUint16,
				},
				Sequence: math.MaxUint32,
			},
		},
		Outputs: []*types.Output{
			{
				AssetID:     config.ELAAssetID,
				Value:       0,
				ProgramHash: crRewardAddr,
				Type:        types.OTNone,
				Payload:     &outputpayload.DefaultOutput{},
			},
			{
				AssetID:     config.ELAAssetID,
				Value:       0,
				ProgramHash: *minerProgramHash,
				Type:        types.OTNone,
				Payload:     &outputpayload.DefaultOutput{},
			},
		},
		Attributes: []*types.Attribute{},
		LockTime:   height,
	}

	nonce := make([]byte, 8)
	binary.BigEndian.PutUint64(nonce, rand.Uint64())
	txAttr := types.NewAttribute(types.Nonce, nonce)
	tx.Attributes = append(tx.Attributes, &txAttr)

	return tx, nil
}

func (pow *Service) AssignCoinbaseTxRewards(block *types.Block, totalReward common.Fixed64) error {
	// main version >= H2
	if block.Height >= pow.chainParams.PublicDPOSHeight {
		rewardCyberRepublic := common.Fixed64(math.Ceil(float64(totalReward) * 0.3))
		rewardDposArbiter := common.Fixed64(math.Ceil(float64(totalReward) * 0.35))
		rewardMergeMiner := common.Fixed64(totalReward) - rewardCyberRepublic - rewardDposArbiter

		if rewards := pow.arbiters.GetArbitersRoundReward(); len(rewards) > 0 {

			var dposChange common.Fixed64
			var err error
			if dposChange, err = pow.distributeDPOSReward(block.Transactions[0],
				rewards); err != nil {
				return err
			}
			rewardMergeMiner += dposChange
		}

		block.Transactions[0].Outputs[0].Value = rewardCyberRepublic
		block.Transactions[0].Outputs[1].Value = rewardMergeMiner
		return nil
	}

	// version [0, H2)
	// PoW miners and DPoS are each equally allocated 35%. The remaining 30% goes to the Cyber Republic fund
	rewardCyberRepublic := common.Fixed64(float64(totalReward) * 0.3)
	rewardMergeMiner := common.Fixed64(float64(totalReward) * 0.35)
	rewardDposArbiter := common.Fixed64(totalReward) - rewardCyberRepublic - rewardMergeMiner
	block.Transactions[0].Outputs[0].Value = rewardCyberRepublic
	block.Transactions[0].Outputs[1].Value = rewardMergeMiner
	block.Transactions[0].Outputs = append(block.Transactions[0].Outputs, &types.Output{
		AssetID:     config.ELAAssetID,
		Value:       rewardDposArbiter,
		ProgramHash: blockchain.FoundationAddress,
	})
	return nil
}

func (pow *Service) distributeDPOSReward(coinBaseTx *types.Transaction,
	rewards map[common.Uint168]common.Fixed64) (common.Fixed64, error) {

	for ownerHash, reward := range rewards {
		coinBaseTx.Outputs = append(coinBaseTx.Outputs, &types.Output{
			AssetID:     config.ELAAssetID,
			Value:       reward,
			ProgramHash: ownerHash,
			Type:        types.OTNone,
			Payload:     &outputpayload.DefaultOutput{},
		})
	}
	return pow.arbiters.GetFinalRoundChange(), nil
}

func (pow *Service) GenerateBlock(minerAddr string,
	txPerBlock int) (*types.Block, error) {
	bestChain := pow.chain.BestChain
	nextBlockHeight := bestChain.Height + 1
	coinBaseTx, err := pow.CreateCoinbaseTx(minerAddr, nextBlockHeight)
	if err != nil {
		return nil, err
	}

	header := types.Header{
		Version:    0,
		Previous:   *pow.chain.BestChain.Hash,
		MerkleRoot: common.EmptyHash,
		Timestamp:  uint32(pow.chain.MedianAdjustedTime().Unix()),
		Bits:       pow.chainParams.PowLimitBits,
		Height:     nextBlockHeight,
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
	txs := pow.txMemPool.GetTxsInPool()

	isHighPriority := func(tx *types.Transaction) bool {
		if tx.IsIllegalTypeTx() || tx.IsInactiveArbitrators() ||
			tx.IsSideChainPowTx() || tx.IsUpdateVersion() ||
			tx.IsActivateProducerTx() || tx.IsCRCAppropriationTx() ||
			tx.IsCRAssetsRectifyTx() {
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

	var proposalsUsedAmount common.Fixed64
	for _, tx := range txs {
		size := totalTxsSize + tx.GetSize()
		if size > int(pact.MaxBlockContextSize) {
			continue
		}
		totalTxsSize = size
		if txCount >= txPerBlock {
			log.Warn("txCount reached max MaxTxPerBlock")
			break
		}

		if !blockchain.IsFinalizedTransaction(tx, nextBlockHeight) {
			continue
		}
		_, errCode := pow.chain.CheckTransactionContext(nextBlockHeight, tx, proposalsUsedAmount)
		if errCode != nil {
			log.Warn("check transaction context failed, wrong transaction:", tx.Hash().String())
			continue
		}
		msgBlock.Transactions = append(msgBlock.Transactions, tx)
		totalTxFee += tx.Fee
		if tx.IsCRCProposalTx() {
			blockchain.RecordCRCProposalAmount(&proposalsUsedAmount, tx)
		}
		txCount++
	}

	totalReward := totalTxFee + pow.chainParams.RewardPerBlock
	pow.AssignCoinbaseTxRewards(msgBlock, totalReward)

	txHash := make([]common.Uint256, 0, len(msgBlock.Transactions))
	for _, tx := range msgBlock.Transactions {
		txHash = append(txHash, tx.Hash())
	}
	txRoot, _ := crypto.ComputeRoot(txHash)
	msgBlock.Header.MerkleRoot = txRoot

	msgBlock.Header.Bits, err = pow.chain.CalcNextRequiredDifficulty(bestChain, time.Now())
	log.Infof("block height %d with difficulty: %d",
		msgBlock.Height, msgBlock.Header.Bits)

	return msgBlock, err
}

func (pow *Service) CreateAuxBlock(payToAddr string) (*types.Block, error) {
	pow.mutex.Lock()
	defer pow.mutex.Unlock()

	if pow.chain.GetHeight() == 0 || pow.preChainHeight != pow.chain.GetHeight() ||
		time.Now().After(pow.preTime.Add(updateInterval)) {

		if pow.preChainHeight != pow.chain.GetHeight() {
			// Clear old blocks since they're obsolete now.
			pow.currentAuxBlock = nil
			pow.auxBlockPool.ClearBlock()
		}

		// Create new block with nonce = 0
		auxBlock, err := pow.GenerateBlock(payToAddr, maxTxPerBlock)
		if err != nil {
			return nil, err
		}

		// Update state only when CreateNewBlock succeeded
		pow.preChainHeight = pow.chain.GetHeight()
		pow.preTime = time.Now()

		// Save
		pow.currentAuxBlock = auxBlock
		pow.auxBlockPool.AppendBlock(auxBlock)
	}

	// At this point, currentAuxBlock is always initialised: If we make it here without creating
	// a new block above, it means that, in particular, preChainHeight == ServerNode.Height().
	// But for that to happen, we must already have created a currentAuxBlock in a previous call,
	// as preChainHeight is initialised only when currentAuxBlock is.
	if pow.currentAuxBlock == nil {
		return nil, fmt.Errorf("no block cached")
	}

	return pow.currentAuxBlock, nil
}

func (pow *Service) SubmitAuxBlock(hash *common.Uint256, auxPow *auxpow.AuxPow) error {
	pow.mutex.Lock()
	defer pow.mutex.Unlock()

	msgAuxBlock, ok := pow.auxBlockPool.GetBlock(*hash)
	if !ok {
		log.Debug("[json-rpc:SubmitAuxBlock] block hash unknown", hash)
		return fmt.Errorf("block hash unknown")
	}

	msgAuxBlock.Header.AuxPow = *auxPow
	_, _, err := pow.blkMemPool.AddDposBlock(&types.DposBlock{
		Block: msgAuxBlock,
	})
	return err
}

func (pow *Service) DiscreteMining(n uint32) ([]*common.Uint256, error) {
	pow.mutex.Lock()

	if pow.started || pow.discreteMining {
		pow.mutex.Unlock()
		return nil, errors.New("node is mining")
	}

	pow.started = true
	pow.discreteMining = true
	pow.mutex.Unlock()

	log.Debugf("Pow generating %d blocks", n)
	i := uint32(0)
	blockHashes := make([]*common.Uint256, 0)

	log.Info("<================Discrete Mining==============>\n")
	for {
		msgBlock, err := pow.GenerateBlock(pow.PayToAddr, maxTxPerBlock)
		if err != nil {
			log.Warn("Generate block failed, ", err.Error())
			continue
		}
		log.Info("Generate block, " + msgBlock.Hash().String())

		if pow.SolveBlock(msgBlock, nil) {
			if msgBlock.Header.Height == pow.chain.GetHeight()+1 {

				_, _, err := pow.blkMemPool.AddDposBlock(&types.DposBlock{
					Block: msgBlock,
				})
				if err != nil {
					continue
				}

				h := msgBlock.Hash()
				blockHashes = append(blockHashes, &h)
				i++
				if i == n {
					pow.mutex.Lock()
					pow.started = false
					pow.discreteMining = false
					pow.mutex.Unlock()
					return blockHashes, nil
				}
			}
		}
	}
}

func (pow *Service) SolveBlock(msgBlock *types.Block, lastBlockHash *common.Uint256) bool {
	ticker := time.NewTicker(updateInterval)
	defer ticker.Stop()
	// fake a btc blockheader and coinbase
	auxPow := auxpow.GenerateAuxPow(msgBlock.Hash())
	header := msgBlock.Header
	targetDifficulty := blockchain.CompactToBig(header.Bits)
	for i := uint32(0); i <= maxNonce; i++ {
		select {
		case <-ticker.C:
			log.Info("five second countdown ends. Re-generate block.")
			return false
		default:
			// Non-blocking select to fall through
		}

		auxPow.ParBlockHeader.Nonce = i
		hash := auxPow.ParBlockHeader.Hash() // solve parBlockHeader hash
		if blockchain.HashToBig(&hash).Cmp(targetDifficulty) <= 0 {
			msgBlock.Header.AuxPow = *auxPow
			return true
		}
	}

	return false
}

func (pow *Service) Start() {
	pow.mutex.Lock()
	defer pow.mutex.Unlock()
	if pow.started || pow.discreteMining {
		log.Debug("cpuMining is already started")
	}

	pow.quit = make(chan struct{})
	pow.wg.Add(1)
	pow.started = true

	go pow.cpuMining()
}

func (pow *Service) Halt() {
	log.Info("POW Stop")
	pow.mutex.Lock()
	defer pow.mutex.Unlock()

	if !pow.started || pow.discreteMining {
		return
	}

	close(pow.quit)
	pow.wg.Wait()
	pow.started = false
}

func (pow *Service) cpuMining() {
out:
	for {
		select {
		case <-pow.quit:
			break out
		default:
			// Non-blocking select to fall through
		}
		log.Debug("<================Packing Block==============>")
		//time.Sleep(15 * time.Second)

		msgBlock, err := pow.GenerateBlock(pow.PayToAddr, maxTxPerBlock)
		if err != nil {
			log.Debug("generage block err", err)
			continue
		}

		//begin to mine the block with POW
		if pow.SolveBlock(msgBlock, nil) {
			log.Info("<================Solved Block==============>")
			//send the valid block to p2p networkd
			if msgBlock.Header.Height == pow.chain.GetHeight()+1 {

				inMainChain, isOrphan, err := pow.blkMemPool.AddDposBlock(&types.DposBlock{
					Block: msgBlock,
				})
				if err != nil {
					log.Debug(err)
					continue
				}

				if isOrphan || !inMainChain {
					continue
				}
			}
		}

	}

	pow.wg.Done()
}

func NewService(cfg *Config) *Service {
	block, _ := cfg.Chain.GetBlockByHash(*cfg.Chain.BestChain.Hash)
	pow := &Service{
		PayToAddr:      cfg.PayToAddr,
		MinerInfo:      cfg.MinerInfo,
		chain:          cfg.Chain,
		chainParams:    cfg.ChainParams,
		txMemPool:      cfg.TxMemPool,
		blkMemPool:     cfg.BlkMemPool,
		broadcast:      cfg.BroadcastBlock,
		arbiters:       cfg.Arbitrators,
		started:        false,
		discreteMining: false,
		auxBlockPool:   AuxBlockPool{mapNewBlock: make(map[common.Uint256]*types.Block)},
		lastBlock:      block,
	}

	return pow
}
