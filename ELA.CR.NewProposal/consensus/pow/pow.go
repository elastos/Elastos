package pow

import (
	"Elastos.ELA/net/protocol"
	"encoding/binary"
	"errors"
	"math"
	"math/rand"
	"sort"
	"sync"
	"time"

	. "Elastos.ELA/common"
	"Elastos.ELA/common/config"
	"Elastos.ELA/common/log"
	"Elastos.ELA/core/auxpow"
	"Elastos.ELA/core/ledger"
	tx "Elastos.ELA/core/transaction"
	"Elastos.ELA/core/transaction/payload"
	"Elastos.ELA/crypto"
	"Elastos.ELA/events"
	//	"ELA/net"
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

	OrginAmountOfEla = 3300 * 10000 * 100000000
	SubsidyInterval  = 365 * 24 * 60 * 60 / TargetTimePerBlock
	RetargetPersent  = 25
)

type msgBlock struct {
	BlockData map[string]*ledger.Block
	Mutex     sync.Mutex
}

type PowService struct {
	PayToAddr     string
	MsgBlock      msgBlock
	ZMQPublish    chan bool
	Mutex         sync.Mutex
	logDictionary string
	started       bool
	manualMining  bool
	localNet      protocol.Noder

	blockPersistCompletedSubscriber events.Subscriber
	RollbackTransactionSubscriber   events.Subscriber

	wg   sync.WaitGroup
	quit chan struct{}
}

func (pow *PowService) GetTransactionCount() int {
	transactionsPool := pow.localNet.GetTxnPool(true)
	return len(transactionsPool)
}

func (pow *PowService) CollectTransactions(MsgBlock *ledger.Block) int {
	txs := 0
	transactionsPool := pow.localNet.GetTxnPool(true)

	for _, tx := range transactionsPool {
		log.Trace(tx)
		MsgBlock.Transactions = append(MsgBlock.Transactions, tx)
		txs++
	}
	return txs
}

func (pow *PowService) CreateCoinbaseTrx(nextBlockHeight uint32, addr string) (*tx.Transaction, error) {
	minerProgramHash, err := ToScriptHash(addr)
	if err != nil {
		return nil, err
	}
	foundationProgramHash, err := ToScriptHash(ledger.FoundationAddress)
	if err != nil {
		return nil, err
	}

	pd := &payload.CoinBase{
		CoinbaseData: []byte(config.Parameters.PowConfiguration.MinerInfo),
	}

	txn, err := tx.NewCoinBaseTransaction(pd, ledger.DefaultLedger.Blockchain.GetBestHeight()+1)
	if err != nil {
		return nil, err
	}
	txn.UTXOInputs = []*tx.UTXOTxInput{
		{
			ReferTxID:          Uint256{},
			ReferTxOutputIndex: math.MaxUint16,
			Sequence:           math.MaxUint32,
		},
	}
	txn.Outputs = []*tx.TxOutput{
		{
			AssetID:     ledger.DefaultLedger.Blockchain.AssetID,
			Value:       0,
			ProgramHash: foundationProgramHash,
		},
		{
			AssetID:     ledger.DefaultLedger.Blockchain.AssetID,
			Value:       0,
			ProgramHash: minerProgramHash,
		},
	}

	nonce := make([]byte, 8)
	binary.BigEndian.PutUint64(nonce, rand.Uint64())
	txAttr := tx.NewTxAttribute(tx.Nonce, nonce)
	txn.Attributes = append(txn.Attributes, &txAttr)
	log.Trace("txAttr", txAttr)

	return txn, nil
}

func calcBlockSubsidy(currentHeight uint32) int64 {
	ToTalAmountOfEla := int64(OrginAmountOfEla)
	for i := uint32(0); i < (currentHeight / uint32(SubsidyInterval)); i++ {
		incr := float64(ToTalAmountOfEla) / float64(RetargetPersent)
		subsidyPerBlock := int64(float64(incr) / float64(SubsidyInterval))
		ToTalAmountOfEla += subsidyPerBlock * int64(SubsidyInterval)
	}
	incr := float64(ToTalAmountOfEla) / float64(RetargetPersent)
	subsidyPerBlock := int64(float64(incr) / float64(SubsidyInterval))
	log.Trace("subsidyPerBlock: ", subsidyPerBlock)

	return subsidyPerBlock
}

type txSorter []*tx.Transaction

func (s txSorter) Len() int {
	return len(s)
}

func (s txSorter) Swap(i, j int) {
	s[i], s[j] = s[j], s[i]
}

func (s txSorter) Less(i, j int) bool {
	return s[i].FeePerKB < s[j].FeePerKB
}

func (pow *PowService) GenerateBlock(addr string) (*ledger.Block, error) {
	nextBlockHeight := ledger.DefaultLedger.Blockchain.GetBestHeight() + 1
	coinBaseTx, err := pow.CreateCoinbaseTrx(nextBlockHeight, addr)
	if err != nil {
		return nil, err
	}

	blockData := &ledger.Blockdata{
		Version:          0,
		PrevBlockHash:    *ledger.DefaultLedger.Blockchain.BestChain.Hash,
		TransactionsRoot: Uint256{},
		Timestamp:        uint32(ledger.DefaultLedger.Blockchain.MedianAdjustedTime().Unix()),
		Bits:             config.Parameters.ChainParam.PowLimitBits,
		Height:           nextBlockHeight,
		Nonce:            0,
		AuxPow:           auxpow.AuxPow{},
	}

	msgBlock := &ledger.Block{
		Blockdata:    blockData,
		Transactions: []*tx.Transaction{},
	}

	msgBlock.Transactions = append(msgBlock.Transactions, coinBaseTx)
	calcTxsSize := coinBaseTx.GetSize()
	calcTxsAmount := 1
	totalFee := int64(0)
	var txPool txSorter
	txPool = make([]*tx.Transaction, 0)
	transactionsPool := pow.localNet.GetTxnPool(false)
	for _, v := range transactionsPool {
		txPool = append(txPool, v)
	}
	sort.Sort(sort.Reverse(txPool))

	for _, tx := range txPool {
		if (tx.GetSize() + calcTxsSize) > ledger.MaxBlockSize {
			break
		}
		if calcTxsAmount >= config.Parameters.MaxTxInBlock {
			break
		}

		if !ledger.IsFinalizedTransaction(tx, nextBlockHeight) {
			continue
		}
		fee := tx.GetFee(ledger.DefaultLedger.Blockchain.AssetID)
		if fee != int64(tx.Fee) {
			continue
		}
		msgBlock.Transactions = append(msgBlock.Transactions, tx)
		calcTxsSize = calcTxsSize + tx.GetSize()
		calcTxsAmount++
		totalFee += fee
	}

	subsidy := calcBlockSubsidy(nextBlockHeight)
	reward := totalFee + subsidy
	reward_foundation := Fixed64(float64(reward) * 0.3)
	msgBlock.Transactions[0].Outputs[0].Value = reward_foundation
	msgBlock.Transactions[0].Outputs[1].Value = Fixed64(reward) - reward_foundation

	txHash := []Uint256{}
	for _, tx := range msgBlock.Transactions {
		txHash = append(txHash, tx.Hash())
	}
	txRoot, _ := crypto.ComputeRoot(txHash)
	msgBlock.Blockdata.TransactionsRoot = txRoot

	msgBlock.Blockdata.Bits, err = ledger.CalcNextRequiredDifficulty(ledger.DefaultLedger.Blockchain.BestChain, time.Now())
	log.Info("difficulty: ", msgBlock.Blockdata.Bits)

	return msgBlock, err
}

func (pow *PowService) ManualMining(n uint32) ([]*Uint256, error) {
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
	blockHashes := make([]*Uint256, n)
	ticker := time.NewTicker(time.Second * hashUpdateSecs)
	defer ticker.Stop()

	for {
		log.Trace("<================Manual Mining==============>\n")

		msgBlock, err := pow.GenerateBlock(pow.PayToAddr)
		if err != nil {
			log.Trace("generage block err", err)
			continue
		}

		if pow.SolveBlock(msgBlock, ticker) {
			if msgBlock.Blockdata.Height == ledger.DefaultLedger.Blockchain.GetBestHeight()+1 {
				inMainChain, isOrphan, err := ledger.DefaultLedger.Blockchain.AddBlock(msgBlock)
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
					pow.started = false
					pow.manualMining = false
					pow.Mutex.Unlock()
					return blockHashes, nil
				}
			}
		}
	}
}

func (pow *PowService) SolveBlock(MsgBlock *ledger.Block, ticker *time.Ticker) bool {
	// fake a btc blockheader and coinbase
	auxPow := generateAuxPow(MsgBlock.Hash())
	header := MsgBlock.Blockdata
	targetDifficulty := ledger.CompactToBig(header.Bits)

	for i := uint32(0); i <= maxNonce; i++ {
		select {
		case <-ticker.C:
			if MsgBlock.Blockdata.PrevBlockHash.CompareTo(*ledger.DefaultLedger.Blockchain.BestChain.Hash) != 0 {
				return false
			}
			//UpdateBlockTime(msgBlock, m.server.blockManager)

		default:
			// Non-blocking select to fall through
		}

		auxPow.ParBlockHeader.Nonce = i
		hash := auxPow.ParBlockHeader.Hash() // solve parBlockHeader hash
		if ledger.HashToBig(&hash).Cmp(targetDifficulty) <= 0 {
			MsgBlock.Blockdata.AuxPow = *auxPow
			return true
		}
	}

	return false
}

func (pow *PowService) BroadcastBlock(MsgBlock *ledger.Block) error {
	pow.localNet.Xmit(MsgBlock)
	return nil
}

func (pow *PowService) Start() {
	pow.Mutex.Lock()
	defer pow.Mutex.Unlock()
	if pow.started || pow.manualMining {
		log.Trace("cpuMining is already started")
	}

	pow.quit = make(chan struct{})
	pow.wg.Add(1)
	pow.started = true

	go pow.cpuMining()
}

func (pow *PowService) Halt() {
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

func (pow *PowService) RollbackTransaction(v interface{}) {
	if block, ok := v.(*ledger.Block); ok {
		for _, tx := range block.Transactions[1:] {
			err := pow.localNet.MaybeAcceptTransaction(tx)
			if err == nil {
				pow.localNet.RemoveTransaction(tx)
			} else {
				log.Error(err)
			}
		}
	}
}

func (pow *PowService) BlockPersistCompleted(v interface{}) {
	log.Debug()
	if block, ok := v.(*ledger.Block); ok {
		log.Infof("persist block: %x", block.Hash())
		err := pow.localNet.CleanSubmittedTransactions(block)
		if err != nil {
			log.Warn(err)
		}
		pow.localNet.SetHeight(uint64(ledger.DefaultLedger.Blockchain.GetBestHeight()))
	}
}

func NewPowService(logDictionary string, localNet protocol.Noder) *PowService {
	pow := &PowService{
		PayToAddr:     config.Parameters.PowConfiguration.PayToAddr,
		started:       false,
		manualMining:  false,
		MsgBlock:      msgBlock{BlockData: make(map[string]*ledger.Block)},
		ZMQPublish:    make(chan bool, 1),
		localNet:      localNet,
		logDictionary: logDictionary,
	}

	pow.blockPersistCompletedSubscriber = ledger.DefaultLedger.Blockchain.BCEvents.Subscribe(events.EventBlockPersistCompleted, pow.BlockPersistCompleted)
	pow.RollbackTransactionSubscriber = ledger.DefaultLedger.Blockchain.BCEvents.Subscribe(events.EventRollbackTransaction, pow.RollbackTransaction)

	go pow.ZMQServer()
	log.Trace("pow Service Init succeed and ZMQServer start succeed")
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
		log.Trace("<================POW Mining==============>\n")
		//time.Sleep(15 * time.Second)

		msgBlock, err := pow.GenerateBlock(pow.PayToAddr)
		if err != nil {
			log.Trace("generage block err", err)
			continue
		}

		// push notifyed message into ZMQ
		pow.ZMQPublish <- true

		//begin to mine the block with POW
		if pow.SolveBlock(msgBlock, ticker) {
			//send the valid block to p2p networkd
			if msgBlock.Blockdata.Height == ledger.DefaultLedger.Blockchain.GetBestHeight()+1 {
				inMainChain, isOrphan, err := ledger.DefaultLedger.Blockchain.AddBlock(msgBlock)
				if err != nil {
					log.Trace(err)
					continue
				}
				//TODO if co-mining condition
				if isOrphan || !inMainChain {
					continue
				}
				//pow.ZMQClientSend(*msgBlock)
				pow.BroadcastBlock(msgBlock)
			}
		}

	}

	pow.wg.Done()
}
