// Copyright (c) 2017-2019 Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package mempool

import (
	"bytes"
	"encoding/hex"
	"errors"
	"fmt"
	"sync"

	"github.com/elastos/Elastos.ELA/blockchain"
	. "github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/contract"
	. "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/elanet/pact"
	. "github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/events"
	"github.com/elastos/Elastos.ELA/vm"
)

type TxPool struct {
	chainParams *config.Params

	sync.RWMutex
	txnList         map[Uint256]*Transaction // transaction which have been verifyed will put into this map
	inputUTXOList   map[string]*Transaction  // transaction which pass the verify will add the UTXO to this map
	sidechainTxList map[Uint256]*Transaction // sidechain tx pool
	ownerPublicKeys map[string]struct{}
	nodePublicKeys  map[string]struct{}
	crDIDs          map[Uint168]struct{}
	specialTxList   map[Uint256]struct{} // specialTxList holds the payload hashes of all illegal transactions and inactive arbitrators transactions

	tempInputUTXOList   map[string]*Transaction
	tempSidechainTxList map[Uint256]*Transaction
	tempOwnerPublicKeys map[string]struct{}
	tempNodePublicKeys  map[string]struct{}
	tempCrDIDs          map[Uint168]struct{}
	tempSpecialTxList   map[Uint256]struct{}

	txnListSize int
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
	references, err := chain.UTXOCache.GetTxReference(tx)
	if err != nil {
		log.Warn("[CheckTransactionContext] get transaction reference failed")
		return ErrUnknownReferredTx
	}
	if errCode := chain.CheckTransactionContext(bestHeight+1, tx, references); errCode != Success {
		log.Warn("[TxPool CheckTransactionContext] failed", tx.Hash())
		return errCode
	}
	//verify transaction by pool with lock
	if errCode := mp.verifyTransactionWithTxnPool(tx); errCode != Success {
		mp.clearTemp()
		log.Warn("[TxPool verifyTransactionWithTxnPool] failed", tx.Hash())
		return errCode
	}

	size := tx.GetSize()
	if mp.txnListSize+size > pact.MaxTxPoolSize {
		log.Warn("TxPool check transactions size failed", tx.Hash())
		return ErrTransactionPoolSize
	}

	mp.commitTemp()
	mp.clearTemp()

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

		inputUtxos, err := blockchain.DefaultLedger.Blockchain.UTXOCache.GetTxReference(blockTx)
		if err != nil {
			log.Infof("Transaction=%s not exist when deleting, %s.",
				blockTx.Hash(), err)
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
						" Delete transaction in the transaction pool. Transaction id: %s", tx.Hash())
				} else {
					log.Debugf("double spent UTXO inputs detected in transaction pool when adding a new block. "+
						"Delete transaction in the transaction pool. "+
						"block transaction hash: %s, transaction hash: %s, the same input: %s, index: %d",
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
				case RegisterCR:
					rcPayload, ok := tx.Payload.(*payload.CRInfo)
					if !ok {
						log.Error("register CR payload cast failed, tx:", tx.Hash())
						continue
					}
					mp.delCRDID(rcPayload.DID)
					mp.delPublicKeyByCode(rcPayload.Code)
				case UpdateCR:
					rcPayload, ok := tx.Payload.(*payload.CRInfo)
					if !ok {
						log.Error("update CR payload cast failed, tx:", tx.Hash())
						continue
					}
					mp.delCRDID(rcPayload.DID)
				case UnregisterCR:
					unrcPayload, ok := tx.Payload.(*payload.UnregisterCR)
					if !ok {
						log.Error("unregisterCR CR payload cast failed, tx:", tx.Hash())
						continue
					}
					ct, err := contract.CreateCRDIDContractByCode(unrcPayload.Code)
					if err != nil {
						log.Error("invalid unregister CR code, tx:", tx.Hash())
						continue
					}
					did := ct.ToProgramHash()
					mp.delCRDID(*did)
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
							for _, cv := range content.CandidateVotes {
								if bytes.Equal(ownerPublicKey, cv.Candidate) {
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

	if errCode := mp.verifyProducerRelatedTx(txn); errCode != Success {
		return errCode
	}

	return mp.verifyCRRelatedTx(txn)
}

//verify producer related transaction with txnpool
func (mp *TxPool) verifyProducerRelatedTx(txn *Transaction) ErrCode {
	switch txn.TxType {
	case RegisterProducer:
		p, ok := txn.Payload.(*payload.ProducerInfo)
		if !ok {
			log.Error("register producer payload cast failed, tx:", txn.Hash())
			return ErrProducerProcessing
		}
		if err := mp.verifyDuplicateOwnerAndNode(BytesToHexString(p.OwnerPublicKey),
			BytesToHexString(p.NodePublicKey)); err != nil {
			log.Warn(err)
			return ErrProducerProcessing
		}
	case UpdateProducer:
		p, ok := txn.Payload.(*payload.ProducerInfo)
		if !ok {
			log.Error("update producer payload cast failed, tx:", txn.Hash())
			return ErrProducerProcessing
		}
		if err := mp.verifyDuplicateOwnerAndNode(BytesToHexString(p.OwnerPublicKey),
			BytesToHexString(p.NodePublicKey)); err != nil {
			log.Warn(err)
			return ErrProducerProcessing
		}
	case CancelProducer:
		p, ok := txn.Payload.(*payload.ProcessProducer)
		if !ok {
			log.Error("cancel producer payload cast failed, tx:", txn.Hash())
			return ErrProducerProcessing
		}
		if err := mp.verifyDuplicateOwner(BytesToHexString(p.OwnerPublicKey)); err != nil {
			log.Warn(err)
			return ErrProducerProcessing
		}
	case ActivateProducer:
		p, ok := txn.Payload.(*payload.ActivateProducer)
		if !ok {
			log.Error("activate producer payload cast failed, tx:", txn.Hash())
			return ErrProducerProcessing
		}
		if err := mp.verifyDuplicateNode(BytesToHexString(p.NodePublicKey)); err != nil {
			log.Warn(err)
			return ErrProducerNodeProcessing
		}
	case IllegalProposalEvidence, IllegalVoteEvidence, IllegalBlockEvidence,
		IllegalSidechainEvidence, InactiveArbitrators:
		illegalData, ok := txn.Payload.(payload.DPOSIllegalData)
		if !ok {
			log.Error("special tx payload cast failed, tx:", txn.Hash())
			return ErrProducerProcessing
		}
		hash := illegalData.Hash()
		if err := mp.verifyDuplicateSpecialTx(&hash); err != nil {
			log.Warn(err)
			return ErrProducerProcessing
		}
	}

	return Success
}

//verify CR related transaction with txnpool
func (mp *TxPool) verifyCRRelatedTx(txn *Transaction) ErrCode {
	switch txn.TxType {
	case RegisterCR:
		p, ok := txn.Payload.(*payload.CRInfo)
		if !ok {
			log.Error("register CR payload cast failed, tx:", txn.Hash())
			return ErrCRProcessing
		}
		if err := mp.verifyDuplicateCRAndProducer(p.DID, p.Code); err != nil {
			log.Warn(err)
			return ErrCRProcessing
		}
	case UpdateCR:
		p, ok := txn.Payload.(*payload.CRInfo)
		if !ok {
			log.Error("update producer payload cast failed, tx:", txn.Hash())
			return ErrCRProcessing
		}
		if err := mp.verifyDuplicateCR(p.DID); err != nil {
			log.Warn(err)
			return ErrCRProcessing
		}
	case UnregisterCR:
		p, ok := txn.Payload.(*payload.UnregisterCR)
		if !ok {
			log.Error("update producer payload cast failed, tx:", txn.Hash())
			return ErrCRProcessing
		}
		ct, err := contract.CreateCRDIDContractByCode(p.Code)
		if err != nil {
			log.Error("invalid unregister CR code, tx:", txn.Hash())
			return ErrCRProcessing
		}
		did := ct.ToProgramHash()
		if err := mp.verifyDuplicateCR(*did); err != nil {
			log.Warn(err)
			return ErrCRProcessing
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
	reference, err := blockchain.DefaultLedger.Blockchain.UTXOCache.GetTxReference(tx)
	if err != nil {
		log.Infof("Transaction=%s not exist when deleting, %s",
			tx.Hash(), err)
		return
	}
	for UTXOTxInput := range reference {
		mp.delInputUTXOList(UTXOTxInput)
	}
}

//check and add to utxo list pool
func (mp *TxPool) verifyDoubleSpend(txn *Transaction) error {
	reference, err := blockchain.DefaultLedger.Blockchain.UTXOCache.GetTxReference(txn)
	if err != nil {
		return err
	}
	inputs := make([]*Input, 0)
	for k := range reference {
		if txn := mp.getInputUTXOList(k); txn != nil {
			return fmt.Errorf("double spent UTXO inputs detected, "+
				"transaction hash: %s, input: %s, index: %d",
				txn.Hash(), k.Previous.TxID, k.Previous.Index)
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

func (mp *TxPool) verifyDuplicateOwnerAndNode(ownerPublicKey string, nodePublicKey string) error {
	_, ok := mp.ownerPublicKeys[ownerPublicKey]
	if ok {
		return errors.New("this producer in being processed")
	}
	_, ok = mp.nodePublicKeys[nodePublicKey]
	if ok {
		return errors.New("this producer node in being processed")
	}
	mp.addOwnerPublicKey(ownerPublicKey)
	mp.addNodePublicKey(nodePublicKey)

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
	mp.tempOwnerPublicKeys[publicKey] = struct{}{}
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
	mp.tempNodePublicKeys[nodePublicKey] = struct{}{}
}

func (mp *TxPool) delNodePublicKey(nodePublicKey string) {
	delete(mp.nodePublicKeys, nodePublicKey)
}

func (mp *TxPool) verifyDuplicateCR(did Uint168) error {
	_, ok := mp.crDIDs[did]
	if ok {
		return errors.New("this CR in being processed")
	}
	mp.addCRDID(did)

	return nil
}

func (mp *TxPool) verifyDuplicateCRAndProducer(did Uint168, code []byte) error {
	_, ok := mp.crDIDs[did]
	if ok {
		return errors.New("this CR in being processed")
	}
	signType, err := crypto.GetScriptType(code)
	if err != nil {
		return err
	}

	if signType == vm.CHECKSIG {
		pk := hex.EncodeToString(code[1 : len(code)-1])
		if _, ok := mp.ownerPublicKeys[pk]; ok {
			return errors.New("this public key in being" +
				" processed by producer owner public key")
		}

		if _, ok := mp.nodePublicKeys[pk]; ok {
			return errors.New("this public key in being" +
				" processed by producer node public key")
		}
		mp.addOwnerPublicKey(pk)
		mp.addNodePublicKey(pk)
	}

	mp.addCRDID(did)
	return nil
}

func (mp *TxPool) addCRDID(did Uint168) {
	mp.tempCrDIDs[did] = struct{}{}
}

func (mp *TxPool) delCRDID(did Uint168) {
	delete(mp.crDIDs, did)
}

func (mp *TxPool) delPublicKeyByCode(code []byte) {
	signType, err := crypto.GetScriptType(code)
	if err != nil {
		return
	}
	if signType == vm.CHECKSIG {
		pk := hex.EncodeToString(code[1 : len(code)-1])
		delete(mp.ownerPublicKeys, pk)
		delete(mp.nodePublicKeys, pk)
	}
}

func (mp *TxPool) addSpecialTx(hash *Uint256) {
	mp.tempSpecialTxList[*hash] = struct{}{}
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
	mp.tempInputUTXOList[id] = tx
}

func (mp *TxPool) delInputUTXOList(input *Input) {
	id := input.ReferKey()
	delete(mp.inputUTXOList, id)
}

func (mp *TxPool) addSidechainTx(txn *Transaction) {
	witPayload := txn.Payload.(*payload.WithdrawFromSideChain)
	for _, hash := range witPayload.SideChainTransactionHashes {
		mp.tempSidechainTxList[hash] = txn
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

func (mp *TxPool) clearTemp() {
	mp.tempInputUTXOList = make(map[string]*Transaction)
	mp.tempSidechainTxList = make(map[Uint256]*Transaction)
	mp.tempOwnerPublicKeys = make(map[string]struct{})
	mp.tempNodePublicKeys = make(map[string]struct{})
	mp.tempSpecialTxList = make(map[Uint256]struct{})
	mp.tempCrDIDs = make(map[Uint168]struct{})
}

func (mp *TxPool) commitTemp() {
	for k, v := range mp.tempInputUTXOList {
		mp.inputUTXOList[k] = v
	}
	for k, v := range mp.tempSidechainTxList {
		mp.sidechainTxList[k] = v
	}
	for k, v := range mp.tempOwnerPublicKeys {
		mp.ownerPublicKeys[k] = v
	}
	for k, v := range mp.tempNodePublicKeys {
		mp.nodePublicKeys[k] = v
	}
	for k, v := range mp.tempCrDIDs {
		mp.crDIDs[k] = v
	}
	for k, v := range mp.tempSpecialTxList {
		mp.specialTxList[k] = v
	}
}

func NewTxPool(params *config.Params) *TxPool {
	return &TxPool{
		chainParams:         params,
		inputUTXOList:       make(map[string]*Transaction),
		txnList:             make(map[Uint256]*Transaction),
		sidechainTxList:     make(map[Uint256]*Transaction),
		ownerPublicKeys:     make(map[string]struct{}),
		nodePublicKeys:      make(map[string]struct{}),
		specialTxList:       make(map[Uint256]struct{}),
		crDIDs:              make(map[Uint168]struct{}),
		tempInputUTXOList:   make(map[string]*Transaction),
		tempSidechainTxList: make(map[Uint256]*Transaction),
		tempOwnerPublicKeys: make(map[string]struct{}),
		tempNodePublicKeys:  make(map[string]struct{}),
		tempSpecialTxList:   make(map[Uint256]struct{}),
		tempCrDIDs:          make(map[Uint168]struct{}),
	}
}
