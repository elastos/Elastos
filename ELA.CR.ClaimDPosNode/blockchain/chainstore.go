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

func NewChainStore(dataDir string, genesisBlock *Block) (
	IChainStore, error) {
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

	if err := s.init(genesisBlock); err != nil {
		log.Debug("chain store not contain genesis block")
	}

	return s, nil
}

func (c *ChainStore) Close() {
	c.persistMutex.Lock()
	defer c.persistMutex.Unlock()
	if err := c.fflDB.Close(); err != nil {
		log.Error("fflDB close failed:", err)
	}
	if err := c.IStore.Close(); err != nil {
		log.Error("IStore close failed:", err)
	}
}

func (c *ChainStore) init(genesisBlock *Block) error {
	prefix := []byte{byte(CFGVersion)}
	version, err := c.Get(prefix)
	if err != nil {
		version = []byte{0x00}
	}

	if version[0] == 0x00 {
		// batch delete old data
		c.NewBatch()
		iter := c.NewIterator(nil)
		for iter.Next() {
			c.BatchDelete(iter.Key())
		}
		iter.Release()

		err := c.BatchCommit()
		if err != nil {
			return err
		}

		// persist genesis block
		hash := genesisBlock.Hash()
		genesisBlockNode := NewBlockNode(&genesisBlock.Header, &hash)
		err = c.persist(genesisBlock, genesisBlockNode, nil,
			CalcPastMedianTime(genesisBlockNode))
		if err != nil {
			return err
		}

		// put version to db
		err = c.Put(prefix, []byte{0x01})
		if err != nil {
			return err
		}
	}

	// GenesisBlock should exist in chain
	// Or the bookkeepers are not consistent with the chain
	hash := genesisBlock.Hash()
	if !c.IsBlockInStore(&hash) {
		return errors.New("genesis block is not consistent with the chain")
	}

	// Get Current Block
	currentBlockPrefix := []byte{byte(SYSCurrentBlock)}
	data, err := c.Get(currentBlockPrefix)
	if err != nil {
		return err
	}

	r := bytes.NewReader(data)
	var blockHash Uint256
	blockHash.Deserialize(r)
	c.currentBlockHeight, err = ReadUint32(r)
	return err
}

func (c *ChainStore) IsTxHashDuplicate(txID Uint256) bool {
	txn, _, err := c.fflDB.GetTransaction(txID)
	if err != nil || txn == nil {
		return false
	}

	return true
}

func (c *ChainStore) isTxHashDuplicate(txhash Uint256) bool {
	prefix := []byte{byte(DATATransaction)}
	_, err := c.Get(append(prefix, txhash.Bytes()...))
	if err != nil {
		return false
	} else {
		return true
	}
}

func (c *ChainStore) IsSidechainTxHashDuplicate(sidechainTxHash Uint256) bool {
	prefix := []byte{byte(IXSideChainTx)}
	_, err := c.Get(append(prefix, sidechainTxHash.Bytes()...))
	if err != nil {
		return false
	} else {
		return true
	}
}

func (c *ChainStore) IsDoubleSpend(txn *Transaction) bool {
	if len(txn.Inputs) == 0 {
		return false
	}

	unspentPrefix := []byte{byte(IXUnspent)}
	for i := 0; i < len(txn.Inputs); i++ {
		txID := txn.Inputs[i].Previous.TxID
		unspentValue, err := c.Get(append(unspentPrefix, txID.Bytes()...))
		if err != nil {
			return true
		}

		unspents, _ := GetUint16Array(unspentValue)
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

func (c *ChainStore) GetBlockHash(height uint32) (Uint256, error) {
	queryKey := new(bytes.Buffer)
	queryKey.WriteByte(byte(DATABlockHash))
	err := WriteUint32(queryKey, height)

	if err != nil {
		return EmptyHash, err
	}
	blockHash, err := c.Get(queryKey.Bytes())
	if err != nil {
		//TODO: implement error process
		return EmptyHash, err
	}
	blockHash256, err := Uint256FromBytes(blockHash)
	if err != nil {
		return EmptyHash, err
	}

	return *blockHash256, nil
}

func (c *ChainStore) GetCurrentBlockHash() Uint256 {
	hash, err := c.GetBlockHash(c.currentBlockHeight)
	if err != nil {
		return EmptyHash
	}

	return hash
}

func (c *ChainStore) RollbackBlock(b *Block, node *BlockNode,
	confirm *payload.Confirm, medianTimePast time.Time) error {
	now := time.Now()
	err := c.handleRollbackBlockTask(b, node, confirm, medianTimePast)
	tcall := float64(time.Now().Sub(now)) / float64(time.Second)
	log.Debugf("handle block rollback exetime: %g", tcall)
	return err
}

func (c *ChainStore) GetHeader(hash Uint256) (*Header, error) {
	var h = new(Header)

	prefix := []byte{byte(DATAHeader)}
	data, err := c.Get(append(prefix, hash.Bytes()...))
	//log.Debug( "Get Header Data: %x\n",  data )
	if err != nil {
		//TODO: implement error process
		return nil, err
	}

	r := bytes.NewReader(data)
	// first 8 bytes is sys_fee
	_, err = ReadUint64(r)
	if err != nil {
		return nil, err
	}

	// Deserialize block data
	err = h.Deserialize(r)
	if err != nil {
		return nil, err
	}

	return h, err
}

func (c *ChainStore) PersistAsset(assetID Uint256, asset payload.Asset) error {
	w := new(bytes.Buffer)

	asset.Serialize(w)

	// generate key
	assetKey := new(bytes.Buffer)
	// add asset prefix.
	assetKey.WriteByte(byte(STInfo))
	// contact asset id
	if err := assetID.Serialize(assetKey); err != nil {
		return err
	}

	log.Debugf("asset key: %x", assetKey)

	// PUT VALUE
	c.BatchPut(assetKey.Bytes(), w.Bytes())
	return nil
}

func (c *ChainStore) GetAsset(hash Uint256) (*payload.Asset, error) {
	asset := new(payload.Asset)
	prefix := []byte{byte(STInfo)}
	data, err := c.Get(append(prefix, hash.Bytes()...))
	if err != nil {
		return nil, err
	}
	err = asset.Deserialize(bytes.NewReader(data))
	if err != nil {
		return nil, err
	}

	return asset, nil
}

func (c *ChainStore) PersistSidechainTx(sidechainTxHash Uint256) {
	key := []byte{byte(IXSideChainTx)}
	key = append(key, sidechainTxHash.Bytes()...)

	// PUT VALUE
	c.BatchPut(key, []byte{0})
}

func (c *ChainStore) GetSidechainTx(sidechainTxHash Uint256) (byte, error) {
	key := []byte{byte(IXSideChainTx)}
	data, err := c.Get(append(key, sidechainTxHash.Bytes()...))
	if err != nil {
		return 0, err
	}

	return data[0], nil
}

func (c *ChainStore) GetTransaction(txID Uint256) (*Transaction, uint32, error) {
	return c.fflDB.GetTransaction(txID)
}

func (c *ChainStore) getTransaction(txID Uint256) (*Transaction, uint32, error) {
	key := append([]byte{byte(DATATransaction)}, txID.Bytes()...)
	value, err := c.Get(key)
	if err != nil {
		return nil, 0, err
	}

	r := bytes.NewReader(value)
	height, err := ReadUint32(r)
	if err != nil {
		return nil, 0, err
	}

	var txn Transaction
	if err := txn.Deserialize(r); err != nil {
		return nil, height, err
	}

	return &txn, height, nil
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

func (c *ChainStore) persistTransaction(tx *Transaction, height uint32) error {
	// generate key with DATA_Transaction prefix
	key := new(bytes.Buffer)
	// add transaction header prefix.
	key.WriteByte(byte(DATATransaction))
	// get transaction hash
	hash := tx.Hash()
	if err := hash.Serialize(key); err != nil {
		return err
	}

	// generate value
	value := new(bytes.Buffer)
	if err := WriteUint32(value, height); err != nil {
		return err
	}
	if err := tx.Serialize(value); err != nil {
		return err
	}

	// put value
	c.BatchPut(key.Bytes(), value.Bytes())
	return nil
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

func (c *ChainStore) getBlockHeader(hash Uint256) (*Header, error) {
	header := new(Header)
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

	// deserialize block header data
	if err := header.Deserialize(r); err != nil {
		return nil, err
	}

	return header, nil
}

func (c *ChainStore) rollback(b *Block, node *BlockNode,
	confirm *payload.Confirm, medianTimePast time.Time) error {
	c.NewBatch()
	if err := c.RollbackTransactions(b); err != nil {
		return err
	}
	if err := c.RollbackUnspendUTXOs(b); err != nil {
		return err
	}
	if err := c.RollbackUnspend(b); err != nil {
		return err
	}
	if err := c.RollbackConfirm(b); err != nil {
		return err
	}

	if err := c.fflDB.RollbackBlock(b, node, confirm, medianTimePast); err != nil {
		return err
	}

	if err := c.BatchCommit(); err != nil {
		return err
	}

	atomic.StoreUint32(&c.currentBlockHeight, b.Height-1)

	return nil
}

func (c *ChainStore) persist(b *Block, node *BlockNode,
	confirm *payload.Confirm, medianTimePast time.Time) error {
	c.persistMutex.Lock()
	defer c.persistMutex.Unlock()

	c.NewBatch()
	if err := c.PersistTransactions(b); err != nil {
		return err
	}
	if err := c.persistUTXOs(b); err != nil {
		return err
	}
	if err := c.persistUnspend(b); err != nil {
		return err
	}
	if err := c.persistConfirm(confirm); err != nil {
		return err
	}
	// todo save genesis block at same time
	if b.Height != 0 {
		if err := c.fflDB.SaveBlock(b, node, confirm, medianTimePast); err != nil {
			return err
		}
	}
	return c.BatchCommit()
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

func (c *ChainStore) persistConfirm(confirm *payload.Confirm) error {
	if confirm == nil {
		return nil
	}
	if err := c.PersistConfirm(confirm); err != nil {
		log.Fatal("[persistConfirm]: error to persist confirm:", err.Error())
		return err
	}
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

func (c *ChainStore) GetUnspent(txID Uint256, index uint16) (*Output, error) {
	if ok, _ := c.ContainsUnspent(txID, index); ok {
		tx, _, err := c.GetTransaction(txID)
		if err != nil {
			return nil, err
		}

		return tx.Outputs[index], nil
	}

	return nil, errors.New("[GetUnspent] NOT ContainsUnspent.")
}

func (c *ChainStore) ContainsUnspent(txID Uint256, index uint16) (bool, error) {
	unspentPrefix := []byte{byte(IXUnspent)}
	unspentValue, err := c.Get(append(unspentPrefix, txID.Bytes()...))

	if err != nil {
		return false, err
	}

	unspentArray, err := GetUint16Array(unspentValue)
	if err != nil {
		return false, err
	}

	for i := 0; i < len(unspentArray); i++ {
		if unspentArray[i] == index {
			return true, nil
		}
	}

	return false, nil
}

func (c *ChainStore) GetHeight() uint32 {
	return atomic.LoadUint32(&c.currentBlockHeight)
}

func (c *ChainStore) SetHeight(height uint32) {
	atomic.StoreUint32(&c.currentBlockHeight, height)
}

func (c *ChainStore) IsBlockInStore(hash *Uint256) bool {
	h, err := c.getBlockHeader(*hash)
	if err != nil {
		return false
	}

	if h.Height > c.currentBlockHeight {
		return false
	}

	return true
}

func (c *ChainStore) GetUnspentElementFromProgramHash(programHash Uint168, assetid Uint256, height uint32) ([]*UTXO, error) {
	prefix := []byte{byte(IXUnspentUTXO)}
	prefix = append(prefix, programHash.Bytes()...)
	prefix = append(prefix, assetid.Bytes()...)

	key := bytes.NewBuffer(prefix)
	if err := WriteUint32(key, height); err != nil {
		return nil, err
	}
	unspentsData, err := c.Get(key.Bytes())
	if err != nil {
		return nil, err
	}
	r := bytes.NewReader(unspentsData)
	listNum, err := ReadVarUint(r, 0)
	if err != nil {
		return nil, err
	}

	// read unspent list in store
	unspents := make([]*UTXO, 0, listNum)
	for i := 0; i < int(listNum); i++ {
		uu := new(UTXO)
		err := uu.Deserialize(r)
		if err != nil {
			break
		}

		unspents = append(unspents, uu)
	}

	return unspents, nil
}

func (c *ChainStore) GetUnspentFromProgramHash(programHash Uint168, assetid Uint256) ([]*UTXO, error) {
	unspents := make([]*UTXO, 0)

	key := []byte{byte(IXUnspentUTXO)}
	key = append(key, programHash.Bytes()...)
	key = append(key, assetid.Bytes()...)
	iter := c.NewIterator(key)
	defer iter.Release()
	for iter.Next() {
		r := bytes.NewReader(iter.Value())
		listNum, err := ReadVarUint(r, 0)
		if err != nil {
			return nil, err
		}

		for i := 0; i < int(listNum); i++ {
			uu := new(UTXO)
			err := uu.Deserialize(r)
			if err != nil {
				break
			}

			unspents = append(unspents, uu)
		}

	}

	return unspents, nil

}

func (c *ChainStore) GetUnspentsFromProgramHash(programHash Uint168) (map[Uint256][]*UTXO, error) {
	uxtoUnspents := make(map[Uint256][]*UTXO)

	prefix := []byte{byte(IXUnspentUTXO)}
	key := append(prefix, programHash.Bytes()...)
	iter := c.NewIterator(key)
	defer iter.Release()
	for iter.Next() {
		rk := bytes.NewReader(iter.Key())

		// read prefix
		_, _ = ReadBytes(rk, 1)
		var ph Uint168
		ph.Deserialize(rk)
		var assetid Uint256
		assetid.Deserialize(rk)

		r := bytes.NewReader(iter.Value())
		listNum, err := ReadVarUint(r, 0)
		if err != nil {
			return nil, err
		}

		// read unspent list in store
		unspents := make([]*UTXO, 0, listNum)
		for i := 0; i < int(listNum); i++ {
			uu := new(UTXO)
			err := uu.Deserialize(r)
			if err != nil {
				break
			}

			unspents = append(unspents, uu)
		}
		uxtoUnspents[assetid] = append(uxtoUnspents[assetid], unspents[:]...)
	}

	return uxtoUnspents, nil
}

func (c *ChainStore) PersistUnspentWithProgramHash(programHash Uint168, assetid Uint256, height uint32, unspents []*UTXO) error {
	prefix := []byte{byte(IXUnspentUTXO)}
	prefix = append(prefix, programHash.Bytes()...)
	prefix = append(prefix, assetid.Bytes()...)
	key := bytes.NewBuffer(prefix)
	if err := WriteUint32(key, height); err != nil {
		return err
	}

	storeCount := 0
	listnum := len(unspents)
	w := new(bytes.Buffer)
	for i := 0; i < listnum; i++ {
		if unspents[i].Value > 0 {
			storeCount++
		}
	}
	if storeCount == 0 {
		c.BatchDelete(key.Bytes())
		return nil
	}
	WriteVarUint(w, uint64(storeCount))
	for i := 0; i < listnum; i++ {
		if unspents[i].Value > 0 {
			unspents[i].Serialize(w)
		}
	}

	// BATCH PUT VALUE
	c.BatchPut(key.Bytes(), w.Bytes())
	return nil
}

func (c *ChainStore) GetAssets() map[Uint256]*payload.Asset {
	assets := make(map[Uint256]*payload.Asset)

	iter := c.NewIterator([]byte{byte(STInfo)})
	defer iter.Release()
	for iter.Next() {
		rk := bytes.NewReader(iter.Key())

		// read prefix
		_, _ = ReadBytes(rk, 1)
		var assetid Uint256
		assetid.Deserialize(rk)
		log.Debugf("[GetAssets] assetid: %x", assetid.Bytes())

		asset := new(payload.Asset)
		r := bytes.NewReader(iter.Value())
		asset.Deserialize(r)

		assets[assetid] = asset
	}

	return assets
}
