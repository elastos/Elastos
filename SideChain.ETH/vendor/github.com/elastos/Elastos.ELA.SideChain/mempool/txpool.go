package mempool

import (
	"bytes"
	"errors"
	"fmt"
	"sync"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/events"
	"github.com/elastos/Elastos.ELA.SideChain/spv"
	"github.com/elastos/Elastos.ELA.SideChain/types"

	"github.com/elastos/Elastos.ELA/common"
)

type Config struct {
	ChainParams *config.Params
	ChainStore  *blockchain.ChainStore
	SpvService  *spv.Service
	Validator   *Validator
	FeeHelper   *FeeHelper
}

type TxPool struct {
	chainParams *config.Params
	validator   *Validator
	feeHelper   *FeeHelper
	sync.RWMutex
	txCount         uint64                                // count
	txnList         map[common.Uint256]*types.Transaction // transaction which have been verifyed will put into this map
	inputUTXOList   map[string]*types.Transaction         // transaction which pass the verify will add the UTXO to this map
	mainchainTxList map[common.Uint256]*types.Transaction // mainchain tx pool
}

func New(cfg *Config) *TxPool {
	p := TxPool{
		chainParams:     cfg.ChainParams,
		validator:       cfg.Validator,
		feeHelper:       cfg.FeeHelper,
		txCount:         0,
		inputUTXOList:   make(map[string]*types.Transaction),
		txnList:         make(map[common.Uint256]*types.Transaction),
		mainchainTxList: make(map[common.Uint256]*types.Transaction),
	}
	return &p
}

//append transaction to txnpool when check ok.
//1.check  2.check with ledger(db) 3.check with pool
func (p *TxPool) AppendToTxPool(tx *types.Transaction) error {
	p.Lock()
	defer p.Unlock()
	if err := p.appendToTxPool(tx); err != nil {
		return err
	}

	// Notify transaction accepted.
	go events.Notify(events.ETTransactionAccepted, tx)

	return nil
}

func (p *TxPool) appendToTxPool(tx *types.Transaction) error {
	//verify transaction with Concurrency
	if err := p.validator.CheckTransactionSanity(tx); err != nil {
		return err
	}
	if err := p.validator.CheckTransactionContext(tx); err != nil {
		return err
	}
	//verify transaction by p with lock
	if err := p.verifyTransactionWithTxnPool(tx); err != nil {
		return err
	}

	fee, err := p.feeHelper.GetTxFee(tx, p.chainParams.ElaAssetId)
	if err != nil {
		return err
	}
	tx.Fee = fee

	buf := new(bytes.Buffer)
	tx.Serialize(buf)
	tx.FeePerKB = tx.Fee * 1000 / common.Fixed64(len(buf.Bytes()))

	//add the transaction to process scope
	p.txnList[tx.Hash()] = tx

	return nil
}

// HaveTransaction returns if a transaction is in transaction pool by the given
// transaction id. If no transaction match the transaction id, return false
func (p *TxPool) HaveTransaction(txId common.Uint256) bool {
	p.RLock()
	defer p.RUnlock()
	_, ok := p.txnList[txId]
	return ok
}

// GetTxsInPool returns a copy of the transactions in transaction pool,
// It is safe to modify the returned map.
func (p *TxPool) GetTxsInPool() map[common.Uint256]*types.Transaction {
	p.RLock()
	defer p.RUnlock()
	copy := make(map[common.Uint256]*types.Transaction)
	for txId, tx := range p.txnList {
		copy[txId] = tx
	}
	return copy
}

//clean the trasaction Pool with committed block.
func (p *TxPool) CleanSubmittedTransactions(block *types.Block) error {
	p.Lock()
	defer p.Unlock()
	p.cleanTransactionList(block.Transactions)
	p.cleanUTXOList(block.Transactions)
	p.cleanMainchainTx(block.Transactions)
	return nil
}

//get the transaction by hash
func (p *TxPool) GetTransaction(hash common.Uint256) *types.Transaction {
	p.RLock()
	defer p.RUnlock()
	return p.getTransaction(hash)
}

func (p *TxPool) getTransaction(hash common.Uint256) *types.Transaction {
	return p.txnList[hash]
}

//verify transaction with txnpool
func (p *TxPool) verifyTransactionWithTxnPool(txn *types.Transaction) error {
	if txn.IsRechargeToSideChainTx() {
		// check if the recharge transaction includes duplicate mainchain tx in p
		if err := p.verifyDuplicateMainchainTx(txn); err != nil {
			return ruleError(ErrMainchainTxDuplicate, err.Error())
		}
	}

	// check if the transaction includes double spent UTXO inputs
	if err := p.verifyDoubleSpend(txn); err != nil {
		return ruleError(ErrDoubleSpend, err.Error())
	}

	return nil
}

//remove from associated map
func (p *TxPool) removeTransaction(tx *types.Transaction) {
	//1.remove from txnList
	p.delFromTxList(tx.Hash())
	//2.remove from UTXO list map
	for _, input := range tx.Inputs {
		p.delInputUTXOList(input)
	}
}

//check and add to utxo list pool
func (p *TxPool) verifyDoubleSpend(tx *types.Transaction) error {
	inputs := make([]*types.Input, 0, len(tx.Inputs))
	for _, input := range tx.Inputs {
		if txn := p.getInputUTXOList(input); txn != nil {
			return errors.New(fmt.Sprintf("double spent UTXO inputs detected, "+
				"transaction hash: %x, input: %s, index: %d",
				txn.Hash(), input.Previous.TxID, input.Previous.Index))
		}
		inputs = append(inputs, input)
	}

	for _, v := range inputs {
		p.addInputUTXOList(tx, v)
	}

	return nil
}

func (p *TxPool) IsDuplicateMainchainTx(mainchainTxHash common.Uint256) bool {
	p.RLock()
	defer p.RUnlock()
	_, ok := p.mainchainTxList[mainchainTxHash]
	if ok {
		return true
	}

	return false
}

//check and add to mainchain tx pool
func (p *TxPool) verifyDuplicateMainchainTx(txn *types.Transaction) error {
	rechargePayload, ok := txn.Payload.(*types.PayloadRechargeToSideChain)
	if !ok {
		return errors.New("convert the payload of recharge tx failed")
	}

	hash, err := rechargePayload.GetMainchainTxHash(txn.PayloadVersion)
	if err != nil {
		return err
	}
	_, exist := p.mainchainTxList[*hash]
	if exist {
		return errors.New("duplicate mainchain tx detected")
	}

	p.addMainchainTx(txn)

	return nil
}

//clean txnpool utxo map
func (p *TxPool) cleanUTXOList(txs []*types.Transaction) {
	for _, tx := range txs {
		for _, input := range tx.Inputs {
			p.delInputUTXOList(input)
		}
	}
}

// clean the trasaction Pool with committed transactions.
func (p *TxPool) cleanTransactionList(txns []*types.Transaction) error {
	cleaned := 0
	for _, txn := range txns {
		if txn.TxType == types.CoinBase {
			continue
		}
		inputUtxos, err := p.validator.db.GetTxReference(txn)
		if err != nil {
			log.Info(fmt.Sprintf("Transaction =%x not Exist in Pool when delete.", txn.Hash()), err)
			continue
		}
		for input := range inputUtxos {
			// we search transactions in transaction pool which have the same utxos with those transactions
			// in block. That is, if a transaction in the new-coming block uses the same utxo which a transaction
			// in transaction pool uses, then the latter one should be deleted, because one of its utxos has been used
			// by a confirmed transaction packed in the new-coming block.
			if tx := p.getInputUTXOList(input); tx != nil {
				if tx.Hash() == txn.Hash() {
					// it is evidently that two transactions with the same transaction id has exactly the same utxos with each
					// other. This is a special case of what we've said above.
					log.Debugf("duplicated transactions detected when adding a new block. "+
						" Delete transaction in the transaction pool. Transaction id: %x", tx.Hash())
				} else {
					log.Debugf("double spent UTXO inputs detected in transaction pool when adding a new block. "+
						"Delete transaction in the transaction pool. "+
						"block transaction hash: %x, transaction hash: %x, the same input: %s, index: %d",
						txn.Hash(), tx.Hash(), input.Previous.TxID, input.Previous.Index)
				}

				//1.remove from txnList
				delete(p.txnList, tx.Hash())

				//2.remove from UTXO list map
				for _, input := range tx.Inputs {
					p.delInputUTXOList(input)
				}

				cleaned++
			}
		}
	}

	log.Debugf("[cleanTransactionList] %d cleaned,  Remains %d in TxPool",
		cleaned, p.getTransactionCount())
	return nil
}

// clean the mainchain tx pool
func (p *TxPool) cleanMainchainTx(txs []*types.Transaction) {
	for _, txn := range txs {
		if txn.IsRechargeToSideChainTx() {
			rechargePayload := txn.Payload.(*types.PayloadRechargeToSideChain)
			mainTxHash, err := rechargePayload.GetMainchainTxHash(txn.PayloadVersion)
			if err != nil {
				log.Error("get hash failed when clean mainchain tx:", txn.Hash())
				continue
			}
			poolTx := p.mainchainTxList[*mainTxHash]
			if poolTx != nil {
				// delete tx
				p.delFromTxList(poolTx.Hash())
				// delete utxo
				for _, input := range poolTx.Inputs {
					p.delInputUTXOList(input)
				}
				// delete mainchain tx
				p.delMainchainTx(*mainTxHash)

			}
		}
	}
}

func (p *TxPool) delFromTxList(txId common.Uint256) bool {
	if _, ok := p.txnList[txId]; !ok {
		return false
	}
	delete(p.txnList, txId)
	return true
}

func (p *TxPool) copyTxList() map[common.Uint256]*types.Transaction {
	txnMap := make(map[common.Uint256]*types.Transaction, len(p.txnList))
	for txnId, txn := range p.txnList {
		txnMap[txnId] = txn
	}
	return txnMap
}

func (p *TxPool) getTransactionCount() int {
	return len(p.txnList)
}

func (p *TxPool) getInputUTXOList(input *types.Input) *types.Transaction {
	return p.inputUTXOList[input.ReferKey()]
}

func (p *TxPool) addInputUTXOList(tx *types.Transaction, input *types.Input) bool {
	id := input.ReferKey()
	_, ok := p.inputUTXOList[id]
	if ok {
		return false
	}
	p.inputUTXOList[id] = tx

	return true
}

func (p *TxPool) delInputUTXOList(input *types.Input) bool {
	id := input.ReferKey()
	_, ok := p.inputUTXOList[id]
	if !ok {
		return false
	}
	delete(p.inputUTXOList, id)
	return true
}

func (p *TxPool) addMainchainTx(txn *types.Transaction) {
	rechargePayload := txn.Payload.(*types.PayloadRechargeToSideChain)
	hash, err := rechargePayload.GetMainchainTxHash(txn.PayloadVersion)
	if err != nil {
		return
	}
	p.mainchainTxList[*hash] = txn
}

func (p *TxPool) delMainchainTx(hash common.Uint256) bool {
	_, ok := p.mainchainTxList[hash]
	if !ok {
		return false
	}
	delete(p.mainchainTxList, hash)
	return true
}

func (p *TxPool) MaybeAcceptTransaction(txn *types.Transaction) error {
	p.Lock()
	defer p.Unlock()
	txHash := txn.Hash()

	// Don't accept the transaction if it already exists in the p.  This
	// applies to orphan transactions as well.  This check is intended to
	// be a quick check to weed out duplicates.
	if txn := p.txnList[txHash]; txn != nil {
		return fmt.Errorf("already have transaction")
	}

	// A standalone transaction must not be a coinbase
	if txn.IsCoinBaseTx() {
		return fmt.Errorf("transaction is an individual coinbase")
	}

	if err := p.appendToTxPool(txn); err != nil {
		if e, ok := err.(*RuleError); ok {
			log.Infof("rule error when adding transaction pool, "+
				"error %s, desc %s", e.ErrorCode, e.Description)
		}
		return fmt.Errorf("VerifyTxs failed when AppendToTxnPool")
	}

	return nil
}

func (p *TxPool) RemoveTransaction(txn *types.Transaction) {
	p.Lock()
	defer p.Unlock()
	txHash := txn.Hash()
	for i := range txn.Outputs {
		input := types.Input{
			Previous: types.OutPoint{
				TxID:  txHash,
				Index: uint16(i),
			},
		}

		txn := p.getInputUTXOList(&input)
		if txn != nil {
			p.removeTransaction(txn)
		}
	}
}
