// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package blockchain

import (
	"bytes"
	"errors"
	"path/filepath"
	"sync"
	"sync/atomic"
	"time"

	. "github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	. "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	_ "github.com/elastos/Elastos.ELA/database/ffldb"
)

type ProducerState byte

type ProducerInfo struct {
	Payload   *payload.ProducerInfo
	RegHeight uint32
	Vote      Fixed64
}

type ChainStore struct {
	levelDB            IStore
	fflDB              IFFLDBChainStore
	currentBlockHeight uint32
	persistMutex       sync.Mutex
}

func NewChainStore(dataDir string, params *config.Params) (IChainStore, error) {
	db, err := NewLevelDB(filepath.Join(dataDir, "chain"))
	if err != nil {
		return nil, err
	}
	fflDB, err := NewChainStoreFFLDB(dataDir, params)
	if err != nil {
		return nil, err
	}
	s := &ChainStore{
		levelDB: db,
		fflDB:   fflDB,
	}

	return s, nil
}

func (c *ChainStore) CloseLeveldb() {
	c.levelDB.Close()
}

func (c *ChainStore) Close() {
	c.persistMutex.Lock()
	defer c.persistMutex.Unlock()
	if err := c.fflDB.Close(); err != nil {
		log.Error("fflDB close failed:", err)
	}
}

func (c *ChainStore) IsTxHashDuplicate(txID Uint256) bool {
	txn, _, err := c.fflDB.GetTransaction(txID)
	if err != nil || txn == nil {
		return false
	}

	return true
}

func (c *ChainStore) IsSidechainTxHashDuplicate(sidechainTxHash Uint256) bool {
	return c.GetFFLDB().IsTx3Exist(&sidechainTxHash)
}

func (c *ChainStore) IsDoubleSpend(txn *Transaction) bool {
	if len(txn.Inputs) == 0 {
		return false
	}
	for i := 0; i < len(txn.Inputs); i++ {
		txID := txn.Inputs[i].Previous.TxID
		unspents, err := c.GetFFLDB().GetUnspent(txID)
		if err != nil {
			return true
		}
		findFlag := false
		for k := 0; k < len(unspents); k++ {
			if unspents[k] == txn.Inputs[i].Previous.Index {
				findFlag = true
				break
			}
		}
		if !findFlag {
			return true
		}
	}

	return false
}

func (c *ChainStore) RollbackBlock(b *Block, node *BlockNode,
	confirm *payload.Confirm, medianTimePast time.Time) error {
	now := time.Now()
	err := c.handleRollbackBlockTask(b, node, confirm, medianTimePast)
	tcall := float64(time.Now().Sub(now)) / float64(time.Second)
	log.Debugf("handle block rollback exetime: %g", tcall)
	return err
}

func (c *ChainStore) GetTransaction(txID Uint256) (*Transaction, uint32, error) {
	return c.fflDB.GetTransaction(txID)
}

func (c *ChainStore) GetTxReference(tx *Transaction) (map[*Input]*Output, error) {
	if tx.TxType == RegisterAsset {
		return nil, nil
	}
	txOutputsCache := make(map[Uint256][]*Output)
	//UTXO input /  Outputs
	reference := make(map[*Input]*Output)
	// Key index，v UTXOInput
	for _, input := range tx.Inputs {
		txID := input.Previous.TxID
		index := input.Previous.Index
		if outputs, ok := txOutputsCache[txID]; ok {
			reference[input] = outputs[index]
		} else {
			transaction, _, err := c.GetTransaction(txID)

			if err != nil {
				return nil, errors.New("GetTxReference failed, previous transaction not found")
			}
			if int(index) >= len(transaction.Outputs) {
				return nil, errors.New("GetTxReference failed, refIdx out of range.")
			}
			reference[input] = transaction.Outputs[index]
			txOutputsCache[txID] = transaction.Outputs
		}
	}
	return reference, nil
}

func (c *ChainStore) rollback(b *Block, node *BlockNode,
	confirm *payload.Confirm, medianTimePast time.Time) error {
	if err := c.fflDB.RollbackBlock(b, node, confirm, medianTimePast); err != nil {
		return err
	}
	atomic.StoreUint32(&c.currentBlockHeight, b.Height-1)

	return nil
}

func (c *ChainStore) persist(b *Block, node *BlockNode,
	confirm *payload.Confirm, medianTimePast time.Time) error {
	c.persistMutex.Lock()
	defer c.persistMutex.Unlock()

	if err := c.fflDB.SaveBlock(b, node, confirm, medianTimePast); err != nil {
		return err
	}
	return nil
}

func (c *ChainStore) GetFFLDB() IFFLDBChainStore {
	return c.fflDB
}

func (c *ChainStore) SaveBlock(b *Block, node *BlockNode,
	confirm *payload.Confirm, medianTimePast time.Time) error {
	log.Debug("SaveBlock()")

	now := time.Now()
	err := c.handlePersistBlockTask(b, node, confirm, medianTimePast)

	tcall := float64(time.Now().Sub(now)) / float64(time.Second)
	log.Debugf("handle block exetime: %g num transactions:%d",
		tcall, len(b.Transactions))
	return err
}

func (c *ChainStore) handleRollbackBlockTask(b *Block, node *BlockNode,
	confirm *payload.Confirm, medianTimePast time.Time) error {
	_, err := c.fflDB.GetBlock(b.Hash())
	if err != nil {
		log.Errorf("block %x can't be found", BytesToHexString(b.Hash().Bytes()))
		return err
	}
	return c.rollback(b, node, confirm, medianTimePast)
}

func (c *ChainStore) handlePersistBlockTask(b *Block, node *BlockNode,
	confirm *payload.Confirm, medianTimePast time.Time) error {
	if b.Header.Height <= c.currentBlockHeight {
		return errors.New("block height less than current block height")
	}

	return c.persistBlock(b, node, confirm, medianTimePast)
}

func (c *ChainStore) persistBlock(b *Block, node *BlockNode,
	confirm *payload.Confirm, medianTimePast time.Time) error {
	err := c.persist(b, node, confirm, medianTimePast)
	if err != nil {
		log.Fatal("[persistBlocks]: error to persist block:", err.Error())
		return err
	}

	atomic.StoreUint32(&c.currentBlockHeight, b.Height)
	return nil
}

func (c *ChainStore) GetConfirm(hash Uint256) (*payload.Confirm, error) {
	var confirm = new(payload.Confirm)
	prefix := []byte{byte(DATAConfirm)}
	confirmBytes, err := c.levelDB.Get(append(prefix, hash.Bytes()...))
	if err != nil {
		return nil, err
	}

	if err = confirm.Deserialize(bytes.NewReader(confirmBytes)); err != nil {
		return nil, err
	}

	return confirm, nil
}

func (c *ChainStore) GetHeight() uint32 {
	return atomic.LoadUint32(&c.currentBlockHeight)
}

func (c *ChainStore) SetHeight(height uint32) {
	atomic.StoreUint32(&c.currentBlockHeight, height)
}
