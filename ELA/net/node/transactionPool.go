package node

import (
	"Elastos.ELA/common"
	"Elastos.ELA/common/config"
	"Elastos.ELA/common/log"
	"Elastos.ELA/core/ledger"
	"Elastos.ELA/core/transaction"
	tx "Elastos.ELA/core/transaction"
	. "Elastos.ELA/errors"
	"Elastos.ELA/events"
	"bytes"
	"errors"
	"fmt"
	"sync"
)

var (
	zeroHash = common.Uint256{}
)

type TXNPool struct {
	sync.RWMutex
	txnCnt  uint64                                      // count
	txnList map[common.Uint256]*transaction.Transaction // transaction which have been verifyed will put into this map
	//issueSummary  map[common.Uint256]common.Fixed64           // transaction which pass the verify will summary the amout to this map
	inputUTXOList map[string]*transaction.Transaction // transaction which pass the verify will add the UTXO to this map
}

func (this *TXNPool) init() {
	this.Lock()
	defer this.Unlock()
	this.txnCnt = 0
	this.inputUTXOList = make(map[string]*transaction.Transaction)
	//this.issueSummary = make(map[common.Uint256]common.Fixed64)
	this.txnList = make(map[common.Uint256]*transaction.Transaction)
}

//append transaction to txnpool when check ok.
//1.check transaction. 2.check with ledger(db) 3.check with pool
func (this *TXNPool) AppendToTxnPool(txn *transaction.Transaction) ErrCode {
	//verify transaction with Concurrency
	if errCode := ledger.CheckTransactionSanity(txn); errCode != Success {
		log.Info("Transaction verification failed", txn.Hash())
		return errCode
	}
	if errCode := ledger.CheckTransactionContext(txn, ledger.DefaultLedger); errCode != Success {
		log.Info("Transaction verification with ledger failed", txn.Hash())
		return errCode
	}
	//verify transaction by pool with lock
	if ok := this.verifyTransactionWithTxnPool(txn); !ok {
		return ErrDoubleSpend
	}

	txn.Fee = common.Fixed64(txn.GetFee(ledger.DefaultLedger.Blockchain.AssetID))
	b_buf := new(bytes.Buffer)
	txn.Serialize(b_buf)
	txn.FeePerKB = txn.Fee * 1000 / common.Fixed64(len(b_buf.Bytes()))
	//add the transaction to process scope
	this.addtxnList(txn)
	return Success
}

//get the transaction in txnpool
func (this *TXNPool) GetTxnPool(byCount bool) map[common.Uint256]*transaction.Transaction {
	this.RLock()
	count := config.Parameters.MaxTxInBlock
	if count <= 0 {
		byCount = false
	}
	if len(this.txnList) < count || !byCount {
		count = len(this.txnList)
	}
	var num int
	txnMap := make(map[common.Uint256]*transaction.Transaction, count)
	for txnId, tx := range this.txnList {
		txnMap[txnId] = tx
		num++
		if num >= count {
			break
		}
	}
	this.RUnlock()
	return txnMap
}

//clean the trasaction Pool with committed block.
func (this *TXNPool) CleanSubmittedTransactions(block *ledger.Block) error {
	this.cleanTransactionList(block.Transactions)
	this.cleanUTXOList(block.Transactions)
	//this.cleanIssueSummary(block.Transactions)
	return nil
}

//get the transaction by hash
func (this *TXNPool) GetTransaction(hash common.Uint256) *transaction.Transaction {
	this.RLock()
	defer this.RUnlock()
	return this.txnList[hash]
}

//verify transaction with txnpool
func (this *TXNPool) verifyTransactionWithTxnPool(txn *transaction.Transaction) bool {
	// check if the transaction includes double spent UTXO inputs
	if err := this.verifyDoubleSpend(txn); err != nil {
		log.Info(err)
		return false
	}

	return true
}

//remove from associated map
func (this *TXNPool) removeTransaction(txn *transaction.Transaction) {
	//1.remove from txnList
	this.deltxnList(txn)
	//2.remove from UTXO list map
	result, err := txn.GetReference()
	if err != nil {
		log.Info(fmt.Sprintf("Transaction =%x not Exist in Pool when delete.", txn.Hash()))
		return
	}
	for UTXOTxInput := range result {
		this.delInputUTXOList(UTXOTxInput)
	}
}

//check and add to utxo list pool
func (this *TXNPool) verifyDoubleSpend(txn *transaction.Transaction) error {
	reference, err := txn.GetReference()
	if err != nil {
		return err
	}
	inputs := []*transaction.Input{}
	for k := range reference {
		if txn := this.getInputUTXOList(k); txn != nil {
			return errors.New(fmt.Sprintf("double spent UTXO inputs detected, "+
				"transaction hash: %x, input: %s, index: %d",
				txn.Hash(), k.Previous.TxID, k.Previous.Index))
		}
		inputs = append(inputs, k)
	}
	for _, v := range inputs {
		this.addInputUTXOList(txn, v)
	}

	return nil
}

//clean txnpool utxo map
func (this *TXNPool) cleanUTXOList(txs []*transaction.Transaction) {
	for _, txn := range txs {
		inputUtxos, _ := txn.GetReference()
		for Utxoinput, _ := range inputUtxos {
			this.delInputUTXOList(Utxoinput)
		}
	}
}

// clean the trasaction Pool with committed transactions.
func (this *TXNPool) cleanTransactionList(txns []*transaction.Transaction) error {
	cleaned := 0
	txnsNum := len(txns)
	for _, txn := range txns {
		if txn.TxType == transaction.CoinBase {
			txnsNum = txnsNum - 1
			continue
		}
		if this.deltxnList(txn) {
			cleaned++
		}
	}
	if txnsNum != cleaned {
		log.Info(fmt.Sprintf("The Transactions num Unmatched. Expect %d, got %d .\n", txnsNum, cleaned))
	}
	log.Debug(fmt.Sprintf("[cleanTransactionList],transaction %d Requested, %d cleaned, Remains %d in TxPool", txnsNum, cleaned, this.GetTransactionCount()))
	return nil
}

func (this *TXNPool) addtxnList(txn *transaction.Transaction) bool {
	this.Lock()
	defer this.Unlock()
	txnHash := txn.Hash()
	if _, ok := this.txnList[txnHash]; ok {
		return false
	}
	this.txnList[txnHash] = txn
	ledger.DefaultLedger.Blockchain.BCEvents.Notify(events.EventNewTransactionPutInPool, txn)
	return true
}

func (this *TXNPool) deltxnList(tx *transaction.Transaction) bool {
	this.Lock()
	defer this.Unlock()
	txHash := tx.Hash()
	if _, ok := this.txnList[txHash]; !ok {
		return false
	}
	delete(this.txnList, tx.Hash())
	return true
}

func (this *TXNPool) copytxnList() map[common.Uint256]*transaction.Transaction {
	this.RLock()
	defer this.RUnlock()
	txnMap := make(map[common.Uint256]*transaction.Transaction, len(this.txnList))
	for txnId, txn := range this.txnList {
		txnMap[txnId] = txn
	}
	return txnMap
}

func (this *TXNPool) GetTransactionCount() int {
	this.RLock()
	defer this.RUnlock()
	return len(this.txnList)
}

func (this *TXNPool) getInputUTXOList(input *transaction.Input) *transaction.Transaction {
	this.RLock()
	defer this.RUnlock()
	return this.inputUTXOList[input.ReferKey()]
}

func (this *TXNPool) addInputUTXOList(tx *transaction.Transaction, input *transaction.Input) bool {
	this.Lock()
	defer this.Unlock()
	id := input.ReferKey()
	_, ok := this.inputUTXOList[id]
	if ok {
		return false
	}
	this.inputUTXOList[id] = tx

	return true
}

func (this *TXNPool) delInputUTXOList(input *transaction.Input) bool {
	this.Lock()
	defer this.Unlock()
	id := input.ReferKey()
	_, ok := this.inputUTXOList[id]
	if !ok {
		return false
	}
	delete(this.inputUTXOList, id)
	return true
}

func (this *TXNPool) MaybeAcceptTransaction(txn *tx.Transaction) error {
	txHash := txn.Hash()

	// Don't accept the transaction if it already exists in the pool.  This
	// applies to orphan transactions as well.  This check is intended to
	// be a quick check to weed out duplicates.
	if txn := this.GetTransaction(txHash); txn != nil {
		return fmt.Errorf("already have transaction")
	}

	// A standalone transaction must not be a coinbase transaction.
	if txn.IsCoinBaseTx() {
		return fmt.Errorf("transaction is an individual coinbase")
	}

	if errCode := this.AppendToTxnPool(txn); errCode != Success {
		return fmt.Errorf("VerifyTxs failed when AppendToTxnPool")
	}

	return nil
}

func (this *TXNPool) RemoveTransaction(txn *tx.Transaction) {
	txHash := txn.Hash()
	for i := range txn.Outputs {
		input := tx.Input{
			Previous: tx.OutPoint{
				TxID:  txHash,
				Index: uint16(i),
			},
		}

		txn := this.getInputUTXOList(&input)
		if txn != nil {
			this.removeTransaction(txn)
		}
	}
}
