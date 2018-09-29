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
	"github.com/elastos/Elastos.ELA.SideChain/events"
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
}

type Service struct {
	foundation    common.Uint168
	minerAddr     string
	minerInfo     string
	limitBits     uint32
	maxBlockSize  int
	maxTxPerBlock int
	server        server.IServer
	chain         *blockchain.BlockChain
	txMemPool     *mempool.TxPool
	txFeeHelper   *mempool.FeeHelper
	MsgBlock      messageBlock
	Mutex         sync.Mutex
	started       bool
	manualMining  bool

	wg   sync.WaitGroup
	quit chan struct{}
}

func NewService(cfg *Config) *Service {
	pow := Service{
		minerAddr:     cfg.MinerAddr,
		minerInfo:     cfg.MinerInfo,
		limitBits:     cfg.LimitBits,
		maxBlockSize:  cfg.MaxBlockSize,
		maxTxPerBlock: cfg.MaxTxPerBlock,
		server:        cfg.Server,
		chain:         cfg.Chain,
		txMemPool:     cfg.TxMemPool,
		txFeeHelper:   cfg.TxFeeHelper,
		started:       false,
		manualMining:  false,
		MsgBlock:      messageBlock{BlockData: make(map[string]*types.Block)},
	}

	events.Subscribe(func(event *events.Event) {
		switch event.Type {
		case events.ETBlockConnected:
			pow.BlockPersistCompleted(event.Data)

		case events.ETBlockDisconnected:
			pow.RollbackTransaction(event.Data)

		}
	})

	return &pow
}

func (pow *Service) SetMinerAddr(minerAddr string) {
	pow.minerAddr = minerAddr
}

func (pow *Service) GetTransactionCount() int {
	transactionsPool := pow.txMemPool.GetTxsInPool()
	return len(transactionsPool)
}

func (pow *Service) CollectTransactions(msgBlock *types.Block) int {
	txs := 0
	transactionsPool := pow.txMemPool.GetTxsInPool()

	for _, tx := range transactionsPool {
		log.Trace(tx)
		msgBlock.Transactions = append(msgBlock.Transactions, tx)
		txs++
	}
	return txs
}

func (pow *Service) CreateCoinBaseTx(nextBlockHeight uint32, addr string) (*types.Transaction, error) {
	minerProgramHash, err := common.Uint168FromAddress(addr)
	if err != nil {
		return nil, err
	}

	pd := &types.PayloadCoinBase{
		CoinbaseData: []byte(pow.minerInfo),
	}

	txn := blockchain.NewCoinBaseTransaction(pd, pow.chain.GetBestHeight()+1)
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
			AssetID:     pow.chain.AssetID,
			Value:       0,
			ProgramHash: pow.foundation,
		},
		{
			AssetID:     pow.chain.AssetID,
			Value:       0,
			ProgramHash: *minerProgramHash,
		},
	}

	nonce := make([]byte, 8)
	binary.BigEndian.PutUint64(nonce, rand.Uint64())
	txAttr := types.NewAttribute(types.Nonce, nonce)
	txn.Attributes = append(txn.Attributes, &txAttr)
	// log.Trace("txAttr", txAttr)

	return txn, nil
}

type ByFeeDesc []*types.Transaction

func (s ByFeeDesc) Len() int           { return len(s) }
func (s ByFeeDesc) Swap(i, j int)      { s[i], s[j] = s[j], s[i] }
func (s ByFeeDesc) Less(i, j int) bool { return s[i].FeePerKB > s[j].FeePerKB }

func (pow *Service) GenerateBlock(addr string) (*types.Block, error) {
	nextBlockHeight := pow.chain.GetBestHeight() + 1
	coinBaseTx, err := pow.CreateCoinBaseTx(nextBlockHeight, addr)
	if err != nil {
		return nil, err
	}

	header := types.Header{
		Version:    0,
		Previous:   *pow.chain.BestChain.Hash,
		MerkleRoot: common.EmptyHash,
		Timestamp:  uint32(pow.chain.MedianAdjustedTime().Unix()),
		Bits:       pow.limitBits,
		Height:     nextBlockHeight,
		Nonce:      0,
	}

	msgBlock := &types.Block{
		Header:       header,
		Transactions: []*types.Transaction{},
	}

	msgBlock.Transactions = append(msgBlock.Transactions, coinBaseTx)

	pow.GenerateBlockTransactions(msgBlock, coinBaseTx)

	txHash := make([]common.Uint256, 0, len(msgBlock.Transactions))
	for _, tx := range msgBlock.Transactions {
		txHash = append(txHash, tx.Hash())
	}
	txRoot, _ := crypto.ComputeRoot(txHash)
	msgBlock.Header.MerkleRoot = txRoot

	msgBlock.Header.Bits, err = blockchain.CalcNextRequiredDifficulty(pow.chain.BestChain, time.Now())
	log.Info("difficulty: ", msgBlock.Header.Bits)

	return msgBlock, err
}

func (pow *Service) GenerateBlockTransactions(msgBlock *types.Block, coinBaseTx *types.Transaction) {
	nextBlockHeight := pow.chain.GetBestHeight() + 1
	totalTxsSize := coinBaseTx.GetSize()
	txCount := 1
	totalFee := common.Fixed64(0)
	var txsByFeeDesc ByFeeDesc
	txsInPool := pow.txMemPool.GetTxsInPool()
	txsByFeeDesc = make([]*types.Transaction, 0, len(txsInPool))
	for _, v := range txsInPool {
		txsByFeeDesc = append(txsByFeeDesc, v)
	}
	sort.Sort(txsByFeeDesc)

	for _, tx := range txsByFeeDesc {
		totalTxsSize = totalTxsSize + tx.GetSize()
		if totalTxsSize > pow.maxBlockSize {
			break
		}
		if txCount >= pow.maxTxPerBlock {
			break
		}

		if err := blockchain.CheckTransactionFinalize(tx, nextBlockHeight); err != nil {
			continue
		}

		fee := pow.txFeeHelper.GetTxFee(tx, pow.chain.AssetID)
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

func (pow *Service) DiscreteMining(n uint32) ([]*common.Uint256, error) {
	pow.Mutex.Lock()

	if pow.started || pow.manualMining {
		pow.Mutex.Unlock()
		return nil, errors.New("Server is already CPU mining.")
	}

	pow.started = true
	pow.manualMining = true
	pow.Mutex.Unlock()

	log.Tracef("Pow generating %d blocks", n)
	i := uint32(0)
	blockHashes := make([]*common.Uint256, n)
	ticker := time.NewTicker(time.Second * hashUpdateSecs)
	defer ticker.Stop()

	for {
		log.Trace("<================Discrete Mining==============>\n")

		msgBlock, err := pow.GenerateBlock(pow.minerAddr)
		if err != nil {
			log.Trace("generage block err", err)
			continue
		}

		if pow.SolveBlock(msgBlock, ticker) {
			if msgBlock.Header.Height == pow.chain.GetBestHeight()+1 {
				inMainChain, isOrphan, err := pow.chain.AddBlock(msgBlock)
				if err != nil {
					log.Trace(err)
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
	genesisHash, err := pow.chain.GetBlockHash(0)
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
			if !msgBlock.Header.Previous.IsEqual(*pow.chain.BestChain.Hash) {
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
	pow.server.BroadcastMessage(msg.NewBlock(block))
}

func (pow *Service) Start() {
	pow.Mutex.Lock()
	defer pow.Mutex.Unlock()
	if pow.started || pow.manualMining {
		log.Trace("CpuMining is already started")
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

func (pow *Service) RollbackTransaction(v interface{}) {
	if block, ok := v.(*types.Block); ok {
		for _, tx := range block.Transactions[1:] {
			err := pow.txMemPool.MaybeAcceptTransaction(tx)
			if err == nil {
				pow.txMemPool.RemoveTransaction(tx)
			} else {
				log.Error(err)
			}
		}
	}
}

func (pow *Service) BlockPersistCompleted(v interface{}) {
	log.Debug()
	if block, ok := v.(*types.Block); ok {
		log.Infof("persist block: %x", block.Hash())
		err := pow.txMemPool.CleanSubmittedTransactions(block)
		if err != nil {
			log.Warn(err)
		}
	}
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
		log.Trace("<================POW Mining==============>\n")
		//time.Sleep(15 * time.Second)

		msgBlock, err := pow.GenerateBlock(pow.minerAddr)
		if err != nil {
			log.Trace("generage block err", err)
			continue
		}

		//begin to mine the block with POW
		if pow.SolveBlock(msgBlock, ticker) {
			//send the valid block to p2p networkd
			if msgBlock.Header.Height == pow.chain.GetBestHeight()+1 {
				inMainChain, isOrphan, err := pow.chain.AddBlock(msgBlock)
				if err != nil {
					log.Trace(err)
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
