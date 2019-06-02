package mempool

import (
	"bytes"
	"errors"
	"fmt"
	"sync"

	"github.com/elastos/Elastos.ELA/blockchain"
	. "github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	. "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/elanet/pact"
	. "github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/events"
)

type TxPool struct {
	chainParams *config.Params

	sync.RWMutex
	txnList         map[Uint256]*Transaction // transaction which have been verifyed will put into this map
	inputUTXOList   map[string]*Transaction  // transaction which pass the verify will add the UTXO to this map
	sidechainTxList map[Uint256]*Transaction // sidechain tx pool
	ownerPublicKeys map[string]struct{}
	nodePublicKeys  map[string]struct{}
	specialTxList   map[Uint256]struct{} // specialTxList holds the payload hashes of all illegal transactions and inactive arbitrators transactions
	txnListSize     int
}

//append transaction to txnpool when check ok.
//1.check  2.check with ledger(db) 3.check with pool
func (mp *TxPool) AppendToTxPool(tx *Transaction) error {
	mp.Lock()
	defer mp.Unlock()
	code := mp.appendToTxPool(tx)
	if code != Success {
		return code
	}

	go events.Notify(events.ETTransactionAccepted, tx)
	return nil
}

func (mp *TxPool) appendToTxPool(tx *Transaction) ErrCode {
	txHash := tx.Hash()

	// Don't accept the transaction if it already exists in the pool.  This
	// applies to orphan transactions as well.  This check is intended to
	// be a quick check to weed out duplicates.
	if _, ok := mp.txnList[txHash]; ok {
		return ErrTransactionDuplicate
	}

	if tx.IsCoinBaseTx() {
		log.Warnf("coinbase tx %s cannot be added into transaction pool", tx.Hash())
		return ErrIneffectiveCoinbase
	}

	chain := blockchain.DefaultLedger.Blockchain
	bestHeight := blockchain.DefaultLedger.Blockchain.GetHeight()
	if errCode := chain.CheckTransactionSanity(bestHeight+1, tx); errCode != Success {
		log.Warn("[TxPool CheckTransactionSanity] failed", tx.Hash())
		return errCode
	}
	if errCode := chain.CheckTransactionContext(bestHeight+1, tx); errCode != Success {
		log.Warn("[TxPool CheckTransactionContext] failed", tx.Hash())
		return errCode
	}
	//verify transaction by pool with lock
	if errCode := mp.verifyTransactionWithTxnPool(tx); errCode != Success {
		log.Warn("[TxPool verifyTransactionWithTxnPool] failed", tx.Hash())
		return errCode
	}

	size := tx.GetSize()
	if mp.txnListSize+size > pact.MaxTxPoolSize {
		log.Warn("TxPool check transactions size failed", tx.Hash())
		return ErrTransactionPoolSize
	}
	// Add the transaction to mem pool
	mp.txnList[txHash] = tx
	mp.txnListSize += size

	return Success
}

// HaveTransaction returns if a transaction is in transaction pool by the given
// transaction id. If no transaction match the transaction id, return false
func (mp *TxPool) HaveTransaction(txId Uint256) bool {
	mp.RLock()
	_, ok := mp.txnList[txId]
	mp.RUnlock()
	return ok
}

// GetTxsInPool returns a slice of all transactions in the mp.
//
// This function is safe for concurrent access.
func (mp *TxPool) GetTxsInPool() []*Transaction {
	mp.RLock()
	txs := make([]*Transaction, 0, len(mp.txnList))
	for _, tx := range mp.txnList {
		txs = append(txs, tx)
	}
	mp.RUnlock()
	return txs
}

//clean the trasaction Pool with committed block.
func (mp *TxPool) CleanSubmittedTransactions(block *Block) {
	mp.Lock()
	mp.cleanTransactions(block.Transactions)
	mp.cleanSidechainTx(block.Transactions)
	mp.cleanSideChainPowTx()
	mp.cleanCanceledProducer(block.Transactions)
	mp.Unlock()
}

func (mp *TxPool) cleanTransactions(blockTxs []*Transaction) {
	txsInPool := len(mp.txnList)
	deleteCount := 0
	for _, blockTx := range blockTxs {
		if blockTx.TxType == CoinBase {
			continue
		}

		if blockTx.IsIllegalTypeTx() || blockTx.IsInactiveArbitrators() {
			illegalData, ok := blockTx.Payload.(payload.DPOSIllegalData)
			if !ok {
				log.Error("cancel producer payload cast failed, tx:", blockTx.Hash())
				continue
			}
			hash := illegalData.Hash()
			if _, ok := mp.txnList[blockTx.Hash()]; ok {
				mp.doRemoveTransaction(blockTx.Hash(), blockTx.GetSize())
				deleteCount++
			}
			mp.delSpecialTx(&hash)
			continue
		} else if blockTx.IsNewSideChainPowTx() || blockTx.IsUpdateVersion() {
			if _, ok := mp.txnList[blockTx.Hash()]; ok {
				mp.doRemoveTransaction(blockTx.Hash(), blockTx.GetSize())
				deleteCount++
			}
			continue
		} else if blockTx.IsActivateProducerTx() {
			apPayload, ok := blockTx.Payload.(*payload.ActivateProducer)
			if !ok {
				log.Error("activate producer payload cast failed, tx:",
					blockTx.Hash())
				continue
			}
			mp.delNodePublicKey(BytesToHexString(apPayload.NodePublicKey))
			if _, ok := mp.txnList[blockTx.Hash()]; ok {
				mp.doRemoveTransaction(blockTx.Hash(), blockTx.GetSize())
				deleteCount++
			}
			continue
		}

		inputUtxos, err := blockchain.DefaultLedger.Store.GetTxReference(blockTx)
		if err != nil {
			log.Info(fmt.Sprintf("Transaction =%x not Exist in Pool when delete.", blockTx.Hash()), err)
			continue
		}
		for input := range inputUtxos {
			// we search transactions in transaction pool which have the same utxos with those transactions
			// in block. That is, if a transaction in the new-coming block uses the same utxo which a transaction
			// in transaction pool uses, then the latter one should be deleted, because one of its utxos has been used
			// by a confirmed transaction packed in the new-coming block.
			if tx := mp.getInputUTXOList(input); tx != nil {
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

				//1.remove from txnList
				mp.doRemoveTransaction(tx.Hash(), tx.GetSize())

				//2.remove from UTXO list map
				for _, input := range tx.Inputs {
					mp.delInputUTXOList(input)
				}

				switch tx.TxType {
				case WithdrawFromSideChain:
					payload, ok := tx.Payload.(*payload.WithdrawFromSideChain)
					if !ok {
						log.Error("type cast failed when clean sidechain tx:", tx.Hash())
						continue
					}
					for _, hash := range payload.SideChainTransactionHashes {
						mp.delSidechainTx(hash)
					}
				case RegisterProducer:
					rpPayload, ok := tx.Payload.(*payload.ProducerInfo)
					if !ok {
						log.Error("register producer payload cast failed, tx:", tx.Hash())
						continue
					}
					mp.delOwnerPublicKey(BytesToHexString(rpPayload.OwnerPublicKey))
					mp.delNodePublicKey(BytesToHexString(rpPayload.NodePublicKey))
				case UpdateProducer:
					upPayload, ok := tx.Payload.(*payload.ProducerInfo)
					if !ok {
						log.Error("update producer payload cast failed, tx:", tx.Hash())
						continue
					}
					mp.delOwnerPublicKey(BytesToHexString(upPayload.OwnerPublicKey))
					mp.delNodePublicKey(BytesToHexString(upPayload.NodePublicKey))
				case CancelProducer:
					cpPayload, ok := tx.Payload.(*payload.ProcessProducer)
					if !ok {
						log.Error("cancel producer payload cast failed, tx:", tx.Hash())
						continue
					}
					mp.delOwnerPublicKey(BytesToHexString(cpPayload.OwnerPublicKey))
				}

				deleteCount++
			}
		}
	}
	log.Debug(fmt.Sprintf("[cleanTransactionList],transaction %d in block, %d in transaction pool before, %d deleted,"+
		" Remains %d in TxPool",
		len(blockTxs), txsInPool, deleteCount, len(mp.txnList)))
}

func (mp *TxPool) cleanCanceledProducer(txs []*Transaction) error {
	for _, txn := range txs {
		if txn.TxType == CancelProducer {
			cpPayload, ok := txn.Payload.(*payload.ProcessProducer)
			if !ok {
				return errors.New("invalid cancel producer payload")
			}
			if err := mp.cleanVoteAndUpdateProducer(cpPayload.OwnerPublicKey); err != nil {
				log.Error(err)
			}
		}
	}

	return nil
}

func (mp *TxPool) cleanVoteAndUpdateProducer(ownerPublicKey []byte) error {
	for _, txn := range mp.txnList {
		if txn.TxType == TransferAsset {
			for _, output := range txn.Outputs {
				if output.Type == OTVote {
					opPayload, ok := output.Payload.(*outputpayload.VoteOutput)
					if !ok {
						return errors.New("invalid vote output payload")
					}
					for _, content := range opPayload.Contents {
						if content.VoteType == outputpayload.Delegate {
							for _, pubKey := range content.Candidates {
								if bytes.Equal(ownerPublicKey, pubKey) {
									mp.removeTransaction(txn)
								}
							}
						}
					}
				}
			}
		} else if txn.TxType == UpdateProducer {
			upPayload, ok := txn.Payload.(*payload.ProducerInfo)
			if !ok {
				return errors.New("invalid update producer payload")
			}
			if bytes.Equal(upPayload.OwnerPublicKey, ownerPublicKey) {
				mp.removeTransaction(txn)
				mp.delOwnerPublicKey(BytesToHexString(upPayload.OwnerPublicKey))
				mp.delNodePublicKey(BytesToHexString(upPayload.NodePublicKey))
			}
		}
	}

	return nil
}

//get the transaction by hash
func (mp *TxPool) GetTransaction(hash Uint256) *Transaction {
	mp.RLock()
	defer mp.RUnlock()
	return mp.txnList[hash]
}

//verify transaction with txnpool
func (mp *TxPool) verifyTransactionWithTxnPool(txn *Transaction) ErrCode {
	if txn.IsSideChainPowTx() {
		// check and replace the duplicate sidechainpow tx
		mp.replaceDuplicateSideChainPowTx(txn)
	} else if txn.IsWithdrawFromSideChainTx() {
		// check if the withdraw transaction includes duplicate sidechain tx in pool
		if err := mp.verifyDuplicateSidechainTx(txn); err != nil {
			log.Warn(err)
			return ErrSidechainTxDuplicate
		}
	}

	// check if the transaction includes double spent UTXO inputs
	if err := mp.verifyDoubleSpend(txn); err != nil {
		log.Warn(err)
		return ErrDoubleSpend
	}

	return mp.verifyProducerRelatedTx(txn)
}

//verify producer related transaction with txnpool
func (mp *TxPool) verifyProducerRelatedTx(txn *Transaction) ErrCode {
	switch txn.TxType {
	case RegisterProducer:
		payload, ok := txn.Payload.(*payload.ProducerInfo)
		if !ok {
			log.Error("register producer payload cast failed, tx:", txn.Hash())
		}
		if err := mp.verifyDuplicateOwner(BytesToHexString(payload.OwnerPublicKey)); err != nil {
			log.Warn(err)
			return ErrProducerProcessing
		}
		if err := mp.verifyDuplicateNode(BytesToHexString(payload.NodePublicKey)); err != nil {
			log.Warn(err)
			return ErrProducerNodeProcessing
		}
	case UpdateProducer:
		payload, ok := txn.Payload.(*payload.ProducerInfo)
		if !ok {
			log.Error("update producer payload cast failed, tx:", txn.Hash())
		}
		if err := mp.verifyDuplicateOwner(BytesToHexString(payload.OwnerPublicKey)); err != nil {
			log.Warn(err)
			return ErrProducerProcessing
		}
		if err := mp.verifyDuplicateNode(BytesToHexString(payload.NodePublicKey)); err != nil {
			log.Warn(err)
			return ErrProducerNodeProcessing
		}
	case CancelProducer:
		payload, ok := txn.Payload.(*payload.ProcessProducer)
		if !ok {
			log.Error("cancel producer payload cast failed, tx:", txn.Hash())
		}
		if err := mp.verifyDuplicateOwner(BytesToHexString(payload.OwnerPublicKey)); err != nil {
			log.Warn(err)
			return ErrProducerProcessing
		}
	case ActivateProducer:
		payload, ok := txn.Payload.(*payload.ActivateProducer)
		if !ok {
			log.Error("activate producer payload cast failed, tx:", txn.Hash())
		}
		if err := mp.verifyDuplicateNode(BytesToHexString(payload.NodePublicKey)); err != nil {
			log.Warn(err)
			return ErrProducerNodeProcessing
		}
	case IllegalProposalEvidence, IllegalVoteEvidence, IllegalBlockEvidence,
		IllegalSidechainEvidence, InactiveArbitrators:
		illegalData, ok := txn.Payload.(payload.DPOSIllegalData)
		if !ok {
			log.Error("special tx payload cast failed, tx:", txn.Hash())
		}
		hash := illegalData.Hash()
		if err := mp.verifyDuplicateSpecialTx(&hash); err != nil {
			log.Warn(err)
			return ErrProducerProcessing
		}
	}

	return Success
}

//remove from associated map
func (mp *TxPool) removeTransaction(tx *Transaction) {
	//1.remove from txnList
	if _, ok := mp.txnList[tx.Hash()]; ok {
		mp.doRemoveTransaction(tx.Hash(), tx.GetSize())
	}

	//2.remove from UTXO list map
	result, err := blockchain.DefaultLedger.Store.GetTxReference(tx)
	if err != nil {
		log.Info(fmt.Sprintf("Transaction =%x not Exist in Pool when delete.", tx.Hash()))
		return
	}
	for UTXOTxInput := range result {
		mp.delInputUTXOList(UTXOTxInput)
	}
}

//check and add to utxo list pool
func (mp *TxPool) verifyDoubleSpend(txn *Transaction) error {
	reference, err := blockchain.DefaultLedger.Store.GetTxReference(txn)
	if err != nil {
		return err
	}
	inputs := []*Input{}
	for k := range reference {
		if txn := mp.getInputUTXOList(k); txn != nil {
			return errors.New(fmt.Sprintf("double spent UTXO inputs detected, "+
				"transaction hash: %x, input: %s, index: %d",
				txn.Hash(), k.Previous.TxID, k.Previous.Index))
		}
		inputs = append(inputs, k)
	}
	for _, v := range inputs {
		mp.addInputUTXOList(txn, v)
	}

	return nil
}

func (mp *TxPool) IsDuplicateSidechainTx(sidechainTxHash Uint256) bool {
	mp.RLock()
	_, ok := mp.sidechainTxList[sidechainTxHash]
	mp.RUnlock()
	return ok
}

//check and add to sidechain tx pool
func (mp *TxPool) verifyDuplicateSidechainTx(txn *Transaction) error {
	withPayload, ok := txn.Payload.(*payload.WithdrawFromSideChain)
	if !ok {
		return errors.New("convert the payload of withdraw tx failed")
	}

	for _, hash := range withPayload.SideChainTransactionHashes {
		_, ok := mp.sidechainTxList[hash]
		if ok {
			return errors.New("duplicate sidechain tx detected")
		}
	}
	mp.addSidechainTx(txn)

	return nil
}

func (mp *TxPool) verifyDuplicateOwner(ownerPublicKey string) error {
	_, ok := mp.ownerPublicKeys[ownerPublicKey]
	if ok {
		return errors.New("this producer in being processed")
	}
	mp.addOwnerPublicKey(ownerPublicKey)

	return nil
}

func (mp *TxPool) addOwnerPublicKey(publicKey string) {
	mp.ownerPublicKeys[publicKey] = struct{}{}
}

func (mp *TxPool) delOwnerPublicKey(publicKey string) {
	delete(mp.ownerPublicKeys, publicKey)
}

func (mp *TxPool) verifyDuplicateNode(nodePublicKey string) error {
	_, ok := mp.nodePublicKeys[nodePublicKey]
	if ok {
		return errors.New("this producer node in being processed")
	}
	mp.addNodePublicKey(nodePublicKey)

	return nil
}

func (mp *TxPool) addNodePublicKey(nodePublicKey string) {
	mp.nodePublicKeys[nodePublicKey] = struct{}{}
}

func (mp *TxPool) delNodePublicKey(nodePublicKey string) {
	delete(mp.nodePublicKeys, nodePublicKey)
}

func (mp *TxPool) addSpecialTx(hash *Uint256) {
	mp.specialTxList[*hash] = struct{}{}
}

func (mp *TxPool) delSpecialTx(hash *Uint256) {
	delete(mp.specialTxList, *hash)
}

func (mp *TxPool) verifyDuplicateSpecialTx(hash *Uint256) error {
	if _, ok := mp.specialTxList[*hash]; ok {
		return errors.New("this special tx has being processed")
	}
	mp.addSpecialTx(hash)

	return nil
}

// check and replace the duplicate sidechainpow tx
func (mp *TxPool) replaceDuplicateSideChainPowTx(txn *Transaction) {
	var replaceList []*Transaction

	for _, v := range mp.txnList {
		if v.TxType == SideChainPow {
			oldPayload := v.Payload.Data(payload.SideChainPowVersion)
			oldGenesisHashData := oldPayload[32:64]

			newPayload := txn.Payload.Data(payload.SideChainPowVersion)
			newGenesisHashData := newPayload[32:64]

			if bytes.Equal(oldGenesisHashData, newGenesisHashData) {
				replaceList = append(replaceList, v)
			}
		}
	}

	for _, txn := range replaceList {
		txid := txn.Hash()
		log.Info("replace sidechainpow transaction, txid=", txid.String())
		mp.removeTransaction(txn)
	}
}

// clean the sidechain tx pool
func (mp *TxPool) cleanSidechainTx(txs []*Transaction) {
	for _, txn := range txs {
		if txn.IsWithdrawFromSideChainTx() {
			withPayload := txn.Payload.(*payload.WithdrawFromSideChain)
			for _, hash := range withPayload.SideChainTransactionHashes {
				tx, ok := mp.sidechainTxList[hash]
				if ok {
					// delete tx
					if _, ok := mp.txnList[tx.Hash()]; ok {
						mp.doRemoveTransaction(tx.Hash(), tx.GetSize())
					}
					//delete utxo map
					for _, input := range tx.Inputs {
						mp.delInputUTXOList(input)
					}
					//delete sidechain tx map
					payload, ok := tx.Payload.(*payload.WithdrawFromSideChain)
					if !ok {
						log.Error("type cast failed when clean sidechain tx:", tx.Hash())
					}
					for _, hash := range payload.SideChainTransactionHashes {
						mp.delSidechainTx(hash)
					}
				}
			}
		}
	}
}

// clean the sidechainpow tx pool
func (mp *TxPool) cleanSideChainPowTx() {
	for hash, txn := range mp.txnList {
		if txn.IsSideChainPowTx() {
			arbiter := blockchain.DefaultLedger.Arbitrators.GetOnDutyCrossChainArbitrator()
			if err := blockchain.CheckSideChainPowConsensus(txn, arbiter); err != nil {
				// delete tx
				mp.doRemoveTransaction(hash, txn.GetSize())

				//delete utxo map
				for _, input := range txn.Inputs {
					delete(mp.inputUTXOList, input.ReferKey())
				}
			}
		}
	}
}

func (mp *TxPool) addToTxList(tx *Transaction) bool {
	txHash := tx.Hash()
	if _, ok := mp.txnList[txHash]; ok {
		return false
	}

	return true
}

func (mp *TxPool) GetTransactionCount() int {
	mp.RLock()
	defer mp.RUnlock()
	return len(mp.txnList)
}

func (mp *TxPool) getInputUTXOList(input *Input) *Transaction {
	return mp.inputUTXOList[input.ReferKey()]
}

func (mp *TxPool) addInputUTXOList(tx *Transaction, input *Input) {
	id := input.ReferKey()
	mp.inputUTXOList[id] = tx
}

func (mp *TxPool) delInputUTXOList(input *Input) {
	id := input.ReferKey()
	delete(mp.inputUTXOList, id)
}

func (mp *TxPool) addSidechainTx(txn *Transaction) {
	witPayload := txn.Payload.(*payload.WithdrawFromSideChain)
	for _, hash := range witPayload.SideChainTransactionHashes {
		mp.sidechainTxList[hash] = txn
	}
}

func (mp *TxPool) delSidechainTx(hash Uint256) {
	delete(mp.sidechainTxList, hash)
}

func (mp *TxPool) MaybeAcceptTransaction(tx *Transaction) error {
	mp.Lock()
	code := mp.appendToTxPool(tx)
	mp.Unlock()
	if code != Success {
		return code
	}
	return nil
}

func (mp *TxPool) RemoveTransaction(txn *Transaction) {
	mp.Lock()
	txHash := txn.Hash()
	for i := range txn.Outputs {
		input := Input{
			Previous: OutPoint{
				TxID:  txHash,
				Index: uint16(i),
			},
		}

		txn := mp.getInputUTXOList(&input)
		if txn != nil {
			mp.removeTransaction(txn)
		}
	}
	mp.Unlock()
}

func (mp *TxPool) doRemoveTransaction(hash Uint256, txSize int) {
	delete(mp.txnList, hash)
	mp.txnListSize -= txSize
}

func NewTxPool(params *config.Params) *TxPool {
	return &TxPool{
		chainParams:     params,
		inputUTXOList:   make(map[string]*Transaction),
		txnList:         make(map[Uint256]*Transaction),
		sidechainTxList: make(map[Uint256]*Transaction),
		ownerPublicKeys: make(map[string]struct{}),
		nodePublicKeys:  make(map[string]struct{}),
		specialTxList:   make(map[Uint256]struct{}),
	}
}
