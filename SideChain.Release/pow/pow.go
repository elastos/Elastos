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
	. "github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/core"
	"github.com/elastos/Elastos.ELA.SideChain/events"
	"github.com/elastos/Elastos.ELA.SideChain/log"
	"github.com/elastos/Elastos.ELA.SideChain/protocol"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/crypto"
)

var TaskCh chan bool

const (
	maxNonce       = ^uint32(0) // 2^32 - 1
	maxExtraNonce  = ^uint64(0) // 2^64 - 1
	hpsUpdateSecs  = 10
	hashUpdateSecs = 15
)

var (
	TargetTimePerBlock = int64(config.Parameters.ChainParam.TargetTimePerBlock / time.Second)
)

type messageBlock struct {
	BlockData map[string]*core.Block
	Mutex     sync.Mutex
}

type PowService struct {
	PayToAddr    string
	MsgBlock     messageBlock
	Mutex        sync.Mutex
	started      bool
	manualMining bool
	LocalNode    protocol.Noder

	wg   sync.WaitGroup
	quit chan struct{}

	GetTransactionCount       func() int
	CollectTransactions       func(msgBlock *core.Block) int
	CreateCoinBaseTx          func(nextBlockHeight uint32, addr string) (*core.Transaction, error)
	GenerateBlock             func(addr string) (*core.Block, error)
	GenerateBlockTransactions func(msgBlock *core.Block, coinBaseTx *core.Transaction)
	DiscreteMining            func(n uint32) ([]*common.Uint256, error)
	SolveBlock                func(msgBlock *core.Block, ticker *time.Ticker) bool
	BroadcastBlock            func(msgBlock *core.Block) error
	Start                     func()
	Halt                      func()
	RollbackTransaction       func(v interface{})
	BlockPersistCompleted     func(v interface{})
	CpuMining                 func()
}

func NewPowService(localNode protocol.Noder) *PowService {
	pow := &PowService{
		PayToAddr:    config.Parameters.PowConfiguration.PayToAddr,
		started:      false,
		manualMining: false,
		MsgBlock:     messageBlock{BlockData: make(map[string]*core.Block)},
		LocalNode:    localNode,
	}
	pow.Init()
	pow.InitPowServiceSubscriber()

	log.Trace("pow Service Init succeed")
	return pow
}

func (pow *PowService) Init() {
	pow.GetTransactionCount = pow.GetTransactionCountImpl
	pow.CollectTransactions = pow.CollectTransactionsImpl
	pow.CreateCoinBaseTx = pow.CreateCoinBaseTxImpl
	pow.GenerateBlock = pow.GenerateBlockImpl
	pow.GenerateBlockTransactions = pow.GenerateBlockTransactionsImpl
	pow.DiscreteMining = pow.DiscreteMiningImpl
	pow.SolveBlock = pow.SolveBlockImpl
	pow.BroadcastBlock = pow.BroadcastBlockImpl
	pow.Start = pow.StartImpl
	pow.Halt = pow.HaltImpl
	pow.RollbackTransaction = pow.RollbackTransactionImpl
	pow.BlockPersistCompleted = pow.BlockPersistCompletedImpl
	pow.CpuMining = pow.CpuMiningImpl
}

func (pow *PowService) InitPowServiceSubscriber() {
	events.Subscribe(func(event *events.Event) {
		switch event.Type {
		case events.ETBlockConnected:
			pow.BlockPersistCompleted(event.Data)

		case events.ETBlockDisconnected:
			pow.RollbackTransaction(event.Data)

		}
	})
}

func (pow *PowService) GetTransactionCountImpl() int {
	transactionsPool := pow.LocalNode.GetTxsInPool()
	return len(transactionsPool)
}

func (pow *PowService) CollectTransactionsImpl(msgBlock *core.Block) int {
	txs := 0
	transactionsPool := pow.LocalNode.GetTxsInPool()

	for _, tx := range transactionsPool {
		log.Trace(tx)
		msgBlock.Transactions = append(msgBlock.Transactions, tx)
		txs++
	}
	return txs
}

func (pow *PowService) CreateCoinBaseTxImpl(nextBlockHeight uint32, addr string) (*core.Transaction, error) {
	minerProgramHash, err := common.Uint168FromAddress(addr)
	if err != nil {
		return nil, err
	}

	pd := &core.PayloadCoinBase{
		CoinbaseData: []byte(config.Parameters.PowConfiguration.MinerInfo),
	}

	txn := NewCoinBaseTransaction(pd, DefaultChain.GetBestHeight()+1)
	txn.Inputs = []*core.Input{
		{
			Previous: core.OutPoint{
				TxID:  common.EmptyHash,
				Index: math.MaxUint16,
			},
			Sequence: math.MaxUint32,
		},
	}
	txn.Outputs = []*core.Output{
		{
			AssetID:     DefaultChain.AssetID,
			Value:       0,
			ProgramHash: FoundationAddress,
		},
		{
			AssetID:     DefaultChain.AssetID,
			Value:       0,
			ProgramHash: *minerProgramHash,
		},
	}

	nonce := make([]byte, 8)
	binary.BigEndian.PutUint64(nonce, rand.Uint64())
	txAttr := core.NewAttribute(core.Nonce, nonce)
	txn.Attributes = append(txn.Attributes, &txAttr)
	// log.Trace("txAttr", txAttr)

	return txn, nil
}

type ByFeeDesc []*core.Transaction

func (s ByFeeDesc) Len() int           { return len(s) }
func (s ByFeeDesc) Swap(i, j int)      { s[i], s[j] = s[j], s[i] }
func (s ByFeeDesc) Less(i, j int) bool { return s[i].FeePerKB > s[j].FeePerKB }

func (pow *PowService) GenerateBlockImpl(addr string) (*core.Block, error) {
	nextBlockHeight := DefaultChain.GetBestHeight() + 1
	coinBaseTx, err := pow.CreateCoinBaseTx(nextBlockHeight, addr)
	if err != nil {
		return nil, err
	}

	header := core.Header{
		Version:    0,
		Previous:   *DefaultChain.BestChain.Hash,
		MerkleRoot: common.EmptyHash,
		Timestamp:  uint32(DefaultChain.MedianAdjustedTime().Unix()),
		Bits:       config.Parameters.ChainParam.PowLimitBits,
		Height:     nextBlockHeight,
		Nonce:      0,
	}

	msgBlock := &core.Block{
		Header:       header,
		Transactions: []*core.Transaction{},
	}

	msgBlock.Transactions = append(msgBlock.Transactions, coinBaseTx)

	pow.GenerateBlockTransactions(msgBlock, coinBaseTx)

	txHash := make([]common.Uint256, 0, len(msgBlock.Transactions))
	for _, tx := range msgBlock.Transactions {
		txHash = append(txHash, tx.Hash())
	}
	txRoot, _ := crypto.ComputeRoot(txHash)
	msgBlock.Header.MerkleRoot = txRoot

	msgBlock.Header.Bits, err = CalcNextRequiredDifficulty(DefaultChain.BestChain, time.Now())
	log.Info("difficulty: ", msgBlock.Header.Bits)

	return msgBlock, err
}

func (pow *PowService) GenerateBlockTransactionsImpl(msgBlock *core.Block, coinBaseTx *core.Transaction) {
	nextBlockHeight := DefaultChain.GetBestHeight() + 1
	totalTxsSize := coinBaseTx.GetSize()
	txCount := 1
	totalFee := common.Fixed64(0)
	var txsByFeeDesc ByFeeDesc
	txsInPool := pow.LocalNode.GetTxsInPool()
	txsByFeeDesc = make([]*core.Transaction, 0, len(txsInPool))
	for _, v := range txsInPool {
		txsByFeeDesc = append(txsByFeeDesc, v)
	}
	sort.Sort(txsByFeeDesc)

	for _, tx := range txsByFeeDesc {
		totalTxsSize = totalTxsSize + tx.GetSize()
		if totalTxsSize > config.Parameters.MaxBlockSize {
			break
		}
		if txCount >= config.Parameters.MaxTxInBlock {
			break
		}

		if !BlockValidator.IsFinalizedTransaction(tx, nextBlockHeight) {
			continue
		}

		fee := TxFeeHelper.GetTxFee(tx, DefaultChain.AssetID)
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

func (pow *PowService) DiscreteMiningImpl(n uint32) ([]*common.Uint256, error) {
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

		msgBlock, err := pow.GenerateBlock(pow.PayToAddr)
		if err != nil {
			log.Trace("generage block err", err)
			continue
		}

		if pow.SolveBlock(msgBlock, ticker) {
			if msgBlock.Header.Height == DefaultChain.GetBestHeight()+1 {
				inMainChain, isOrphan, err := DefaultChain.AddBlock(msgBlock)
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

func (pow *PowService) SolveBlockImpl(msgBlock *core.Block, ticker *time.Ticker) bool {
	genesisHash, err := DefaultChain.GetBlockHash(0)
	if err != nil {
		return false
	}
	// fake a mainchain blockheader
	sideAuxPow := aux.GenerateSideAuxPow(msgBlock.Hash(), genesisHash)
	header := msgBlock.Header
	targetDifficulty := CompactToBig(header.Bits)

	for i := uint32(0); i <= maxNonce; i++ {
		select {
		case <-ticker.C:
			if !msgBlock.Header.Previous.IsEqual(*DefaultChain.BestChain.Hash) {
				return false
			}
			//UpdateBlockTime(messageBlock, m.server.blockManager)

		default:
			// Non-blocking select to fall through
		}

		sideAuxPow.MainBlockHeader.AuxPow.ParBlockHeader.Nonce = i
		hash := sideAuxPow.MainBlockHeader.AuxPow.ParBlockHeader.Hash() // solve parBlockHeader hash
		if HashToBig(&hash).Cmp(targetDifficulty) <= 0 {
			msgBlock.Header.SideAuxPow = *sideAuxPow
			return true
		}
	}

	return false
}

func (pow *PowService) BroadcastBlockImpl(msgBlock *core.Block) error {
	return pow.LocalNode.Relay(nil, msgBlock)
}

func (pow *PowService) StartImpl() {
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

func (pow *PowService) HaltImpl() {
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

func (pow *PowService) RollbackTransactionImpl(v interface{}) {
	if block, ok := v.(*core.Block); ok {
		for _, tx := range block.Transactions[1:] {
			err := pow.LocalNode.MaybeAcceptTransaction(tx)
			if err == nil {
				pow.LocalNode.RemoveTransaction(tx)
			} else {
				log.Error(err)
			}
		}
	}
}

func (pow *PowService) BlockPersistCompletedImpl(v interface{}) {
	log.Debug()
	if block, ok := v.(*core.Block); ok {
		log.Infof("persist block: %x", block.Hash())
		err := pow.LocalNode.CleanSubmittedTransactions(block)
		if err != nil {
			log.Warn(err)
		}
		pow.LocalNode.SetHeight(uint64(DefaultChain.GetBestHeight()))
	}
}

func (pow *PowService) CpuMiningImpl() {
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

		msgBlock, err := pow.GenerateBlock(pow.PayToAddr)
		if err != nil {
			log.Trace("generage block err", err)
			continue
		}

		//begin to mine the block with POW
		if pow.SolveBlock(msgBlock, ticker) {
			//send the valid block to p2p networkd
			if msgBlock.Header.Height == DefaultChain.GetBestHeight()+1 {
				inMainChain, isOrphan, err := DefaultChain.AddBlock(msgBlock)
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
