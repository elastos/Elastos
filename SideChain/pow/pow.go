package pow

import (
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
	"math"
	"math/rand"
	"sort"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA.SideChain/auxpow"
	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/mempool"
	"github.com/elastos/Elastos.ELA.SideChain/types"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/p2p/server"
)

const (
	maxNonce                 = ^uint32(0) // 2^32 - 1
	maxExtraNonce            = ^uint64(0) // 2^64 - 1
	hpsUpdateSecs            = 10
	hashUpdateSecs           = 15
	auxBlockGenerateInterval = 5
)

type Config struct {
	ChainParams *config.Params
	MinerAddr   string
	MinerInfo   string
	Server      server.IServer
	Chain       *blockchain.BlockChain
	TxMemPool   *mempool.TxPool
	TxFeeHelper *mempool.FeeHelper
	Validator   *mempool.Validator

	CreateCoinBaseTx          func(cfg *Config, nextBlockHeight uint32, addr string) (*types.Transaction, error)
	GenerateBlock             func(cfg *Config) (*types.Block, error)
	GenerateBlockTransactions func(cfg *Config, msgBlock *types.Block, coinBaseTx *types.Transaction)
}

type Service struct {
	mutex        sync.Mutex
	cfg          Config
	started      bool
	manualMining bool

	// This params are protected by blockMtx
	blockMtx       sync.Mutex
	msgBlocks      map[string]*types.Block
	preChainHeight uint32
	preTime        int64
	preTxCount     int

	wg   sync.WaitGroup
	quit chan struct{}
}

func NewService(cfg *Config) *Service {
	pow := Service{
		cfg:          *cfg,
		started:      false,
		manualMining: false,
		msgBlocks:    make(map[string]*types.Block),
	}

	return &pow
}

func (s *Service) SetMinerAddr(minerAddr string) {
	s.cfg.MinerAddr = minerAddr
}

func (s *Service) GetTransactionCount() int {
	transactionsPool := s.cfg.TxMemPool.GetTxsInPool()
	return len(transactionsPool)
}

func (s *Service) CollectTransactions(msgBlock *types.Block) int {
	txs := 0
	transactionsPool := s.cfg.TxMemPool.GetTxsInPool()

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

func (s *Service) DiscreteMining(n uint32) ([]*common.Uint256, error) {
	s.mutex.Lock()

	if s.started || s.manualMining {
		s.mutex.Unlock()
		return nil, errors.New("Server is already CPU mining.")
	}

	s.started = true
	s.manualMining = true
	s.mutex.Unlock()

	log.Infof("Pow generating %d blocks", n)
	i := uint32(0)
	blockHashes := make([]*common.Uint256, n)
	ticker := time.NewTicker(time.Second * hashUpdateSecs)
	defer ticker.Stop()

	for {
		log.Info("<================Discrete Mining==============>\n")

		msgBlock, err := s.cfg.GenerateBlock(&s.cfg)
		if err != nil {
			log.Error("generate block err", err)
			continue
		}

		if s.SolveBlock(msgBlock, ticker) {
			if msgBlock.Header.Height == s.cfg.Chain.GetBestHeight()+1 {
				inMainChain, isOrphan, err := s.cfg.Chain.ProcessBlock(msgBlock)
				if err != nil {
					return nil, err
				}
				//TODO if co-mining condition
				if isOrphan || !inMainChain {
					continue
				}
				h := msgBlock.Hash()
				blockHashes[i] = &h
				i++
				if i == n {
					s.mutex.Lock()
					s.started = false
					s.manualMining = false
					s.mutex.Unlock()
					return blockHashes, nil
				}
			}
		}
	}
}

func (s *Service) GenerateAuxBlock(addr string) (*types.Block, string, bool) {
	if len(addr) == 0 {
		addr = s.cfg.MinerAddr
	} else {
		s.cfg.MinerAddr = addr
	}

	s.blockMtx.Lock()
	defer s.blockMtx.Unlock()

	msgBlock := &types.Block{}
	bestHeight := s.cfg.Chain.GetBestHeight()
	if s.cfg.Chain.GetBestHeight() == 0 || s.preChainHeight != bestHeight ||
		time.Now().Unix()-s.preTime > auxBlockGenerateInterval {
		if s.preChainHeight != bestHeight {
			s.preChainHeight = bestHeight
			s.preTime = time.Now().Unix()
			s.preTxCount = s.GetTransactionCount()
		}

		currentTxsCount := s.CollectTransactions(msgBlock)
		if 0 == currentTxsCount {
			// return nil, "currentTxs is nil", false
		}

		msgBlock, err := s.cfg.GenerateBlock(&s.cfg)
		if nil != err {
			return nil, "msgBlock generate err", false
		}

		curHash := msgBlock.Hash()
		curHashStr := common.BytesToHexString(curHash.Bytes())

		s.msgBlocks[curHashStr] = msgBlock
		s.preChainHeight = bestHeight
		s.preTime = time.Now().Unix()
		s.preTxCount = currentTxsCount // Don't Call GetTransactionCount()

		return msgBlock, curHashStr, true
	}
	return nil, "", false
}

func (s *Service) SubmitAuxBlock(blockHash string, sideAuxData []byte) error {
	s.blockMtx.Lock()
	defer s.blockMtx.Unlock()

	msgBlock, ok := s.msgBlocks[blockHash]
	if !ok {
		return fmt.Errorf("receive invalid block hash %s", blockHash)
	}

	err := msgBlock.Header.SideAuxPow.Deserialize(bytes.NewReader(sideAuxData))
	if err != nil {
		log.Warn(err)
		return fmt.Errorf("deserialize side aux pow failed")
	}

	inMainChain, isOrphan, err := s.cfg.Chain.ProcessBlock(msgBlock)
	if err != nil {
		return err
	}

	if isOrphan || !inMainChain {
		return fmt.Errorf("aux block can not be accepted")
	}

	s.msgBlocks = make(map[string]*types.Block)

	return nil
}

func (s *Service) SolveBlock(msgBlock *types.Block, ticker *time.Ticker) bool {
	genesisHash, err := s.cfg.Chain.GetBlockHash(0)
	if err != nil {
		return false
	}
	// fake a mainchain blockheader
	sideAuxPow := auxpow.GenerateSideAuxPow(msgBlock.Hash(), genesisHash)
	header := msgBlock.Header
	targetDifficulty := blockchain.CompactToBig(header.Bits)

	for i := uint32(0); i <= maxNonce; i++ {
		select {
		case <-ticker.C:
			if !msgBlock.Header.Previous.IsEqual(*s.cfg.Chain.BestChain.Hash) {
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

func (s *Service) Start() {
	s.mutex.Lock()
	defer s.mutex.Unlock()
	if s.started || s.manualMining {
		log.Warn("CpuMining is already started")
	}

	s.quit = make(chan struct{})
	s.wg.Add(1)
	s.started = true

	go s.CpuMining()
}

func (s *Service) Halt() {
	log.Info("POW Stop")
	s.mutex.Lock()
	defer s.mutex.Unlock()

	if !s.started || s.manualMining {
		return
	}

	close(s.quit)
	s.wg.Wait()
	s.started = false
}

func (s *Service) CpuMining() {
	ticker := time.NewTicker(time.Second * hashUpdateSecs)
	defer ticker.Stop()

out:
	for {
		select {
		case <-s.quit:
			break out
		default:
			// Non-blocking select to fall through
		}
		log.Info("<================POW Mining==============>\n")
		//time.Sleep(15 * time.Second)

		msgBlock, err := s.cfg.GenerateBlock(&s.cfg)
		if err != nil {
			log.Error("generate block err", err)
			continue
		}

		//begin to mine the block with POW
		if s.SolveBlock(msgBlock, ticker) {
			//send the valid block to p2p networkd
			if msgBlock.Header.Height == s.cfg.Chain.GetBestHeight()+1 {
				inMainChain, isOrphan, err := s.cfg.Chain.ProcessBlock(msgBlock)
				if err != nil {
					continue
				}
				//TODO if co-mining condition
				if isOrphan || !inMainChain {
					continue
				}
			}
		}
	}

	s.wg.Done()
}

func CreateCoinBaseTx(cfg *Config, nextBlockHeight uint32, addr string) (*types.Transaction, error) {
	minerProgramHash, err := common.Uint168FromAddress(addr)
	if err != nil {
		return nil, err
	}

	pd := &types.PayloadCoinBase{
		CoinbaseData: []byte(cfg.MinerInfo),
	}

	txn := NewCoinBaseTransaction(pd, cfg.Chain.GetBestHeight()+1)
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
			AssetID:     cfg.ChainParams.ElaAssetId,
			Value:       0,
			ProgramHash: cfg.ChainParams.Foundation,
		},
		{
			AssetID:     cfg.ChainParams.ElaAssetId,
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

func NewCoinBaseTransaction(coinBasePayload *types.PayloadCoinBase, currentHeight uint32) *types.Transaction {
	return &types.Transaction{
		TxType:         types.CoinBase,
		PayloadVersion: types.PayloadCoinBaseVersion,
		Payload:        coinBasePayload,
		Inputs: []*types.Input{
			{
				Previous: types.OutPoint{
					TxID:  common.EmptyHash,
					Index: 0x0000,
				},
				Sequence: 0x00000000,
			},
		},
		Attributes: []*types.Attribute{},
		LockTime:   currentHeight,
		Programs:   []*types.Program{},
	}
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
		Bits:       cfg.ChainParams.PowLimitBits,
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

	msgBlock.Header.Bits, err = cfg.Chain.CalcNextRequiredDifficulty(
		cfg.Chain.BestChain, time.Now())
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
		if totalTxsSize > types.MaxBlockSize {
			break
		}
		if txCount >= types.MaxTxPerBlock {
			break
		}

		if err := blockchain.CheckTransactionFinalize(tx, nextBlockHeight); err != nil {
			continue
		}

		if err := cfg.Validator.CheckTransactionContext(tx); err != nil {
			continue
		}

		fee, err := cfg.TxFeeHelper.GetTxFee(tx, cfg.ChainParams.ElaAssetId)
		if err != nil || fee != tx.Fee {
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
