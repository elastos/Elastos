package blockchain

import (
	"bytes"
	"errors"
	"fmt"
	"sync"

	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/config"
	. "github.com/elastos/Elastos.ELA/core"
	. "github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/events"
	"github.com/elastos/Elastos.ELA/log"
)

type TxPool struct {
	sync.RWMutex
	txnCnt  uint64                   // count
	txnList map[Uint256]*Transaction // transaction which have been verifyed will put into this map
	//issueSummary  map[Uint256]Fixed64           // transaction which pass the verify will summary the amout to this map
	inputUTXOList   map[string]*Transaction // transaction which pass the verify will add the UTXO to this map
	sidechainTxList map[string]struct{}     // sidechain tx pool
}

func (pool *TxPool) Init() {
	pool.Lock()
	defer pool.Unlock()
	pool.txnCnt = 0
	pool.inputUTXOList = make(map[string]*Transaction)
	//pool.issueSummary = make(map[Uint256]Fixed64)
	pool.txnList = make(map[Uint256]*Transaction)
	pool.sidechainTxList = make(map[string]struct{})
}

//append transaction to txnpool when check ok.
//1.check  2.check with ledger(db) 3.check with pool
func (pool *TxPool) AppendToTxnPool(txn *Transaction) ErrCode {

	if txn.IsCoinBaseTx() {
		log.Warn("coinbase cannot be added into transaction pool", txn.Hash().String())
		return ErrIneffectiveCoinbase
	}

	//verify transaction with Concurrency
	if errCode := CheckTransactionSanity(CheckTxOut, txn); errCode != Success {
		log.Warn("[TxPool CheckTransactionSanity] failed", txn.Hash().String())
		return errCode
	}
	if errCode := CheckTransactionContext(txn); errCode != Success {
		log.Warn("[TxPool CheckTransactionContext] failed", txn.Hash().String())
		return errCode
	}

	//verify transaction by pool with lock
	if errCode := pool.verifyTransactionWithTxnPool(txn); errCode != Success {
		log.Warn("[TxPool verifyTransactionWithTxnPool] failed", txn.Hash())
		return errCode
	}

	txn.Fee = GetTxFee(txn, DefaultLedger.Blockchain.AssetID)
	buf := new(bytes.Buffer)
	txn.Serialize(buf)
	txn.FeePerKB = txn.Fee * 1000 / Fixed64(len(buf.Bytes()))
	//add the transaction to process scope
	if ok := pool.addToTxList(txn); !ok {
		// reject duplicated transaction
		log.Debugf("Transaction duplicate %s", txn.Hash().String())
		return ErrTransactionDuplicate
	}
	return Success
}

//get the transaction in txnpool
func (pool *TxPool) GetTransactionPool(hasMaxCount bool) map[Uint256]*Transaction {
	pool.RLock()
	count := config.Parameters.MaxTxsInBlock
	if count <= 0 {
		hasMaxCount = false
	}
	if len(pool.txnList) < count || !hasMaxCount {
		count = len(pool.txnList)
	}
	var num int
	txnMap := make(map[Uint256]*Transaction, count)
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
	pool.cleanTransactions(block.Transactions)
	pool.cleanSidechainTx(block.Transactions)

	return nil
}

func (pool *TxPool) cleanTransactions(blockTxs []*Transaction) error {
	txCountInPool := pool.GetTransactionCount()
	deleteCount := 0
	for _, blockTx := range blockTxs {
		if blockTx.TxType == CoinBase {
			continue
		}

		inputUtxos, err := DefaultLedger.Store.GetTxReference(blockTx)
		if err != nil {
			log.Info("get tx reference failed", err)
		}
		for input := range inputUtxos {
			// we search transactions in transaction pool which have the same utxos with those transactions
			// in block. That is, if a transaction in the new-coming block uses the same utxo which a transaction
			// in transaction pool uses, then the latter one should be deleted, because one of its utxos has been used
			// by a confirmed transaction packed in the new-coming block.
			if tx := pool.getInputUTXOList(input); tx != nil {
				if tx.Hash() == blockTx.Hash() {
					// it is evidently that two transactions with the same transaction id has exactly the same utxos with each
					// other. This is a special case of what we've said above.
					log.Debugf("duplicated transactions detected when adding a new block. "+
						" Delete transaction in the transaction pool. Transaction id: %x", tx.Hash())
				} else {
					log.Debugf("double spent UTXO inputs detected in transaction pool when adding a new block. "+
						"Delete transaction in the transaction pool. "+
						"block transaction hash: %x, transaction hash: %x, the same input: %s, index: %d",
						blockTx.Hash(), tx.Hash(), input.Previous.TxID, input.Previous.Index)
				}
				pool.delFromTxList(tx.Hash())
				pool.delInputUTXOList(input)
				deleteCount++
			}
		}
	}
	log.Debug(fmt.Sprintf("[cleanTransactionList],transaction %d in block, %d in transaction pool before, %d deleted,"+
		" Remains %d in TxPool",
		len(blockTxs), txCountInPool, deleteCount, pool.GetTransactionCount()))
	return nil
}

//get the transaction by hash
func (pool *TxPool) GetTransaction(hash Uint256) *Transaction {
	pool.RLock()
	defer pool.RUnlock()
	return pool.txnList[hash]
}

//verify transaction with txnpool
func (pool *TxPool) verifyTransactionWithTxnPool(txn *Transaction) ErrCode {
	if txn.IsSideChainPowTx() {
		// check and replace the duplicate sidechainpow tx
		pool.replaceDuplicateSideChainPowTx(txn)
	} else if txn.IsWithdrawFromSideChainTx() {
		// check if the withdraw transaction includes duplicate sidechain tx in pool
		if err := pool.verifyDuplicateSidechainTx(txn); err != nil {
			log.Warn(err)
			return ErrSidechainTxDuplicate
		}
	}

	// check if the transaction includes double spent UTXO inputs
	if err := pool.verifyDoubleSpend(txn); err != nil {
		log.Warn(err)
		return ErrDoubleSpend
	}

	return Success
}

//remove from associated map
func (pool *TxPool) removeTransaction(txn *Transaction) {
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
func (pool *TxPool) verifyDoubleSpend(txn *Transaction) error {
	reference, err := DefaultLedger.Store.GetTxReference(txn)
	if err != nil {
		return err
	}
	inputs := []*Input{}
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

func (pool *TxPool) IsDuplicateSidechainTx(sidechainTxHash string) bool {
	_, ok := pool.sidechainTxList[sidechainTxHash]
	if ok {
		return true
	}

	return false
}

//check and add to sidechain tx pool
func (pool *TxPool) verifyDuplicateSidechainTx(txn *Transaction) error {
	withPayload, ok := txn.Payload.(*PayloadWithdrawFromSideChain)
	if !ok {
		return errors.New("convert the payload of withdraw tx failed")
	}

	sidechainTxs := []string{}
	for _, hash := range withPayload.SideChainTransactionHashes {
		_, ok := pool.sidechainTxList[hash]
		if ok {
			return errors.New("duplicate sidechain tx detected")
		}
		sidechainTxs = append(sidechainTxs, hash)
	}
	for _, v := range sidechainTxs {
		pool.addSidechainTx(v)
	}

	return nil
}

// check and replace the duplicate sidechainpow tx
func (pool *TxPool) replaceDuplicateSideChainPowTx(txn *Transaction) {
	for _, v := range pool.txnList {
		if v.TxType == SideChainPow {
			oldPayload := v.Payload.Data(SideChainPowPayloadVersion)
			oldGenesisHashData := oldPayload[32:64]

			newPayload := txn.Payload.Data(SideChainPowPayloadVersion)
			newGenesisHashData := newPayload[32:64]

			if bytes.Equal(oldGenesisHashData, newGenesisHashData) {
				txid := txn.Hash()
				log.Warn("replace sidechainpow transaction, txid=", txid.String())
				pool.removeTransaction(v)
			}
		}
	}
}

// clean the sidechain tx pool
func (pool *TxPool) cleanSidechainTx(txs []*Transaction) {
	for _, txn := range txs {
		if txn.IsWithdrawFromSideChainTx() {
			withPayload := txn.Payload.(*PayloadWithdrawFromSideChain)
			for _, hash := range withPayload.SideChainTransactionHashes {
				pool.delSidechainTx(hash)
			}
		}
	}
}

func (pool *TxPool) addToTxList(txn *Transaction) bool {
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

func (pool *TxPool) copyTxList() map[Uint256]*Transaction {
	pool.RLock()
	defer pool.RUnlock()
	txnMap := make(map[Uint256]*Transaction, len(pool.txnList))
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

func (pool *TxPool) getInputUTXOList(input *Input) *Transaction {
	pool.RLock()
	defer pool.RUnlock()
	return pool.inputUTXOList[input.ReferKey()]
}

func (pool *TxPool) addInputUTXOList(tx *Transaction, input *Input) bool {
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

func (pool *TxPool) delInputUTXOList(input *Input) bool {
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

func (pool *TxPool) addSidechainTx(hash string) bool {
	pool.Lock()
	defer pool.Unlock()
	_, ok := pool.sidechainTxList[hash]
	if ok {
		return false
	}
	pool.sidechainTxList[hash] = struct{}{}

	return true
}

func (pool *TxPool) delSidechainTx(hash string) bool {
	pool.Lock()
	defer pool.Unlock()
	_, ok := pool.sidechainTxList[hash]
	if !ok {
		return false
	}
	delete(pool.sidechainTxList, hash)
	return true
}

func (pool *TxPool) MaybeAcceptTransaction(txn *Transaction) error {
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

func (pool *TxPool) RemoveTransaction(txn *Transaction) {
	txHash := txn.Hash()
	for i := range txn.Outputs {
		input := Input{
			Previous: OutPoint{
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

func GetTxFee(tx *Transaction, assetId Uint256) Fixed64 {
	feeMap, err := GetTxFeeMap(tx)
	if err != nil {
		return 0
	}

	return feeMap[assetId]
}

func GetTxFeeMap(tx *Transaction) (map[Uint256]Fixed64, error) {
	feeMap := make(map[Uint256]Fixed64)
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
