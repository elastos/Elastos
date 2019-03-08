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
	"github.com/elastos/Elastos.ELA/blockchain/interfaces"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/elanet/pact"
	elaerr "github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/mempool"
)

const (
	maxNonce       = ^uint32(0) // 2^32 - 1
	updateInterval = 5 * time.Second
)

type Config struct {
	PayToAddr      string
	MinerInfo      string
	Chain          *blockchain.BlockChain
	ChainParams    *config.Params
	TxMemPool      *mempool.TxPool
	BlkMemPool     *mempool.BlockPool
	BroadcastBlock func(block *types.Block)
	Arbitrators    interfaces.Arbitrators
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
	arbiters    interfaces.Arbitrators

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

func GetDefaultTxVersion(height uint32) types.TransactionVersion {
	var v types.TransactionVersion = 0
	// when block height greater than H2 use the version TxVersion09
	if height >= config.Parameters.HeightVersions[3] {
		v = types.TxVersion09
	}
	return v
}

func (pow *Service) CreateCoinbaseTx(minerAddr string) (*types.Transaction, error) {
	minerProgramHash, err := common.Uint168FromAddress(minerAddr)
	if err != nil {
		return nil, err
	}

	currentHeight := pow.chain.GetHeight() + 1
	tx := &types.Transaction{
		Version:        GetDefaultTxVersion(currentHeight),
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
				ProgramHash: pow.chainParams.Foundation,
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
		LockTime:   currentHeight,
	}

	nonce := make([]byte, 8)
	binary.BigEndian.PutUint64(nonce, rand.Uint64())
	txAttr := types.NewAttribute(types.Nonce, nonce)
	tx.Attributes = append(tx.Attributes, &txAttr)

	return tx, nil
}

func (pow *Service) AssignCoinbaseTxRewards(height uint32, block *types.Block, totalReward common.Fixed64) error {
	// main version >= H2
	if height >= config.Parameters.HeightVersions[3] {
		rewardCyberRepublic := common.Fixed64(math.Ceil(float64(totalReward) * 0.3))
		rewardDposArbiter := common.Fixed64(float64(totalReward) * 0.35)

		var dposChange common.Fixed64
		var err error
		if dposChange, err = pow.distributeDposReward(block.Transactions[0], rewardDposArbiter); err != nil {
			return err
		}
		rewardMergeMiner := common.Fixed64(totalReward) - rewardCyberRepublic - rewardDposArbiter + dposChange
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

func (pow *Service) distributeDposReward(coinBaseTx *types.Transaction, reward common.Fixed64) (common.Fixed64, error) {
	arbitratorsHashes :=
		pow.arbiters.GetArbitratorsProgramHashes()
	if len(arbitratorsHashes) == 0 {
		return 0, errors.New("not found arbiters when distributeDposReward")
	}
	candidatesHashes := pow.arbiters.GetCandidatesProgramHashes()

	totalBlockConfirmReward := float64(reward) * 0.25
	totalTopProducersReward := float64(reward) * 0.75
	individualBlockConfirmReward := common.Fixed64(math.Floor(totalBlockConfirmReward / float64(len(arbitratorsHashes))))
	individualProducerReward := common.Fixed64(math.Floor(totalTopProducersReward / float64(int(config.Parameters.ArbiterConfiguration.NormalArbitratorsCount)+len(candidatesHashes))))

	realDposReward := common.Fixed64(0)
	for _, v := range arbitratorsHashes {
		reward := individualBlockConfirmReward + individualProducerReward
		if pow.arbiters.IsCRCArbitratorProgramHash(v) {
			reward = individualBlockConfirmReward
		}

		coinBaseTx.Outputs = append(coinBaseTx.Outputs, &types.Output{
			AssetID:     config.ELAAssetID,
			Value:       reward,
			ProgramHash: *v,
			Type:        types.OTNone,
			Payload:     &outputpayload.DefaultOutput{},
		})

		realDposReward += reward
	}

	for _, v := range candidatesHashes {

		coinBaseTx.Outputs = append(coinBaseTx.Outputs, &types.Output{
			AssetID:     config.ELAAssetID,
			Value:       individualProducerReward,
			ProgramHash: *v,
			Type:        types.OTNone,
			Payload:     &outputpayload.DefaultOutput{},
		})

		realDposReward += individualProducerReward
	}

	change := reward - realDposReward
	if change < 0 {
		return 0, errors.New("real dpos reward more than reward limit")
	}
	return change, nil
}

func (pow *Service) GenerateBlock(minerAddr string) (*types.Block, error) {
	bestChain := pow.chain.BestChain
	nextBlockHeight := bestChain.Height + 1
	coinBaseTx, err := pow.CreateCoinbaseTx(minerAddr)
	if err != nil {
		return nil, err
	}

	header := types.Header{
		Version:    0,
		Previous:   *pow.chain.BestChain.Hash,
		MerkleRoot: common.EmptyHash,
		Timestamp:  uint32(pow.chain.MedianAdjustedTime().Unix()),
		Bits:       config.Parameters.ChainParam.PowLimitBits,
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
	sort.Slice(txs, func(i, j int) bool {
		if txs[i].IsIllegalTypeTx() || txs[i].IsInactiveArbitrators() {
			return true
		}
		if txs[j].IsIllegalTypeTx() || txs[j].IsInactiveArbitrators() {
			return false
		}
		return txs[i].FeePerKB > txs[j].FeePerKB
	})

	for _, tx := range txs {
		totalTxsSize = totalTxsSize + tx.GetSize()
		if totalTxsSize > pact.MaxBlockSize {
			break
		}
		if txCount >= config.Parameters.MaxTxsInBlock {
			break
		}

		if !blockchain.IsFinalizedTransaction(tx, nextBlockHeight) {
			continue
		}
		if errCode := pow.chain.CheckTransactionContext(nextBlockHeight, tx); errCode != elaerr.Success {
			log.Warn("check transaction context failed, wrong transaction:", tx.Hash().String())
			continue
		}
		fee := blockchain.GetTxFee(tx, config.ELAAssetID)
		if fee != tx.Fee {
			continue
		}
		msgBlock.Transactions = append(msgBlock.Transactions, tx)
		totalTxFee += fee
		txCount++
	}

	totalReward := totalTxFee + pow.chainParams.RewardPerBlock
	pow.AssignCoinbaseTxRewards(pow.chain.GetHeight(), msgBlock, totalReward)

	txHash := make([]common.Uint256, 0, len(msgBlock.Transactions))
	for _, tx := range msgBlock.Transactions {
		txHash = append(txHash, tx.Hash())
	}
	txRoot, _ := crypto.ComputeRoot(txHash)
	msgBlock.Header.MerkleRoot = txRoot

	msgBlock.Header.Bits, err = pow.chain.CalcNextRequiredDifficulty(bestChain, time.Now())
	log.Info("difficulty: ", msgBlock.Header.Bits)

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
		auxBlock, err := pow.GenerateBlock(payToAddr)
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
	_, _, err := pow.blkMemPool.AddDposBlock(pow.chain.GetHeight(), &types.DposBlock{
		BlockFlag: true,
		Block:     msgAuxBlock,
	})
	return err
}

func (pow *Service) DiscreteMining(n uint32) ([]*common.Uint256, error) {
	pow.mutex.Lock()

	if pow.started || pow.discreteMining {
		pow.mutex.Unlock()
		return nil, fmt.Errorf("Server is already CPU mining.")
	}

	pow.started = true
	pow.discreteMining = true
	pow.mutex.Unlock()

	log.Debugf("Pow generating %d blocks", n)
	i := uint32(0)
	blockHashes := make([]*common.Uint256, 0)

	for {
		log.Debug("<================Discrete Mining==============>\n")

		msgBlock, err := pow.GenerateBlock(pow.PayToAddr)
		if err != nil {
			log.Debug("generage block err", err)
			continue
		}

		if pow.SolveBlock(msgBlock, nil) {
			if msgBlock.Header.Height == pow.chain.GetHeight()+1 {

				_, _, err := pow.blkMemPool.AddDposBlock(pow.chain.GetHeight(), &types.DposBlock{
					BlockFlag: true,
					Block:     msgBlock,
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

		msgBlock, err := pow.GenerateBlock(pow.PayToAddr)
		if err != nil {
			log.Debug("generage block err", err)
			continue
		}

		//begin to mine the block with POW
		if pow.SolveBlock(msgBlock, nil) {
			log.Info("<================Solved Block==============>")
			//send the valid block to p2p networkd
			if msgBlock.Header.Height == pow.chain.GetHeight()+1 {

				inMainChain, isOrphan, err := pow.blkMemPool.AddDposBlock(pow.chain.GetHeight(), &types.DposBlock{
					BlockFlag: true,
					Block:     msgBlock,
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
