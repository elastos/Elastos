package pow

import (
	"encoding/binary"
	"errors"
	"math"
	"math/rand"
	"sort"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA/auxpow"
	. "github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/config"
	. "github.com/elastos/Elastos.ELA/core"
	. "github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/events"
	"github.com/elastos/Elastos.ELA/log"
	"github.com/elastos/Elastos.ELA/node"

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

type msgBlock struct {
	BlockData map[string]*Block
	Mutex     sync.Mutex
}

type PowService struct {
	PayToAddr      string
	MsgBlock       msgBlock
	Mutex          sync.Mutex
	Started        bool
	discreteMining bool

	blockPersistCompletedSubscriber events.Subscriber
	RollbackTransactionSubscriber   events.Subscriber

	wg   sync.WaitGroup
	quit chan struct{}
}

func (pow *PowService) CollectTransactions(MsgBlock *Block) int {
	txs := 0
	transactionsPool := node.LocalNode.GetTransactionPool(true)

	for _, tx := range transactionsPool {
		log.Trace(tx)
		MsgBlock.Transactions = append(MsgBlock.Transactions, tx)
		txs++
	}
	return txs
}

func (pow *PowService) CreateCoinbaseTx(nextBlockHeight uint32, addr string) (*Transaction, error) {
	minerProgramHash, err := common.Uint168FromAddress(addr)
	if err != nil {
		return nil, err
	}

	pd := &PayloadCoinBase{
		CoinbaseData: []byte(config.Parameters.PowConfiguration.MinerInfo),
	}

	txn := NewCoinBaseTransaction(pd, DefaultLedger.Blockchain.GetBestHeight()+1)
	txn.Inputs = []*Input{
		{
			Previous: OutPoint{
				TxID:  common.EmptyHash,
				Index: math.MaxUint16,
			},
			Sequence: math.MaxUint32,
		},
	}
	txn.Outputs = []*Output{
		{
			AssetID:     DefaultLedger.Blockchain.AssetID,
			Value:       0,
			ProgramHash: FoundationAddress,
		},
		{
			AssetID:     DefaultLedger.Blockchain.AssetID,
			Value:       0,
			ProgramHash: *minerProgramHash,
		},
	}

	nonce := make([]byte, 8)
	binary.BigEndian.PutUint64(nonce, rand.Uint64())
	txAttr := NewAttribute(Nonce, nonce)
	txn.Attributes = append(txn.Attributes, &txAttr)

	return txn, nil
}

type byFeeDesc []*Transaction

func (s byFeeDesc) Len() int           { return len(s) }
func (s byFeeDesc) Swap(i, j int)      { s[i], s[j] = s[j], s[i] }
func (s byFeeDesc) Less(i, j int) bool { return s[i].FeePerKB > s[j].FeePerKB }

func (pow *PowService) GenerateBlock(addr string) (*Block, error) {
	nextBlockHeight := DefaultLedger.Blockchain.GetBestHeight() + 1
	coinBaseTx, err := pow.CreateCoinbaseTx(nextBlockHeight, addr)
	if err != nil {
		return nil, err
	}

	header := Header{
		Version:    0,
		Previous:   *DefaultLedger.Blockchain.BestChain.Hash,
		MerkleRoot: common.EmptyHash,
		Timestamp:  uint32(DefaultLedger.Blockchain.MedianAdjustedTime().Unix()),
		Bits:       config.Parameters.ChainParam.PowLimitBits,
		Height:     nextBlockHeight,
		Nonce:      0,
	}

	msgBlock := &Block{
		Header:       header,
		Transactions: []*Transaction{},
	}

	msgBlock.Transactions = append(msgBlock.Transactions, coinBaseTx)
	totalTxsSize := coinBaseTx.GetSize()
	txCount := 1
	totalTxFee := common.Fixed64(0)
	var txsByFeeDesc byFeeDesc
	txsInPool := node.LocalNode.GetTransactionPool(false)
	txsByFeeDesc = make([]*Transaction, 0, len(txsInPool))
	for _, v := range txsInPool {
		txsByFeeDesc = append(txsByFeeDesc, v)
	}
	sort.Sort(txsByFeeDesc)

	for _, tx := range txsByFeeDesc {
		totalTxsSize = totalTxsSize + tx.GetSize()
		if totalTxsSize > config.Parameters.MaxBlockSize {
			break
		}
		if txCount >= config.Parameters.MaxTxsInBlock {
			break
		}

		if !IsFinalizedTransaction(tx, nextBlockHeight) {
			continue
		}
		if errCode := CheckTransactionContext(tx); errCode != Success {
			log.Warn("check transaction context failed, wrong transaction:", tx.Hash().String())
			continue
		}
		fee := GetTxFee(tx, DefaultLedger.Blockchain.AssetID)
		if fee != tx.Fee {
			continue
		}
		msgBlock.Transactions = append(msgBlock.Transactions, tx)
		totalTxFee += fee
		txCount++
	}

	blockReward := GetBlockRewardAmount(nextBlockHeight)
	totalReward := totalTxFee + blockReward
	rewardFoundation := common.Fixed64(float64(totalReward) * 0.3)
	msgBlock.Transactions[0].Outputs[0].Value = rewardFoundation
	msgBlock.Transactions[0].Outputs[1].Value = common.Fixed64(totalReward) - rewardFoundation

	txHash := make([]common.Uint256, 0, len(msgBlock.Transactions))
	for _, tx := range msgBlock.Transactions {
		txHash = append(txHash, tx.Hash())
	}
	txRoot, _ := crypto.ComputeRoot(txHash)
	msgBlock.Header.MerkleRoot = txRoot

	msgBlock.Header.Bits, err = CalcNextRequiredDifficulty(DefaultLedger.Blockchain.BestChain, time.Now())
	log.Info("difficulty: ", msgBlock.Header.Bits)

	return msgBlock, err
}

func (pow *PowService) DiscreteMining(n uint32) ([]*common.Uint256, error) {
	pow.Mutex.Lock()

	if pow.Started || pow.discreteMining {
		pow.Mutex.Unlock()
		return nil, errors.New("Server is already CPU mining.")
	}

	pow.Started = true
	pow.discreteMining = true
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
			if msgBlock.Header.Height == DefaultLedger.Blockchain.GetBestHeight()+1 {
				inMainChain, isOrphan, err := DefaultLedger.Blockchain.AddBlock(msgBlock)
				if err != nil {
					log.Trace(err)
					continue
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
					pow.Started = false
					pow.discreteMining = false
					pow.Mutex.Unlock()
					return blockHashes, nil
				}
			}
		}
	}
}

func (pow *PowService) SolveBlock(MsgBlock *Block, ticker *time.Ticker) bool {
	// fake a btc blockheader and coinbase
	auxPow := auxpow.GenerateAuxPow(MsgBlock.Hash())
	header := MsgBlock.Header
	targetDifficulty := CompactToBig(header.Bits)

	for i := uint32(0); i <= maxNonce; i++ {
		select {
		case <-ticker.C:
			// if !MsgBlock.Header.Previous.IsEqual(*DefaultLedger.Blockchain.BestChain.Hash) {
			// 	return false
			// }
			return false

		default:
			// Non-blocking select to fall through
		}

		auxPow.ParBlockHeader.Nonce = i
		hash := auxPow.ParBlockHeader.Hash() // solve parBlockHeader hash
		if HashToBig(&hash).Cmp(targetDifficulty) <= 0 {
			MsgBlock.Header.AuxPow = *auxPow
			return true
		}
	}

	return false
}

func (pow *PowService) BroadcastBlock(MsgBlock *Block) error {
	return node.LocalNode.Relay(nil, MsgBlock)
}

func (pow *PowService) Start() {
	pow.Mutex.Lock()
	defer pow.Mutex.Unlock()
	if pow.Started || pow.discreteMining {
		log.Trace("cpuMining is already Started")
	}

	pow.quit = make(chan struct{})
	pow.wg.Add(1)
	pow.Started = true

	go pow.cpuMining()
}

func (pow *PowService) Halt() {
	log.Info("POW Stop")
	pow.Mutex.Lock()
	defer pow.Mutex.Unlock()

	if !pow.Started || pow.discreteMining {
		return
	}

	close(pow.quit)
	pow.wg.Wait()
	pow.Started = false
}

func (pow *PowService) RollbackTransaction(v interface{}) {
	if block, ok := v.(*Block); ok {
		for _, tx := range block.Transactions[1:] {
			err := node.LocalNode.MaybeAcceptTransaction(tx)
			if err == nil {
				node.LocalNode.RemoveTransaction(tx)
			} else {
				log.Error(err)
			}
		}
	}
}

func (pow *PowService) BlockPersistCompleted(v interface{}) {
	log.Debug()
	if block, ok := v.(*Block); ok {
		log.Infof("persist block: %x", block.Hash())
		err := node.LocalNode.CleanSubmittedTransactions(block)
		if err != nil {
			log.Warn(err)
		}
		node.LocalNode.SetHeight(uint64(DefaultLedger.Blockchain.GetBestHeight()))
	}
}

func NewPowService() *PowService {
	pow := &PowService{
		PayToAddr:      config.Parameters.PowConfiguration.PayToAddr,
		Started:        false,
		discreteMining: false,
		MsgBlock:       msgBlock{BlockData: make(map[string]*Block)},
	}

	pow.blockPersistCompletedSubscriber = DefaultLedger.Blockchain.BCEvents.Subscribe(events.EventBlockPersistCompleted, pow.BlockPersistCompleted)
	pow.RollbackTransactionSubscriber = DefaultLedger.Blockchain.BCEvents.Subscribe(events.EventRollbackTransaction, pow.RollbackTransaction)

	log.Trace("pow Service Init succeed")
	return pow
}

func (pow *PowService) cpuMining() {
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
		log.Trace("<================Packing Block==============>")
		//time.Sleep(15 * time.Second)

		msgBlock, err := pow.GenerateBlock(pow.PayToAddr)
		if err != nil {
			log.Trace("generage block err", err)
			continue
		}

		//begin to mine the block with POW
		if pow.SolveBlock(msgBlock, ticker) {
			log.Trace("<================Solved Block==============>")
			//send the valid block to p2p networkd
			if msgBlock.Header.Height == DefaultLedger.Blockchain.GetBestHeight()+1 {
				inMainChain, isOrphan, err := DefaultLedger.Blockchain.AddBlock(msgBlock)
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
