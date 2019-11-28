package chain

import (
	"time"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/mempool"
	"github.com/elastos/Elastos.ELA/pow"
	"github.com/elastos/Elastos.ELA/utils/signal"
)

type DataGen struct {
	height         uint32
	txRepo         *TxRepository
	chain          *blockchain.BlockChain
	chainParams    *config.Params
	params         *GenerationParams
	pow            *pow.Service
	txPool         *mempool.TxPool
	foundationAddr string
}

func (g *DataGen) Generate() (err error) {
	var process func(height uint32) error
	switch g.params.Mod {
	case Normal:
		process = g.normalProcess
	case Minimal:
		process = g.minimalProcess
	default:
		process = g.fastProcess
	}

	for i := uint32(1); i <= g.height; i++ {
		if err = process(i); err != nil {
			return
		}
	}
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
	// todo store the block without check
	return
}

func (g *DataGen) generateBlock(
	txs []*types.Transaction) (block *types.Block, err error) {
	for _, v := range txs {
		if err = g.txPool.AppendToTxPool(v); err != nil {
			return
		}
	}

	if block, err = g.pow.GenerateBlock(g.foundationAddr); err != nil {
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

func NewDataGen(height uint32, dataDir string,
	params *GenerationParams) (*DataGen, error) {
	repo, err := NewTxRepository(params)
	if err != nil {
		return nil, err
	}

	var interrupt = signal.NewInterrupt()
	chainParams := generateChainParams(repo.GetFoundationAccount())
	chain, err := newBlockChain(dataDir, chainParams, interrupt.C)
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
		height:         height,
		params:         params,
		chainParams:    chainParams,
		chain:          chain,
		txPool:         txPool,
		foundationAddr: foundationAddr,
		pow: pow.NewService(
			&pow.Config{
				PayToAddr:   foundationAddr,
				MinerInfo:   "foundation",
				Chain:       chain,
				ChainParams: chainParams,
				TxMemPool:   txPool,
			})}, nil
}
