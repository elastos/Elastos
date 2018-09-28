package mempool

import (
	"bytes"
	"errors"
	"fmt"
	"sync"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/core"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type TxPool struct {
	assetId   Uint256
	db        blockchain.IChainStore
	validator *Validator
	feeHelper *FeeHelper
	sync.RWMutex
	txCount         uint64                        // count
	txnList         map[Uint256]*core.Transaction // transaction which have been verifyed will put into this map
	inputUTXOList   map[string]*core.Transaction  // transaction which pass the verify will add the UTXO to this map
	mainchainTxList map[Uint256]*core.Transaction // mainchain tx pool
}

func New(cfg *blockchain.Config) *TxPool {
	p := TxPool{
		assetId:         cfg.AssetId,
		db:              cfg.ChainStore,
		validator:       cfg.TxValidator,
		feeHelper:       cfg.TxFeeHelper,
		txCount:         0,
		inputUTXOList:   make(map[string]*core.Transaction),
		txnList:         make(map[Uint256]*core.Transaction),
		mainchainTxList: make(map[Uint256]*core.Transaction),
	}
	return &p
}

//append transaction to txnpool when check ok.
//1.check  2.check with ledger(db) 3.check with pool
func (p *TxPool) AppendToTxPool(txn *core.Transaction) error {
	//verify transaction with Concurrency
	if err := p.validator.CheckTransactionSanity(txn); err != nil {
		return err
	}
	if err := p.validator.CheckTransactionContext(txn); err != nil {
		return err
	}
	//verify transaction by p with lock
	if err := p.VerifyTransactionWithTxnPool(txn); err != nil {
		return err
	}

	txn.Fee = p.feeHelper.GetTxFee(txn, p.assetId)
	buf := new(bytes.Buffer)
	txn.Serialize(buf)
	txn.FeePerKB = txn.Fee * 1000 / Fixed64(len(buf.Bytes()))
	//add the transaction to process scope
	p.addToTxList(txn)
	return nil
}

// HaveTransaction returns if a transaction is in transaction pool by the given
// transaction id. If no transaction match the transaction id, return false
func (p *TxPool) HaveTransaction(txId Uint256) bool {
	p.RLock()
	defer p.RUnlock()
	_, ok := p.txnList[txId]
	return ok
}

// GetTxsInPool returns a copy of the transactions in transaction pool,
// It is safe to modify the returned map.
func (p *TxPool) GetTxsInPool() map[Uint256]*core.Transaction {
	p.RLock()
	defer p.RUnlock()
	copy := make(map[Uint256]*core.Transaction)
	for txId, tx := range p.txnList {
		copy[txId] = tx
	}
	return copy
}

//clean the trasaction Pool with committed block.
func (p *TxPool) CleanSubmittedTransactions(block *core.Block) error {
	p.cleanTransactionList(block.Transactions)
	p.cleanUTXOList(block.Transactions)
	p.cleanMainchainTx(block.Transactions)
	return nil
}

//get the transaction by hash
func (p *TxPool) GetTransaction(hash Uint256) *core.Transaction {
	p.RLock()
	defer p.RUnlock()
	return p.txnList[hash]
}

//verify transaction with txnpool
func (p *TxPool) VerifyTransactionWithTxnPool(txn *core.Transaction) error {
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
func (p *TxPool) removeTransaction(tx *core.Transaction) {
	//1.remove from txnList
	p.delFromTxList(tx.Hash())
	//2.remove from UTXO list map
	for _, input := range tx.Inputs {
		p.delInputUTXOList(input)
	}
}

//check and add to utxo list pool
func (p *TxPool) verifyDoubleSpend(txn *core.Transaction) error {
	reference, err := p.db.GetTxReference(txn)
	if err != nil {
		return err
	}
	inputs := []*core.Input{}
	for k := range reference {
		if txn := p.getInputUTXOList(k); txn != nil {
			return errors.New(fmt.Sprintf("double spent UTXO inputs detected, "+
				"transaction hash: %x, input: %s, index: %d",
				txn.Hash(), k.Previous.TxID, k.Previous.Index))
		}
		inputs = append(inputs, k)
	}
	for _, v := range inputs {
		p.addInputUTXOList(txn, v)
	}

	return nil
}

func (p *TxPool) IsDuplicateMainchainTx(mainchainTxHash Uint256) bool {
	_, ok := p.mainchainTxList[mainchainTxHash]
	if ok {
		return true
	}

	return false
}

//check and add to mainchain tx pool
func (p *TxPool) verifyDuplicateMainchainTx(txn *core.Transaction) error {
	rechargePayload, ok := txn.Payload.(*core.PayloadRechargeToSideChain)
	if !ok {
		return errors.New("convert the payload of recharge tx failed")
	}

	hash, err := rechargePayload.GetMainchainTxHash()
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
func (p *TxPool) cleanUTXOList(txs []*core.Transaction) {
	for _, txn := range txs {
		inputUtxos, _ := p.db.GetTxReference(txn)
		for Utxoinput, _ := range inputUtxos {
			p.delInputUTXOList(Utxoinput)
		}
	}
}

// clean the trasaction Pool with committed transactions.
func (p *TxPool) cleanTransactionList(txns []*core.Transaction) error {
	cleaned := 0
	txnsNum := len(txns)
	for _, txn := range txns {
		if txn.TxType == core.CoinBase {
			txnsNum = txnsNum - 1
			continue
		}
		if p.delFromTxList(txn.Hash()) {
			cleaned++
		}
	}
	if txnsNum != cleaned {
		log.Info(fmt.Sprintf("The Transactions num Unmatched. Expect %d, got %d .\n", txnsNum, cleaned))
	}
	log.Debug(fmt.Sprintf("[cleanTransactionList],transaction %d Requested, %d cleaned, Remains %d in TxPool", txnsNum, cleaned, p.GetTransactionCount()))
	return nil
}

// clean the mainchain tx pool
func (p *TxPool) cleanMainchainTx(txs []*core.Transaction) {
	for _, txn := range txs {
		if txn.IsRechargeToSideChainTx() {
			rechargePayload := txn.Payload.(*core.PayloadRechargeToSideChain)
			mainTxHash, err := rechargePayload.GetMainchainTxHash()
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

func (p *TxPool) addToTxList(txn *core.Transaction) bool {
	p.Lock()
	defer p.Unlock()
	txnHash := txn.Hash()
	if _, ok := p.txnList[txnHash]; ok {
		return false
	}
	p.txnList[txnHash] = txn
	return true
}

func (p *TxPool) delFromTxList(txId Uint256) bool {
	p.Lock()
	defer p.Unlock()
	if _, ok := p.txnList[txId]; !ok {
		return false
	}
	delete(p.txnList, txId)
	return true
}

func (p *TxPool) copyTxList() map[Uint256]*core.Transaction {
	p.RLock()
	defer p.RUnlock()
	txnMap := make(map[Uint256]*core.Transaction, len(p.txnList))
	for txnId, txn := range p.txnList {
		txnMap[txnId] = txn
	}
	return txnMap
}

func (p *TxPool) GetTransactionCount() int {
	p.RLock()
	defer p.RUnlock()
	return len(p.txnList)
}

func (p *TxPool) getInputUTXOList(input *core.Input) *core.Transaction {
	p.RLock()
	defer p.RUnlock()
	return p.inputUTXOList[input.ReferKey()]
}

func (p *TxPool) addInputUTXOList(tx *core.Transaction, input *core.Input) bool {
	p.Lock()
	defer p.Unlock()
	id := input.ReferKey()
	_, ok := p.inputUTXOList[id]
	if ok {
		return false
	}
	p.inputUTXOList[id] = tx

	return true
}

func (p *TxPool) delInputUTXOList(input *core.Input) bool {
	p.Lock()
	defer p.Unlock()
	id := input.ReferKey()
	_, ok := p.inputUTXOList[id]
	if !ok {
		return false
	}
	delete(p.inputUTXOList, id)
	return true
}

func (p *TxPool) addMainchainTx(txn *core.Transaction) {
	p.Lock()
	defer p.Unlock()
	rechargePayload := txn.Payload.(*core.PayloadRechargeToSideChain)
	hash, err := rechargePayload.GetMainchainTxHash()
	if err != nil {
		return
	}
	p.mainchainTxList[*hash] = txn
}

func (p *TxPool) delMainchainTx(hash Uint256) bool {
	p.Lock()
	defer p.Unlock()
	_, ok := p.mainchainTxList[hash]
	if !ok {
		return false
	}
	delete(p.mainchainTxList, hash)
	return true
}

func (p *TxPool) MaybeAcceptTransaction(txn *core.Transaction) error {
	txHash := txn.Hash()

	// Don't accept the transaction if it already exists in the p.  This
	// applies to orphan transactions as well.  This check is intended to
	// be a quick check to weed out duplicates.
	if txn := p.GetTransaction(txHash); txn != nil {
		return fmt.Errorf("already have transaction")
	}

	// A standalone transaction must not be a coinbase
	if txn.IsCoinBaseTx() {
		return fmt.Errorf("transaction is an individual coinbase")
	}

	if err := p.AppendToTxPool(txn); err != nil {
		return fmt.Errorf("VerifyTxs failed when AppendToTxnPool")
	}

	return nil
}

func (p *TxPool) RemoveTransaction(txn *core.Transaction) {
	txHash := txn.Hash()
	for i := range txn.Outputs {
		input := core.Input{
			Previous: core.OutPoint{
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
