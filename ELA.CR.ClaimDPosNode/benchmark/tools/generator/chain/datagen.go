// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package chain

import (
	"bytes"
	"errors"
	"fmt"
	"io/ioutil"
	"math"
	"os"
	"path"
	"time"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/mempool"
	"github.com/elastos/Elastos.ELA/pow"
	"github.com/elastos/Elastos.ELA/utils"
	"github.com/elastos/Elastos.ELA/utils/signal"
)

const (
	TxRepoFile    = "dategen.repo"
	maxTxPerBlock = math.MaxUint32
)

type TimeCounter struct {
	StartTimer func()
	StopTimer  func()
}

type DataGen struct {
	txRepo         *TxRepository
	chain          *blockchain.BlockChain
	chainParams    *config.Params
	pow            *pow.Service
	txPool         *mempool.TxPool
	prevBlockHash  common.Uint256
	foundationAddr string
	dataDir        string

	processDataCounter *TimeCounter
	addToTxPoolCount   *TimeCounter
	pressure           bool
	pressureTxSize     int
}

func (g *DataGen) GetChain() *blockchain.BlockChain {
	return g.chain
}

func (g *DataGen) SetPressure(enable bool, size int) {
	g.pressure = enable
	g.pressureTxSize = size
}

func (g *DataGen) SetGenerateMode(mode GenerationMod) {
	g.txRepo.Params().Mode = mode
}

func (g *DataGen) SetPrevBlockHash(hash common.Uint256) {
	g.prevBlockHash = hash
}

func (g *DataGen) EnableProcessDataTimer(counter *TimeCounter) {
	g.processDataCounter = counter
}

func (g *DataGen) EnableAddToTxPoolTimer(counter *TimeCounter) {
	g.addToTxPoolCount = counter
}

func (g *DataGen) Generate(height uint32) (err error) {
	var process func(height uint32) error
	switch g.txRepo.Params().Mode {
	case Normal:
		process = g.normalProcess
	case Minimal:
		process = g.minimalProcess
	default:
		process = g.fastProcess
	}

	for i := g.chain.GetHeight(); i < height; i++ {
		if err = process(i); err != nil {
			return
		}
	}
	log.Info("generating complete!")
	return
}

func (g *DataGen) fastProcess(height uint32) (err error) {
	var txs []*types.Transaction
	if txs, err = g.generateTxs(height); err != nil {
		return
	}

	var block *types.Block
	if block, err = g.generateBlock(txs); err != nil {
		return
	}

	g.countProcess(g.processDataCounter, func() {
		err = g.storeData(block)
	})
	if err != nil {
		return
	}

	g.txPool.CleanSubmittedTransactions(block)
	return
}

func (g *DataGen) normalProcess(height uint32) (err error) {
	var txs []*types.Transaction
	if txs, err = g.generateTxs(height); err != nil {
		return
	}

	var block *types.Block
	if block, err = g.generateBlock(txs); err != nil {
		return
	}

	g.countProcess(g.processDataCounter, func() {
		_, _, err = g.chain.ProcessBlock(block, nil)
	})
	if err != nil {
		return
	}

	g.txPool.CleanSubmittedTransactions(block)
	return
}

func (g *DataGen) minimalProcess(height uint32) (err error) {
	var txs []*types.Transaction
	if txs, err = g.generateTxs(height); err != nil {
		return
	}

	var block *types.Block
	if block, err = quickGenerateBlock(g.pow, &g.prevBlockHash, txs,
		g.foundationAddr, g.chainParams, height); err != nil {
		return
	}
	g.prevBlockHash = block.Hash()

	g.countProcess(g.processDataCounter, func() {
		err = g.storeData(block)
	})
	if err != nil {
		return
	}
	return
}

func (g *DataGen) countProcess(counter *TimeCounter, action func()) {
	if counter != nil {
		counter.StartTimer()
		action()
		counter.StopTimer()
	} else {
		action()
	}
}

func (g *DataGen) generateTxs(
	height uint32) (txs []*types.Transaction, err error) {
	if g.pressure {
		return g.txRepo.GeneratePressureTxs(height, g.pressureTxSize)
	} else {
		return g.txRepo.GenerateTxs(height)
	}
}

func (g *DataGen) generateBlock(
	txs []*types.Transaction) (block *types.Block, err error) {
	g.countProcess(g.addToTxPoolCount, func() {
		for _, v := range txs {
			if err = g.txPool.AppendToTxPool(v); err != nil {
				return
			}
		}
	})

	if block, err = g.pow.GenerateBlock(g.foundationAddr,
		maxTxPerBlock); err != nil {
		return
	}
	g.pow.SolveBlock(block, nil)
	return
}

func (g *DataGen) storeData(block *types.Block) error {
	blockHash := block.Hash()
	newNode := blockchain.NewBlockNode(&block.Header, &blockHash)
	if err := g.chain.GetDB().GetFFLDB().SaveBlock(block, newNode,
		nil, time.Unix(int64(block.Timestamp), 0)); err != nil {
		return err
	}

	g.chain.Nodes = append(g.chain.Nodes, newNode)
	g.chain.BestChain = newNode
	return nil
}

func (g *DataGen) Save() (err error) {
	filename := path.Join(g.dataDir, TxRepoFile)
	var file *os.File
	file, err = os.OpenFile(filename,
		os.O_CREATE|os.O_WRONLY|os.O_TRUNC, 0600)
	if err != nil {
		return
	}
	defer file.Close()

	buf := new(bytes.Buffer)
	if err = g.txRepo.Serialize(buf); err != nil {
		return
	}

	if _, err = file.Write(buf.Bytes()); err != nil {
		return
	}

	return
}

func (g *DataGen) Exit() {
	g.chain.GetDB().Close()
}

func LoadDataGen(dataPath string) (*DataGen, error) {
	if !utils.FileExisted(dataPath) {
		return nil, errors.New(fmt.Sprintf("can't find file: %s", dataPath))
	}
	file, err := os.OpenFile(dataPath, os.O_RDONLY, 0400)
	if err != nil {
		return nil, err
	}
	defer file.Close()

	buf := new(bytes.Buffer)
	fileData, err := ioutil.ReadAll(file)
	buf.Write(fileData)

	repo := &TxRepository{}
	if err = repo.Deserialize(buf); err != nil {
		return nil, err
	}

	var interrupt = signal.NewInterrupt()
	return FromTxRepository(path.Dir(dataPath), interrupt.C,
		repo, false)
}

func FromTxRepository(dataDir string, interrupt <-chan struct{},
	repo *TxRepository, initFoundationUTXO bool) (*DataGen, error) {
	chainParams := generateChainParams(repo.GetFoundationAccount())
	chain, err := newBlockChain(dataDir, chainParams, interrupt)
	if err != nil {
		return nil, err
	}

	if initFoundationUTXO {
		fundTx := chainParams.GenesisBlock.Transactions[0]
		repo.SetFoundationUTXO(&types.UTXO{
			TxID:  fundTx.Hash(),
			Index: 0,
			Value: fundTx.Outputs[0].Value,
		})
	}

	foundationAddr, err := repo.GetFoundationAccount().ProgramHash.ToAddress()
	if err != nil {
		return nil, err
	}

	txPool := mempool.NewTxPool(chainParams)
	return &DataGen{
		txRepo:             repo,
		chainParams:        chainParams,
		chain:              chain,
		txPool:             txPool,
		foundationAddr:     foundationAddr,
		prevBlockHash:      chainParams.GenesisBlock.Hash(),
		dataDir:            dataDir,
		pressure:           false,
		pressureTxSize:     8000000,
		processDataCounter: nil,
		pow: pow.NewService(
			&pow.Config{
				PayToAddr:   foundationAddr,
				MinerInfo:   "foundation",
				Chain:       chain,
				ChainParams: chainParams,
				TxMemPool:   txPool,
			})}, nil
}

func NewDataGen(dataDir string, interrupt <-chan struct{},
	params *GenerationParams) (*DataGen, error) {
	repo, err := NewTxRepository(params)
	if err != nil {
		return nil, err
	}

	return FromTxRepository(dataDir, interrupt, repo, true)
}
