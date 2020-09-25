package mempool

import (
	"bytes"
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
	Chain       *blockchain.BlockChain
	ChainStore  *blockchain.ChainStore
	SpvService  *spv.Service
	Validator   *Validator
	FeeHelper   *FeeHelper
}

type TxPool struct {
	conflictManager
	chainParams *config.Params
	validator   *Validator
	feeHelper   *FeeHelper
	sync.RWMutex
	txCount uint64                                // count
	txnList map[common.Uint256]*types.Transaction // transaction which have been verifyed will put into this map
}

func New(cfg *Config) *TxPool {
	p := TxPool{
		chainParams:     cfg.ChainParams,
		validator:       cfg.Validator,
		feeHelper:       cfg.FeeHelper,
		conflictManager: newConflictManager(cfg.Chain),
		txCount:         0,
		txnList:         make(map[common.Uint256]*types.Transaction),
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

	// add data to conflict Slot
	if errCode := p.AppendTx(tx); errCode != nil {
		log.Warn("[TxPool verifyTransactionWithTxnPool] failed", tx.Hash())
		return errCode
	}

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
	p.cleanMainChainTx(block.Transactions)
	return p.cleanTransactionList(block.Transactions)
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
	return p.VerifyTx(txn)
}

func (mp *TxPool) doAddTransaction(tx *types.Transaction) error {
	mp.txnList[tx.Hash()] = tx
	return nil
}

func (mp *TxPool) doRemoveTransaction(tx *types.Transaction) {
	hash := tx.Hash()
	if _, exist := mp.txnList[hash]; exist {
		delete(mp.txnList, hash)
		mp.removeTx(tx)
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
				p.doRemoveTransaction(txn)

				cleaned++
			}
		}

		if err := p.removeTx(txn); err != nil {
			log.Warnf("remove tx %s when delete", txn.Hash())
		}
	}

	log.Debugf("[cleanTransactionList] %d cleaned,  Remains %d in TxPool",
		cleaned, p.getTransactionCount())
	return nil
}

// clean the mainchain tx pool
func (p *TxPool) cleanMainChainTx(txs []*types.Transaction) {
	mainChainHashes := make(map[common.Uint256]struct{}, 0)
	for _, txn := range txs {
		if txn.IsRechargeToSideChainTx() {
			rechargePayload := txn.Payload.(*types.PayloadRechargeToSideChain)
			mainTxHash, err := rechargePayload.GetMainchainTxHash(txn.PayloadVersion)
			if err != nil {
				log.Error("get hash failed when clean mainchain tx:", txn.Hash())
				continue
			}
			mainChainHashes[*mainTxHash] = struct{}{}
		}
	}
	if len(mainChainHashes) == 0 {
		return
	}
	for _, tx := range p.txnList {
		if tx.IsRechargeToSideChainTx() {
			rechargePayload := tx.Payload.(*types.PayloadRechargeToSideChain)
			mainTxHash, err := rechargePayload.GetMainchainTxHash(tx.PayloadVersion)
			if err != nil {
				log.Error("get hash failed when clean main chain tx:", tx.Hash())
				continue
			}
			if _, ok := mainChainHashes[*mainTxHash]; ok {
				p.doRemoveTransaction(tx)
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

func (mp *TxPool) getInputUTXOList(input *types.Input) *types.Transaction {
	return mp.GetTx(input.ReferKey(), SlotTxInputsReferKeys)
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
			p.doRemoveTransaction(txn)
		}
	}
}

func (mp *TxPool) IsDuplicateMainChainTx(mainchainTxHash common.Uint256) bool {
	mp.RLock()
	defer mp.RUnlock()
	return mp.ContainsKey(mainchainTxHash, SlotRechargeToSidechainTxHash)
}
