package pow

import (
	"encoding/binary"
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
	"github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/events"
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
	Versions       interfaces.HeightVersions
	BroadcastBlock func(block *types.Block)
}

type blockPool struct {
	mutex       sync.RWMutex
	mapNewBlock map[common.Uint256]*types.Block
}

func (p *blockPool) AppendBlock(block *types.Block) {
	p.mutex.Lock()
	defer p.mutex.Unlock()

	p.mapNewBlock[block.Hash()] = block
}

func (p *blockPool) ClearBlock() {
	p.mutex.Lock()
	defer p.mutex.Unlock()

	for key := range p.mapNewBlock {
		delete(p.mapNewBlock, key)
	}
}

func (p *blockPool) GetBlock(hash common.Uint256) (*types.Block, bool) {
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
	versions    interfaces.HeightVersions
	txMemPool   *mempool.TxPool
	broadcast   func(block *types.Block)

	mutex           sync.Mutex
	started         bool
	discreteMining  bool
	blockPool       blockPool
	preChainHeight  uint32
	preTime         time.Time
	currentAuxBlock *types.Block

	blockChan chan common.Uint256
	wg        sync.WaitGroup
	quit      chan struct{}

	lock          sync.Mutex
	lastBlock     *types.Block
	needBroadCast bool
}

func (pow *Service) CreateCoinbaseTx(minerAddr string) (*types.Transaction, error) {
	minerProgramHash, err := common.Uint168FromAddress(minerAddr)
	if err != nil {
		return nil, err
	}

	currentHeight := pow.chain.GetHeight() + 1
	version := types.TransactionVersion(pow.versions.GetDefaultTxVersion(currentHeight))
	tx := &types.Transaction{
		Version:        version,
		TxType:         types.CoinBase,
		PayloadVersion: payload.PayloadCoinBaseVersion,
		Payload: &payload.PayloadCoinBase{
			CoinbaseData: []byte(pow.MinerInfo),
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
				AssetID:       config.ELAAssetID,
				Value:         0,
				ProgramHash:   pow.chainParams.Foundation,
				OutputType:    types.DefaultOutput,
				OutputPayload: &outputpayload.DefaultOutput{},
			},
			{
				AssetID:       config.ELAAssetID,
				Value:         0,
				ProgramHash:   *minerProgramHash,
				OutputType:    types.DefaultOutput,
				OutputPayload: &outputpayload.DefaultOutput{},
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

func (pow *Service) GenerateBlock(minerAddr string) (*types.Block, error) {
	bestChain := pow.chain.BestChain
	nextBlockHeight := bestChain.Height + 1
	coinBaseTx, err := pow.CreateCoinbaseTx(minerAddr)
	if err != nil {
		return nil, err
	}

	header := types.Header{
		Version:    pow.versions.GetDefaultBlockVersion(nextBlockHeight),
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
		return txs[i].FeePerKB > txs[j].FeePerKB
	})

	for _, tx := range txs {
		totalTxsSize = totalTxsSize + tx.GetSize()
		if totalTxsSize > config.Parameters.MaxBlockSize {
			break
		}
		if txCount >= config.Parameters.MaxTxsInBlock {
			break
		}

		if !blockchain.IsFinalizedTransaction(tx, nextBlockHeight) {
			continue
		}
		if errCode := blockchain.CheckTransactionContext(nextBlockHeight, tx); errCode != errors.Success {
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

	blockReward := blockchain.RewardAmountPerBlock
	totalReward := totalTxFee + blockReward
	if err := pow.versions.AssignCoinbaseTxRewards(msgBlock, totalReward); err != nil {
		return nil, err
	}

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
			pow.blockPool.ClearBlock()
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
		pow.blockPool.AppendBlock(auxBlock)
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

	msgAuxBlock, ok := pow.blockPool.GetBlock(*hash)
	if !ok {
		log.Debug("[json-rpc:SubmitAuxBlock] block hash unknown", hash)
		return fmt.Errorf("block hash unknown")
	}

	msgAuxBlock.Header.AuxPow = *auxPow
	_, _, err := pow.versions.AddDposBlock(&types.DposBlock{
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

				inMainChain, isOrphan, err := pow.versions.AddDposBlock(&types.DposBlock{
					BlockFlag: true,
					Block:     msgBlock,
				})
				if err != nil {
					continue
				}

				if isOrphan || !inMainChain {
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
		case hash := <-pow.blockChan:
			log.Info("new block received, hash:", hash, " ledger has been changed. Re-generate block.")
			pow.needBroadCast = true
			if pow.versions.GetDefaultBlockVersion(msgBlock.Height) != 1 ||
				lastBlockHash == nil || !lastBlockHash.IsEqual(hash) {
				return false
			}
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
	pow.needBroadCast = true

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
	blockVersion := pow.versions.GetDefaultBlockVersion(pow.chain.BestChain.Height + 1)
	switch blockVersion {
	case 0:
		pow.cpuMiningV0()
	default:
		pow.cpuMiningMain()
	}
}

func (pow *Service) cpuMiningMain() {
out:
	for {
		select {
		case <-pow.quit:
			break out
		default:
			// Non-blocking select to fall through
		}
		log.Info("<================Packing Block==============>")

		pow.lock.Lock()
		msgBlock, err := pow.GenerateBlock(pow.PayToAddr)
		if err != nil {
			log.Error("generage block err", err)
			pow.lock.Unlock()
			continue
		}
		pow.lock.Unlock()

		//begin to mine the block with POW
		hash := pow.lastBlock.Hash()
		if pow.SolveBlock(msgBlock, &hash) {
			log.Info("<================Solved Block==============>")
			//send the valid block to p2p networkd
			pow.lock.Lock()
			if !pow.needBroadCast {
				hash := <-pow.blockChan
				if !hash.IsEqual(pow.lastBlock.Hash()) {
					pow.needBroadCast = true
					pow.lock.Unlock()
					continue
				}
			}
			pow.needBroadCast = false
			pow.lock.Unlock()

			time.Sleep(time.Second * 1)

			inMainChain, isOrphan, err := pow.versions.AddDposBlock(&types.DposBlock{
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

			pow.broadcast(msgBlock)
			hash := msgBlock.Hash()
			node := blockchain.NewBlockNode(&msgBlock.Header, &hash)
			node.InMainChain = true
			prevHash := &msgBlock.Previous
			if parentNode, ok := pow.chain.LookupNodeInIndex(prevHash); ok {
				node.WorkSum = node.WorkSum.Add(parentNode.WorkSum, node.WorkSum)
				node.Parent = parentNode
			}
			pow.lock.Lock()
			pow.lastBlock = msgBlock
			pow.lock.Unlock()
		}
	}

	pow.wg.Done()
}

func (pow *Service) cpuMiningV0() {
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

				inMainChain, isOrphan, err := pow.versions.AddDposBlock(&types.DposBlock{
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
		PayToAddr:   cfg.PayToAddr,
		chain:       cfg.Chain,
		chainParams: cfg.ChainParams,
		versions:    cfg.Versions,
		txMemPool:   cfg.TxMemPool,
		broadcast:   cfg.BroadcastBlock,

		started:        false,
		discreteMining: false,
		blockPool:      blockPool{mapNewBlock: make(map[common.Uint256]*types.Block)},
		lastBlock:      block,
	}

	events.Subscribe(func(e *events.Event) {
		switch e.Type {
		case events.ETBlockConnected:
			if pow.started {
				pow.blockChan <- e.Data.(*types.Block).Hash()
			}
		}
	})

	return pow
}
