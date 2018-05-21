package blockchain

import (
	"bytes"
	"errors"
	"fmt"
	"sync"

	"github.com/elastos/Elastos.ELA.SideChain/config"
	. "github.com/elastos/Elastos.ELA.SideChain/core"
	. "github.com/elastos/Elastos.ELA.SideChain/errors"
	"github.com/elastos/Elastos.ELA.SideChain/events"
	"github.com/elastos/Elastos.ELA.SideChain/log"

	. "github.com/elastos/Elastos.ELA.Utility/common"
	ela "github.com/elastos/Elastos.ELA/core"
)

type TxPool struct {
	sync.RWMutex
	txnCnt  uint64                       // count
	txnList map[Uint256]*ela.Transaction // transaction which have been verifyed will put into this map
	//issueSummary  map[Uint256]Fixed64           // transaction which pass the verify will summary the amout to this map
	inputUTXOList map[string]*ela.Transaction // transaction which pass the verify will add the UTXO to this map
}

func (pool *TxPool) Init() {
	pool.Lock()
	defer pool.Unlock()
	pool.txnCnt = 0
	pool.inputUTXOList = make(map[string]*ela.Transaction)
	//pool.issueSummary = make(map[Uint256]Fixed64)
	pool.txnList = make(map[Uint256]*ela.Transaction)
}

//append transaction to txnpool when check ok.
//1.check  2.check with ledger(db) 3.check with pool
func (pool *TxPool) AppendToTxnPool(txn *ela.Transaction) ErrCode {
	//verify transaction with Concurrency
	if errCode := CheckTransactionSanity(txn); errCode != Success {
		log.Info("Transaction verification failed", txn.Hash())
		return errCode
	}
	if errCode := CheckTransactionContext(txn); errCode != Success {
		log.Info("Transaction verification with ledger failed", txn.Hash())
		return errCode
	}
	//verify transaction by pool with lock
	if ok := pool.verifyTransactionWithTxnPool(txn); !ok {
		return ErrDoubleSpend
	}

	if err := checkCrossChainTransaction(txn); err != nil {
		log.Info("Transaction verification failed:", err)
		return ErrDoubleSpend
	}

	txn.Fee = GetTxFee(txn, DefaultLedger.Blockchain.AssetID)
	buf := new(bytes.Buffer)
	txn.Serialize(buf)
	txn.FeePerKB = txn.Fee * 1000 / Fixed64(len(buf.Bytes()))
	//add the transaction to process scope
	pool.addToTxList(txn)
	return Success
}

//get the transaction in txnpool
func (pool *TxPool) GetTxnPool(byCount bool) map[Uint256]*ela.Transaction {
	pool.RLock()
	count := config.Parameters.MaxTxInBlock
	if count <= 0 {
		byCount = false
	}
	if len(pool.txnList) < count || !byCount {
		count = len(pool.txnList)
	}
	var num int
	txnMap := make(map[Uint256]*ela.Transaction, count)
	for txnId, tx := range pool.txnList {
		txnMap[txnId] = tx
		num++
		if num >= count {
			break
		}
	}
	pool.RUnlock()
	return txnMap
}

//clean the trasaction Pool with committed block.
func (pool *TxPool) CleanSubmittedTransactions(block *Block) error {
	pool.cleanTransactionList(block.Transactions)
	pool.cleanUTXOList(block.Transactions)
	//pool.cleanIssueSummary(block.Transactions)
	return nil
}

//get the transaction by hash
func (pool *TxPool) GetTransaction(hash Uint256) *ela.Transaction {
	pool.RLock()
	defer pool.RUnlock()
	return pool.txnList[hash]
}

//verify transaction with txnpool
func (pool *TxPool) verifyTransactionWithTxnPool(txn *ela.Transaction) bool {
	// check if the transaction includes double spent UTXO inputs
	if err := pool.verifyDoubleSpend(txn); err != nil {
		log.Info(err)
		return false
	}

	return true
}

//remove from associated map
func (pool *TxPool) removeTransaction(txn *ela.Transaction) {
	//1.remove from txnList
	pool.delFromTxList(txn.Hash())
	//2.remove from UTXO list map
	result, err := DefaultLedger.Store.GetTxReference(txn)
	if err != nil {
		log.Info(fmt.Sprintf("Transaction =%x not Exist in Pool when delete.", txn.Hash()))
		return
	}
	for UTXOTxInput := range result {
		pool.delInputUTXOList(UTXOTxInput)
	}
}

//check and add to utxo list pool
func (pool *TxPool) verifyDoubleSpend(txn *ela.Transaction) error {
	reference, err := DefaultLedger.Store.GetTxReference(txn)
	if err != nil {
		return err
	}
	inputs := []*ela.Input{}
	for k := range reference {
		if txn := pool.getInputUTXOList(k); txn != nil {
			return errors.New(fmt.Sprintf("double spent UTXO inputs detected, "+
				"transaction hash: %x, input: %s, index: %d",
				txn.Hash(), k.Previous.TxID, k.Previous.Index))
		}
		inputs = append(inputs, k)
	}
	for _, v := range inputs {
		pool.addInputUTXOList(txn, v)
	}

	return nil
}

//clean txnpool utxo map
func (pool *TxPool) cleanUTXOList(txs []*ela.Transaction) {
	for _, txn := range txs {
		inputUtxos, _ := DefaultLedger.Store.GetTxReference(txn)
		for Utxoinput, _ := range inputUtxos {
			pool.delInputUTXOList(Utxoinput)
		}
	}
}

// clean the trasaction Pool with committed transactions.
func (pool *TxPool) cleanTransactionList(txns []*ela.Transaction) error {
	cleaned := 0
	txnsNum := len(txns)
	for _, txn := range txns {
		if txn.TxType == ela.CoinBase {
			txnsNum = txnsNum - 1
			continue
		}
		if pool.delFromTxList(txn.Hash()) {
			cleaned++
		}
	}
	if txnsNum != cleaned {
		log.Info(fmt.Sprintf("The Transactions num Unmatched. Expect %d, got %d .\n", txnsNum, cleaned))
	}
	log.Debug(fmt.Sprintf("[cleanTransactionList],transaction %d Requested, %d cleaned, Remains %d in TxPool", txnsNum, cleaned, pool.GetTransactionCount()))
	return nil
}

func (pool *TxPool) addToTxList(txn *ela.Transaction) bool {
	pool.Lock()
	defer pool.Unlock()
	txnHash := txn.Hash()
	if _, ok := pool.txnList[txnHash]; ok {
		return false
	}
	pool.txnList[txnHash] = txn
	DefaultLedger.Blockchain.BCEvents.Notify(events.EventNewTransactionPutInPool, txn)
	return true
}

func (pool *TxPool) delFromTxList(txId Uint256) bool {
	pool.Lock()
	defer pool.Unlock()
	if _, ok := pool.txnList[txId]; !ok {
		return false
	}
	delete(pool.txnList, txId)
	return true
}

func (pool *TxPool) copyTxList() map[Uint256]*ela.Transaction {
	pool.RLock()
	defer pool.RUnlock()
	txnMap := make(map[Uint256]*ela.Transaction, len(pool.txnList))
	for txnId, txn := range pool.txnList {
		txnMap[txnId] = txn
	}
	return txnMap
}

func (pool *TxPool) GetTransactionCount() int {
	pool.RLock()
	defer pool.RUnlock()
	return len(pool.txnList)
}

func (pool *TxPool) getInputUTXOList(input *ela.Input) *ela.Transaction {
	pool.RLock()
	defer pool.RUnlock()
	return pool.inputUTXOList[input.ReferKey()]
}

func (pool *TxPool) addInputUTXOList(tx *ela.Transaction, input *ela.Input) bool {
	pool.Lock()
	defer pool.Unlock()
	id := input.ReferKey()
	_, ok := pool.inputUTXOList[id]
	if ok {
		return false
	}
	pool.inputUTXOList[id] = tx

	return true
}

func (pool *TxPool) delInputUTXOList(input *ela.Input) bool {
	pool.Lock()
	defer pool.Unlock()
	id := input.ReferKey()
	_, ok := pool.inputUTXOList[id]
	if !ok {
		return false
	}
	delete(pool.inputUTXOList, id)
	return true
}

func (pool *TxPool) MaybeAcceptTransaction(txn *ela.Transaction) error {
	txHash := txn.Hash()

	// Don't accept the transaction if it already exists in the pool.  This
	// applies to orphan transactions as well.  This check is intended to
	// be a quick check to weed out duplicates.
	if txn := pool.GetTransaction(txHash); txn != nil {
		return fmt.Errorf("already have transaction")
	}

	// A standalone transaction must not be a coinbase
	if txn.IsCoinBaseTx() {
		return fmt.Errorf("transaction is an individual coinbase")
	}

	if errCode := pool.AppendToTxnPool(txn); errCode != Success {
		return fmt.Errorf("VerifyTxs failed when AppendToTxnPool")
	}

	return nil
}

func (pool *TxPool) RemoveTransaction(txn *ela.Transaction) {
	txHash := txn.Hash()
	for i := range txn.Outputs {
		input := ela.Input{
			Previous: ela.OutPoint{
				TxID:  txHash,
				Index: uint16(i),
			},
		}

		txn := pool.getInputUTXOList(&input)
		if txn != nil {
			pool.removeTransaction(txn)
		}
	}
}

func GetTxFee(tx *ela.Transaction, assetId Uint256) Fixed64 {
	feeMap, err := GetTxFeeMap(tx)
	if err != nil {
		return 0
	}

	return feeMap[assetId]
}

func GetTxFeeMap(tx *ela.Transaction) (map[Uint256]Fixed64, error) {
	feeMap := make(map[Uint256]Fixed64)

	if tx.IsIssueTokenTx() {
		depositPayload := tx.Payload.(*ela.PayloadIssueToken)
		mainChainTransaction := new(ela.Transaction)
		reader := bytes.NewReader(depositPayload.MainChainTransaction)
		if err := mainChainTransaction.Deserialize(reader); err != nil {
			return nil, errors.New("GetTxFeeMap mainChainTransaction deserialize failed")
		}

		crossChainPayload := mainChainTransaction.Payload.(*ela.PayloadTransferCrossChainAsset)

		for _, v := range tx.Outputs {
			var mcAmount Fixed64
			for i := 0; i < len(crossChainPayload.CrossChainAddress); i++ {
				targetAddress, err := v.ProgramHash.ToAddress()
				if err != nil {
					return nil, err
				}
				if targetAddress == crossChainPayload.CrossChainAddress[i] {
					mcAmount = mainChainTransaction.Outputs[i].Value
				}
			}

			amount, ok := feeMap[v.AssetID]
			if ok {
				feeMap[v.AssetID] = amount + mcAmount - v.Value
			} else {
				feeMap[v.AssetID] = mcAmount - v.Value
			}
		}

		return feeMap, nil
	}

	reference, err := DefaultLedger.Store.GetTxReference(tx)
	if err != nil {
		return nil, err
	}

	var inputs = make(map[Uint256]Fixed64)
	var outputs = make(map[Uint256]Fixed64)
	for _, v := range reference {
		amout, ok := inputs[v.AssetID]
		if ok {
			inputs[v.AssetID] = amout + v.Value
		} else {
			inputs[v.AssetID] = v.Value
		}
	}

	for _, v := range tx.Outputs {
		amout, ok := outputs[v.AssetID]
		if ok {
			outputs[v.AssetID] = amout + v.Value
		} else {
			outputs[v.AssetID] = v.Value
		}
	}

	//calc the balance of input vs output
	for outputAssetid, outputValue := range outputs {
		if inputValue, ok := inputs[outputAssetid]; ok {
			feeMap[outputAssetid] = inputValue - outputValue
		} else {
			feeMap[outputAssetid] -= outputValue
		}
	}
	for inputAssetid, inputValue := range inputs {
		if _, exist := feeMap[inputAssetid]; !exist {
			feeMap[inputAssetid] += inputValue
		}
	}
	return feeMap, nil
}
