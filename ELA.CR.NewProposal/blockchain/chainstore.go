// Copyright (c) 2017-2019 The Elastos Foundation
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
	"github.com/elastos/Elastos.ELA/common/log"
	. "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	_ "github.com/elastos/Elastos.ELA/database/ffldb"
)

const (
	BlocksCacheSize = 2
)

type ProducerState byte

type ProducerInfo struct {
	Payload   *payload.ProducerInfo
	RegHeight uint32
	Vote      Fixed64
}

type ChainStore struct {
	IStore

	fflDB IFFLDBChainStore

	currentBlockHeight uint32

	mtx              sync.RWMutex
	blockHashesCache []Uint256
	blocksCache      map[Uint256]*Block

	persistMutex sync.Mutex
}

func NewChainStore(dataDir string) (IChainStore, error) {
	db, err := NewLevelDB(filepath.Join(dataDir, "chain"))
	if err != nil {
		return nil, err
	}
	fflDB, err := NewChainStoreFFLDB(dataDir)
	if err != nil {
		return nil, err
	}
	s := &ChainStore{
		IStore:           db,
		fflDB:            fflDB,
		blockHashesCache: make([]Uint256, 0, BlocksCacheSize),
		blocksCache:      make(map[Uint256]*Block),
	}

	return s, nil
}

func (c *ChainStore) CloseLeveldb() {
	c.IStore.Close()
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

func (c *ChainStore) GetBlock(hash Uint256) (*Block, error) {
	c.mtx.RLock()
	if block, exist := c.blocksCache[hash]; exist {
		c.mtx.RUnlock()
		return block, nil
	}
	c.mtx.RUnlock()
	var b = new(Block)
	prefix := []byte{byte(DATAHeader)}
	data, err := c.Get(append(prefix, hash.Bytes()...))
	if err != nil {
		return nil, err
	}

	r := bytes.NewReader(data)

	// first 8 bytes is sys_fee
	_, err = ReadUint64(r)
	if err != nil {
		return nil, err
	}

	// Deserialize block data
	if err := b.Header.Deserialize(r); err != nil {
		return nil, err
	}

	//Transactions
	count, err := ReadUint32(r)
	if err != nil {
		return nil, err
	}
	b.Transactions = make([]*Transaction, count)
	for i := range b.Transactions {
		var hash Uint256
		if err := hash.Deserialize(r); err != nil {
			return nil, err
		}
		tx, _, err := c.GetTransaction(hash)
		if err != nil {
			return nil, err
		}
		b.Transactions[i] = tx
	}

	if c.blocksCache != nil {
		c.mtx.Lock()
		if len(c.blockHashesCache) >= BlocksCacheSize {
			delete(c.blocksCache, c.blockHashesCache[0])
			c.blockHashesCache = c.blockHashesCache[1:BlocksCacheSize]
		}
		c.blockHashesCache = append(c.blockHashesCache, hash)
		c.blocksCache[hash] = b
		c.mtx.Unlock()
	}

	return b, nil
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
	confirmBytes, err := c.Get(append(prefix, hash.Bytes()...))
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
