package pow

import (
	"encoding/binary"
	"errors"
	"math"
	"math/rand"
	"sort"
	"sync"
	"time"

	aux "github.com/elastos/Elastos.ELA.SideChain/auxpow"
	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/mempool"
	"github.com/elastos/Elastos.ELA.SideChain/types"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/crypto"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA.Utility/p2p/server"
)

const (
	maxNonce       = ^uint32(0) // 2^32 - 1
	maxExtraNonce  = ^uint64(0) // 2^64 - 1
	hpsUpdateSecs  = 10
	hashUpdateSecs = 15
)

type messageBlock struct {
	BlockData map[string]*types.Block
	Mutex     sync.Mutex
}

type Config struct {
	Foundation    common.Uint168
	MinerAddr     string
	MinerInfo     string
	LimitBits     uint32
	MaxBlockSize  int
	MaxTxPerBlock int
	Server        server.IServer
	Chain         *blockchain.BlockChain
	TxMemPool     *mempool.TxPool
	TxFeeHelper   *mempool.FeeHelper

	CreateCoinBaseTx          func(cfg *Config, nextBlockHeight uint32, addr string) (*types.Transaction, error)
	GenerateBlock             func(cfg *Config) (*types.Block, error)
	GenerateBlockTransactions func(cfg *Config, msgBlock *types.Block, coinBaseTx *types.Transaction)
}

type Service struct {
	Cfg          *Config
	MsgBlock     messageBlock
	Mutex        sync.Mutex
	started      bool
	manualMining bool

	wg   sync.WaitGroup
	quit chan struct{}
}

func NewService(cfg *Config) *Service {
	pow := Service{
		Cfg:          cfg,
		started:      false,
		manualMining: false,
		MsgBlock:     messageBlock{BlockData: make(map[string]*types.Block)},
	}

	return &pow
}

func (pow *Service) SetMinerAddr(minerAddr string) {
	pow.Cfg.MinerAddr = minerAddr
}

func (pow *Service) GetTransactionCount() int {
	transactionsPool := pow.Cfg.TxMemPool.GetTxsInPool()
	return len(transactionsPool)
}

func (pow *Service) CollectTransactions(msgBlock *types.Block) int {
	txs := 0
	transactionsPool := pow.Cfg.TxMemPool.GetTxsInPool()

	for _, tx := range transactionsPool {
		msgBlock.Transactions = append(msgBlock.Transactions, tx)
		txs++
	}
	return txs
}

type ByFeeDesc []*types.Transaction

func (s ByFeeDesc) Len() int           { return len(s) }
func (s ByFeeDesc) Swap(i, j int)      { s[i], s[j] = s[j], s[i] }
func (s ByFeeDesc) Less(i, j int) bool { return s[i].FeePerKB > s[j].FeePerKB }

func (pow *Service) DiscreteMining(n uint32) ([]*common.Uint256, error) {
	pow.Mutex.Lock()

	if pow.started || pow.manualMining {
		pow.Mutex.Unlock()
		return nil, errors.New("Server is already CPU mining.")
	}

	pow.started = true
	pow.manualMining = true
	pow.Mutex.Unlock()

	log.Infof("Pow generating %d blocks", n)
	i := uint32(0)
	blockHashes := make([]*common.Uint256, n)
	ticker := time.NewTicker(time.Second * hashUpdateSecs)
	defer ticker.Stop()

	for {
		log.Info("<================Discrete Mining==============>\n")

		msgBlock, err := pow.Cfg.GenerateBlock(pow.Cfg)
		if err != nil {
			log.Error("generate block err", err)
			continue
		}

		if pow.SolveBlock(msgBlock, ticker) {
			if msgBlock.Header.Height == pow.Cfg.Chain.GetBestHeight()+1 {
				inMainChain, isOrphan, err := pow.Cfg.Chain.AddBlock(msgBlock)
				if err != nil {
					return nil, err
				}
				//TODO if co-mining condition
				if isOrphan || !inMainChain {
					continue
				}
				pow.BroadcastBlock(msgBlock)
				h := msgBlock.Hash()
				blockHashes[i] = &h
				i++
				if i == n {
					pow.Mutex.Lock()
					pow.started = false
					pow.manualMining = false
					pow.Mutex.Unlock()
					return blockHashes, nil
				}
			}
		}
	}
}

func (pow *Service) SolveBlock(msgBlock *types.Block, ticker *time.Ticker) bool {
	genesisHash, err := pow.Cfg.Chain.GetBlockHash(0)
	if err != nil {
		return false
	}
	// fake a mainchain blockheader
	sideAuxPow := aux.GenerateSideAuxPow(msgBlock.Hash(), genesisHash)
	header := msgBlock.Header
	targetDifficulty := blockchain.CompactToBig(header.Bits)

	for i := uint32(0); i <= maxNonce; i++ {
		select {
		case <-ticker.C:
			if !msgBlock.Header.Previous.IsEqual(*pow.Cfg.Chain.BestChain.Hash) {
				return false
			}
			//UpdateBlockTime(messageBlock, m.server.blockManager)

		default:
			// Non-blocking select to fall through
		}

		sideAuxPow.MainBlockHeader.AuxPow.ParBlockHeader.Nonce = i
		hash := sideAuxPow.MainBlockHeader.AuxPow.ParBlockHeader.Hash() // solve parBlockHeader hash
		if blockchain.HashToBig(&hash).Cmp(targetDifficulty) <= 0 {
			msgBlock.Header.SideAuxPow = *sideAuxPow
			return true
		}
	}

	return false
}

func (pow *Service) BroadcastBlock(block *types.Block) {
	pow.Cfg.Server.BroadcastMessage(msg.NewBlock(block))
}

func (pow *Service) Start() {
	pow.Mutex.Lock()
	defer pow.Mutex.Unlock()
	if pow.started || pow.manualMining {
		log.Warn("Mining is already started")
		return
	}

	pow.quit = make(chan struct{})
	pow.wg.Add(1)
	pow.started = true

	go pow.CpuMining()
}

func (pow *Service) Halt() {
	log.Info("POW Stop")
	pow.Mutex.Lock()
	defer pow.Mutex.Unlock()

	if !pow.started || pow.manualMining {
		return
	}

	close(pow.quit)
	pow.wg.Wait()
	pow.started = false
}

func (pow *Service) CpuMining() {
	ticker := time.NewTicker(time.Second * hashUpdateSecs)
	defer ticker.Stop()

out:
	for {
		select {
		case <-pow.quit:
			break out
		default:
			// Non-blocking select to fall through
		}
		log.Info("<================POW Mining==============>\n")
		//time.Sleep(15 * time.Second)

		msgBlock, err := pow.Cfg.GenerateBlock(pow.Cfg)
		if err != nil {
			log.Error("generate block err", err)
			continue
		}

		//begin to mine the block with POW
		if pow.SolveBlock(msgBlock, ticker) {
			//send the valid block to p2p networkd
			if msgBlock.Header.Height == pow.Cfg.Chain.GetBestHeight()+1 {
				inMainChain, isOrphan, err := pow.Cfg.Chain.AddBlock(msgBlock)
				if err != nil {
					continue
				}
				//TODO if co-mining condition
				if isOrphan || !inMainChain {
					continue
				}
				pow.BroadcastBlock(msgBlock)
			}
		}

	}

	pow.wg.Done()
}

func CreateCoinBaseTx(cfg *Config, nextBlockHeight uint32, addr string) (*types.Transaction, error) {
	minerProgramHash, err := common.Uint168FromAddress(addr)
	if err != nil {
		return nil, err
	}

	pd := &types.PayloadCoinBase{
		CoinbaseData: []byte(cfg.MinerInfo),
	}

	txn := blockchain.NewCoinBaseTransaction(pd, cfg.Chain.GetBestHeight()+1)
	txn.Inputs = []*types.Input{
		{
			Previous: types.OutPoint{
				TxID:  common.EmptyHash,
				Index: math.MaxUint16,
			},
			Sequence: math.MaxUint32,
		},
	}
	txn.Outputs = []*types.Output{
		{
			AssetID:     cfg.Chain.AssetID,
			Value:       0,
			ProgramHash: cfg.Foundation,
		},
		{
			AssetID:     cfg.Chain.AssetID,
			Value:       0,
			ProgramHash: *minerProgramHash,
		},
	}

	nonce := make([]byte, 8)
	binary.BigEndian.PutUint64(nonce, rand.Uint64())
	txAttr := types.NewAttribute(types.Nonce, nonce)
	txn.Attributes = append(txn.Attributes, &txAttr)

	return txn, nil
}

func GenerateBlock(cfg *Config) (*types.Block, error) {
	nextBlockHeight := cfg.Chain.GetBestHeight() + 1
	coinBaseTx, err := cfg.CreateCoinBaseTx(cfg, nextBlockHeight, cfg.MinerAddr)
	if err != nil {
		return nil, err
	}

	header := types.Header{
		Version:    0,
		Previous:   *cfg.Chain.BestChain.Hash,
		MerkleRoot: common.EmptyHash,
		Timestamp:  uint32(cfg.Chain.MedianAdjustedTime().Unix()),
		Bits:       cfg.LimitBits,
		Height:     nextBlockHeight,
		Nonce:      0,
	}

	msgBlock := &types.Block{
		Header:       header,
		Transactions: []*types.Transaction{},
	}

	msgBlock.Transactions = append(msgBlock.Transactions, coinBaseTx)

	cfg.GenerateBlockTransactions(cfg, msgBlock, coinBaseTx)

	txHash := make([]common.Uint256, 0, len(msgBlock.Transactions))
	for _, tx := range msgBlock.Transactions {
		txHash = append(txHash, tx.Hash())
	}
	txRoot, _ := crypto.ComputeRoot(txHash)
	msgBlock.Header.MerkleRoot = txRoot

	msgBlock.Header.Bits, err = blockchain.CalcNextRequiredDifficulty(cfg.Chain.BestChain, time.Now())
	log.Info("difficulty: ", msgBlock.Header.Bits)

	return msgBlock, err
}

func GenerateBlockTransactions(cfg *Config, msgBlock *types.Block, coinBaseTx *types.Transaction) {
	nextBlockHeight := cfg.Chain.GetBestHeight() + 1
	totalTxsSize := coinBaseTx.GetSize()
	txCount := 1
	totalFee := common.Fixed64(0)
	var txsByFeeDesc ByFeeDesc
	txsInPool := cfg.TxMemPool.GetTxsInPool()
	txsByFeeDesc = make([]*types.Transaction, 0, len(txsInPool))
	for _, v := range txsInPool {
		txsByFeeDesc = append(txsByFeeDesc, v)
	}
	sort.Sort(txsByFeeDesc)

	for _, tx := range txsByFeeDesc {
		totalTxsSize = totalTxsSize + tx.GetSize()
		if totalTxsSize > cfg.MaxBlockSize {
			break
		}
		if txCount >= cfg.MaxTxPerBlock {
			break
		}

		if err := blockchain.CheckTransactionFinalize(tx, nextBlockHeight); err != nil {
			continue
		}

		fee := cfg.TxFeeHelper.GetTxFee(tx, cfg.Chain.AssetID)
		if fee != tx.Fee {
			continue
		}
		msgBlock.Transactions = append(msgBlock.Transactions, tx)
		totalFee += fee
		txCount++
	}

	reward := totalFee
	rewardFoundation := common.Fixed64(float64(reward) * 0.3)
	msgBlock.Transactions[0].Outputs[0].Value = rewardFoundation
	msgBlock.Transactions[0].Outputs[1].Value = common.Fixed64(reward) - rewardFoundation
}
