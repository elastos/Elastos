package chain

import (
	"bytes"
	"errors"
	"fmt"
	"io/ioutil"
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
	txRepoFile = "dategen.repo"
)

type DataGen struct {
	txRepo         *TxRepository
	chain          *blockchain.BlockChain
	chainParams    *config.Params
	pow            *pow.Service
	txPool         *mempool.TxPool
	prevBlockHash  common.Uint256
	foundationAddr string
	dataDir        string
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

	for i := g.chain.GetHeight(); i <= height; i++ {
		if err = process(i); err != nil {
			return
		}
	}
	log.Info("generating complete!")
	return
}

func (g *DataGen) fastProcess(height uint32) (err error) {
	var txs []*types.Transaction
	if txs, err = g.txRepo.GenerateTxs(height); err != nil {
		return
	}

	var block *types.Block
	if block, err = g.generateBlock(txs); err != nil {
		return
	}

	if err = g.storeData(block); err != nil {
		return
	}
	g.txPool.CleanSubmittedTransactions(block)
	return
}

func (g *DataGen) normalProcess(height uint32) (err error) {
	var txs []*types.Transaction
	if txs, err = g.txRepo.GenerateTxs(height); err != nil {
		return
	}

	var block *types.Block
	if block, err = g.generateBlock(txs); err != nil {
		return
	}

	if _, _, err = g.chain.ProcessBlock(block, nil); err != nil {
		return
	}

	g.txPool.CleanSubmittedTransactions(block)
	return
}

func (g *DataGen) minimalProcess(height uint32) (err error) {
	var txs []*types.Transaction
	if txs, err = g.txRepo.GenerateTxs(height); err != nil {
		return
	}

	var block *types.Block
	if block, err = quickGenerateBlock(g.pow, &g.prevBlockHash, txs,
		g.foundationAddr, g.chainParams, height); err != nil {
		return
	}
	g.prevBlockHash = block.Hash()

	if err = g.storeData(block); err != nil {
		return
	}
	return
}

func (g *DataGen) generateBlock(
	txs []*types.Transaction) (block *types.Block, err error) {
	for _, v := range txs {
		if err = g.txPool.AppendToTxPool(v); err != nil {
			return
		}
	}

	if block, err = g.pow.GenerateBlock(g.foundationAddr,
		int(g.txRepo.Params().InputsPerBlock)); err != nil {
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
	filename := path.Join(g.dataDir, txRepoFile)
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
	return FromTxRepository(path.Dir(dataPath), interrupt.C, repo)
}

func FromTxRepository(dataDir string, interrupt <-chan struct{},
	repo *TxRepository) (*DataGen, error) {
	chainParams := generateChainParams(repo.GetFoundationAccount())
	chain, err := newBlockChain(dataDir, chainParams, interrupt)
	if err != nil {
		return nil, err
	}

	fundTx := chainParams.GenesisBlock.Transactions[0]
	repo.SetFoundationUTXO(&types.UTXO{
		TxID:  fundTx.Hash(),
		Index: 0,
		Value: fundTx.Outputs[0].Value,
	})

	foundationAddr, err := repo.GetFoundationAccount().ProgramHash.ToAddress()
	if err != nil {
		return nil, err
	}

	txPool := mempool.NewTxPool(chainParams)
	return &DataGen{
		txRepo:         repo,
		chainParams:    chainParams,
		chain:          chain,
		txPool:         txPool,
		foundationAddr: foundationAddr,
		prevBlockHash:  chainParams.GenesisBlock.Hash(),
		dataDir:        dataDir,
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

	return FromTxRepository(dataDir, interrupt, repo)
}
