package blockchain

import (
	"bytes"
	"container/list"
	"errors"
	"fmt"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA.SideChain/core"
	"github.com/elastos/Elastos.ELA.SideChain/log"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

const (
	ValueNone   = 0
	ValueExist  = 1
	TaskChanCap = 4
)

type persistTask interface{}

type rollbackBlockTask struct {
	blockHash Uint256
	reply     chan error
}

type persistBlockTask struct {
	block *core.Block
	reply chan error
}

type ChainStoreFunctionName string

type ChainStore struct {
	IStore

	taskCh chan persistTask
	quit   chan chan bool

	mu          sync.RWMutex // guard the following var
	headerIndex map[uint32]Uint256
	headerCache map[Uint256]*core.Header
	headerIdx   *list.List

	currentBlockHeight uint32
	storedHeaderCount  uint32

	persistFunctions  []func(batch IBatch, b *core.Block) error
	rollbackFunctions []func(batch IBatch, b *core.Block) error
}

func NewChainStore() (*ChainStore, error) {
	// TODO: read config file decide which db to use.
	st, err := NewLevelDB("Chain")
	if err != nil {
		return nil, err
	}

	store := &ChainStore{
		IStore:             st,
		headerIndex:        map[uint32]Uint256{},
		headerCache:        map[Uint256]*core.Header{},
		headerIdx:          list.New(),
		currentBlockHeight: 0,
		storedHeaderCount:  0,
		taskCh:             make(chan persistTask, TaskChanCap),
		quit:               make(chan chan bool, 1),
	}
	store.Init()

	return store, nil
}

func (c *ChainStore) Init() {
	c.RegisterFunctions(true, c.persistTrimmedBlock)
	c.RegisterFunctions(true, c.persistBlockHash)
	c.RegisterFunctions(true, c.persistCurrentBlock)
	c.RegisterFunctions(true, c.persistUnspendUTXOs)
	c.RegisterFunctions(true, c.persistTransactions)
	c.RegisterFunctions(true, c.persistUnspend)

	c.RegisterFunctions(false, c.rollbackTrimmedBlock)
	c.RegisterFunctions(false, c.rollbackBlockHash)
	c.RegisterFunctions(false, c.rollbackCurrentBlock)
	c.RegisterFunctions(false, c.rollbackUnspendUTXOs)
	c.RegisterFunctions(false, c.rollbackTransactions)
	c.RegisterFunctions(false, c.rollbackUnspend)
}

func (c *ChainStore) RegisterFunctions(isPersist bool, function func(batch IBatch, b *core.Block) error) {
	if isPersist {
		c.persistFunctions = append(c.persistFunctions, function)
	} else {
		c.rollbackFunctions = append(c.rollbackFunctions, function)
	}
}

func (c *ChainStore) Close() {
	closed := make(chan bool)
	c.quit <- closed
	<-closed

	c.IStore.Close()
}

func (c *ChainStore) TaskHandler() {
	for {
		select {
		case t := <-c.taskCh:
			now := time.Now()
			switch task := t.(type) {
			case *persistBlockTask:
				task.reply <- c.handlePersistBlockTask(task.block)
				tcall := float64(time.Now().Sub(now)) / float64(time.Second)
				log.Debugf("handle block exetime: %g num transactions:%d", tcall, len(task.block.Transactions))
			case *rollbackBlockTask:
				task.reply <- c.handleRollbackBlockTask(task.blockHash)
				tcall := float64(time.Now().Sub(now)) / float64(time.Second)
				log.Debugf("handle block rollback exetime: %g", tcall)
			}

		case closed := <-c.quit:
			closed <- true
			return
		}
	}
}

// can only be invoked by backend write goroutine
func (c *ChainStore) clearCache(b *core.Block) {
	c.mu.Lock()
	defer c.mu.Unlock()

	for e := c.headerIdx.Front(); e != nil; e = e.Next() {
		n := e.Value.(core.Header)
		h := n.Hash()
		if h.IsEqual(b.Hash()) {
			c.headerIdx.Remove(e)
		}
	}
}

func (c *ChainStore) InitWithGenesisBlock(genesisBlock *core.Block) (uint32, error) {
	prefix := []byte{byte(CFG_Version)}
	version, err := c.Get(prefix)
	if err != nil {
		version = []byte{0x00}
	}

	if version[0] == 0x00 {
		// batch delete old data
		batch := c.NewBatch()
		iter := c.NewIterator(nil)
		for iter.Next() {
			batch.Delete(iter.Key())
		}
		iter.Release()

		err := batch.Commit()
		if err != nil {
			return 0, err
		}

		// persist genesis block
		err = c.persistBlock(genesisBlock)
		if err != nil {
			return 0, err
		}
		// put version to db
		err = c.Put(prefix, []byte{0x01})
		if err != nil {
			return 0, err
		}
	}

	// GenesisBlock should exist in chain
	// Or the bookkeepers are not consistent with the chain
	hash := genesisBlock.Hash()
	if !c.IsBlockInStore(hash) {
		return 0, errors.New("genesis block is not consistent with the chain")
	}

	// Get Current Block
	currentBlockPrefix := []byte{byte(SYS_CurrentBlock)}
	data, err := c.Get(currentBlockPrefix)
	if err != nil {
		return 0, err
	}

	c.currentBlockHeight, err = ReadUint32(bytes.NewReader(data))
	return c.currentBlockHeight, err
}

func (c *ChainStore) IsDuplicateTx(txId Uint256) bool {
	prefix := []byte{byte(DATA_Transaction)}
	_, err_get := c.Get(append(prefix, txId.Bytes()...))
	if err_get != nil {
		return false
	} else {
		return true
	}
}

func (c *ChainStore) IsDoubleSpend(txn *core.Transaction) bool {
	if len(txn.Inputs) == 0 {
		return false
	}

	unspentPrefix := []byte{byte(IX_Unspent)}
	for i := 0; i < len(txn.Inputs); i++ {
		txhash := txn.Inputs[i].Previous.TxID
		unspentValue, err_get := c.Get(append(unspentPrefix, txhash.Bytes()...))
		if err_get != nil {
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

func (c *ChainStore) IsDuplicateMainchainTx(mainchainTxHash Uint256) bool {
	prefix := []byte{byte(IX_MainChain_Tx)}
	_, err := c.Get(append(prefix, mainchainTxHash.Bytes()...))
	if err != nil {
		return false
	} else {
		return true
	}
}

func (c *ChainStore) GetBlockHash(height uint32) (Uint256, error) {
	queryKey := bytes.NewBuffer(nil)
	queryKey.WriteByte(byte(DATA_BlockHash))
	err := WriteUint32(queryKey, height)

	if err != nil {
		return Uint256{}, err
	}
	blockHash, err := c.Get(queryKey.Bytes())
	if err != nil {
		//TODO: implement error process
		return Uint256{}, err
	}
	blockHash256, err := Uint256FromBytes(blockHash)
	if err != nil {
		return Uint256{}, err
	}

	return *blockHash256, nil
}

func (c *ChainStore) getHeaderWithCache(hash Uint256) *core.Header {
	for e := c.headerIdx.Front(); e != nil; e = e.Next() {
		n := e.Value.(core.Header)
		eh := n.Hash()
		if eh.IsEqual(hash) {
			return &n
		}
	}

	h, _ := c.GetHeader(hash)

	return h
}

func (c *ChainStore) GetCurrentBlockHash() Uint256 {
	hash, err := c.GetBlockHash(c.currentBlockHeight)
	if err != nil {
		return Uint256{}
	}

	return hash
}

func (c *ChainStore) RollbackBlock(blockHash Uint256) error {
	reply := make(chan error)
	c.taskCh <- &rollbackBlockTask{blockHash: blockHash, reply: reply}
	return <-reply
}

func (c *ChainStore) GetHeader(hash Uint256) (*core.Header, error) {
	var h = new(core.Header)

	prefix := []byte{byte(DATA_Header)}
	data, err := c.Get(append(prefix, hash.Bytes()...))
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

func (c *ChainStore) PersistAsset(batch IBatch, assetId Uint256, asset core.Asset) error {
	w := bytes.NewBuffer(nil)

	asset.Serialize(w)

	// generate key
	assetKey := bytes.NewBuffer(nil)
	// add asset prefix.
	assetKey.WriteByte(byte(ST_Info))
	// contact asset id
	assetId.Serialize(assetKey)

	log.Debugf("asset key: %x", assetKey)

	return batch.Put(assetKey.Bytes(), w.Bytes())
}

func (c *ChainStore) GetAsset(hash Uint256) (*core.Asset, error) {
	log.Debugf("GetAsset Hash: %s", hash.String())

	asset := new(core.Asset)
	prefix := []byte{byte(ST_Info)}
	data, err := c.Get(append(prefix, hash.Bytes()...))

	log.Debugf("GetAsset Data: %s", data)
	if err != nil {
		return nil, err
	}

	asset.Deserialize(bytes.NewReader(data))

	return asset, nil
}

func (c *ChainStore) PersistMainchainTx(batch IBatch, mainchainTxHash Uint256) {
	key := []byte{byte(IX_MainChain_Tx)}
	key = append(key, mainchainTxHash.Bytes()...)

	// PUT VALUE
	batch.Put(key, []byte{byte(ValueExist)})
}

func (c *ChainStore) GetMainchainTx(mainchainTxHash Uint256) (byte, error) {
	key := []byte{byte(IX_MainChain_Tx)}
	data, err := c.Get(append(key, mainchainTxHash.Bytes()...))
	if err != nil {
		return ValueNone, err
	}

	return data[0], nil
}

func (c *ChainStore) GetTransaction(txId Uint256) (*core.Transaction, uint32, error) {
	key := append([]byte{byte(DATA_Transaction)}, txId.Bytes()...)
	value, err := c.Get(key)
	if err != nil {
		return nil, 0, err
	}

	r := bytes.NewReader(value)
	height, err := ReadUint32(r)
	if err != nil {
		return nil, 0, err
	}

	var txn core.Transaction
	if err := txn.Deserialize(r); err != nil {
		return nil, height, err
	}

	return &txn, height, nil
}

func (c *ChainStore) GetTxReference(tx *core.Transaction) (map[*core.Input]*core.Output, error) {
	if tx.TxType == core.RegisterAsset {
		return nil, nil
	}
	//UTXO input /  Outputs
	reference := make(map[*core.Input]*core.Output)
	// Key index，v UTXOInput
	for _, utxo := range tx.Inputs {
		transaction, _, err := c.GetTransaction(utxo.Previous.TxID)
		if err != nil {
			return nil, errors.New("GetTxReference failed, previous transaction not found")
		}
		index := utxo.Previous.Index
		if int(index) >= len(transaction.Outputs) {
			return nil, errors.New("GetTxReference failed, refIdx out of range.")
		}
		reference[utxo] = transaction.Outputs[index]
	}
	return reference, nil
}

func (c *ChainStore) PersistTransaction(batch IBatch, tx *core.Transaction, height uint32) error {
	// generate key with DATA_Transaction prefix
	key := new(bytes.Buffer)
	// add transaction header prefix.
	key.WriteByte(byte(DATA_Transaction))
	// get transaction hash
	hash := tx.Hash()
	if err := hash.Serialize(key); err != nil {
		return err
	}
	log.Debugf("transaction header + hash: %x", key)

	// generate value
	value := new(bytes.Buffer)
	WriteUint32(value, height)
	if err := tx.Serialize(value); err != nil {
		return err
	}
	log.Debugf("transaction tx data: %x", value)

	return batch.Put(key.Bytes(), value.Bytes())
}

func (c *ChainStore) GetBlock(hash Uint256) (*core.Block, error) {
	var b = new(core.Block)
	prefix := []byte{byte(DATA_Header)}
	bHash, err := c.Get(append(prefix, hash.Bytes()...))
	if err != nil {
		return nil, err
	}

	r := bytes.NewReader(bHash)

	// first 8 bytes is sys_fee
	_, err = ReadUint64(r)
	if err != nil {
		return nil, err
	}

	// Deserialize block data
	if err := b.FromTrimmedData(r); err != nil {
		return nil, err
	}

	// Deserialize transaction
	for i, txn := range b.Transactions {
		tmp, _, err := c.GetTransaction(txn.Hash())
		if err != nil {
			return nil, err
		}
		b.Transactions[i] = tmp
	}

	return b, nil
}

// can only be invoked by backend write goroutine
func (c *ChainStore) addHeader(header *core.Header) {

	log.Debugf("addHeader(), Height=%d", header.Height)

	hash := header.Hash()

	c.mu.Lock()
	c.headerCache[header.Hash()] = header
	c.headerIndex[header.Height] = hash
	c.headerIdx.PushBack(*header)
	c.mu.Unlock()

	log.Debug("[addHeader]: finish, header height:", header.Height)
}

func (c *ChainStore) SaveBlock(b *core.Block) error {
	log.Debug("SaveBlock()")

	reply := make(chan error)
	c.taskCh <- &persistBlockTask{block: b, reply: reply}
	return <-reply
}

func (c *ChainStore) handlePersistBlockTask(block *core.Block) error {
	if block.Header.Height <= c.currentBlockHeight {
		return nil
	}

	if err := c.persistBlock(block); err != nil {
		return fmt.Errorf("persist block %s error %s", block.Hash().String(), err)
	}

	c.mu.Lock()
	c.currentBlockHeight = block.Header.Height
	c.mu.Unlock()

	c.clearCache(block)
	return nil
}

func (c *ChainStore) persistBlock(b *core.Block) error {
	batch := c.NewBatch()
	for _, persistFunc := range c.persistFunctions {
		if err := persistFunc(batch, b); err != nil {
			return err
		}
	}
	return batch.Commit()
}

func (c *ChainStore) handleRollbackBlockTask(blockHash Uint256) error {
	block, err := c.GetBlock(blockHash)
	if err != nil {
		return fmt.Errorf("block %s can't be found", blockHash.String())
	}

	if err := c.rollbackBlock(block); err != nil {
		return fmt.Errorf("rollback block %s error %s", blockHash.String(), err)
	}

	c.mu.Lock()
	c.currentBlockHeight = block.Header.Height - 1
	c.mu.Unlock()

	return nil
}

func (c *ChainStore) rollbackBlock(b *core.Block) error {
	batch := c.NewBatch()
	for _, rollbackFunc := range c.rollbackFunctions {
		rollbackFunc(batch, b)
	}
	return batch.Commit()
}

func (c *ChainStore) GetUnspent(txid Uint256, index uint16) (*core.Output, error) {
	if ok, _ := c.ContainsUnspent(txid, index); ok {
		tx, _, err := c.GetTransaction(txid)
		if err != nil {
			return nil, err
		}

		return tx.Outputs[index], nil
	}

	return nil, errors.New("[GetUnspent] NOT ContainsUnspent.")
}

func (c *ChainStore) ContainsUnspent(txid Uint256, index uint16) (bool, error) {
	unspentPrefix := []byte{byte(IX_Unspent)}
	unspentValue, err := c.Get(append(unspentPrefix, txid.Bytes()...))

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
	c.mu.RLock()
	defer c.mu.RUnlock()

	return c.currentBlockHeight
}

func (c *ChainStore) IsBlockInStore(hash Uint256) bool {
	var b = new(core.Block)
	prefix := []byte{byte(DATA_Header)}
	log.Debug("Get block key: ", BytesToHexString(append(prefix, hash.Bytes()...)))
	blockData, err := c.Get(append(prefix, hash.Bytes()...))
	if err != nil {
		return false
	}

	r := bytes.NewReader(blockData)

	// first 8 bytes is sys_fee
	_, err = ReadUint64(r)
	if err != nil {
		log.Error("Read sys_fee failed: ", err)
		return false
	}

	// Deserialize block data
	err = b.FromTrimmedData(r)
	if err != nil {
		log.Error("Get trimmed data failed: ", err)
		return false
	}

	if b.Header.Height > c.currentBlockHeight {
		log.Error("Header height", b.Header.Height, "greater then current height:", c.currentBlockHeight)
		return false
	}

	return true
}

func (c *ChainStore) GetUnspentElementFromProgramHash(programHash Uint168, assetid Uint256, height uint32) ([]*UTXO, error) {
	prefix := []byte{byte(IX_Unspent_UTXO)}
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
	unspents := make([]*UTXO, listNum)
	for i := 0; i < int(listNum); i++ {
		uu := new(UTXO)
		err := uu.Deserialize(r)
		if err != nil {
			return nil, err
		}

		unspents[i] = uu
	}

	return unspents, nil
}

func (c *ChainStore) GetAssetUnspents(programHash Uint168, assetid Uint256) ([]*UTXO, error) {
	unspents := make([]*UTXO, 0)

	key := []byte{byte(IX_Unspent_UTXO)}
	key = append(key, programHash.Bytes()...)
	key = append(key, assetid.Bytes()...)
	iter := c.NewIterator(key)
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
				return nil, err
			}

			unspents = append(unspents, uu)
		}
	}

	return unspents, nil
}

func (c *ChainStore) GetUnspents(programHash Uint168) (map[Uint256][]*UTXO, error) {
	uxtoUnspents := make(map[Uint256][]*UTXO)

	prefix := []byte{byte(IX_Unspent_UTXO)}
	key := append(prefix, programHash.Bytes()...)
	iter := c.NewIterator(key)
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
		unspents := make([]*UTXO, listNum)
		for i := 0; i < int(listNum); i++ {
			uu := new(UTXO)
			err := uu.Deserialize(r)
			if err != nil {
				return nil, err
			}

			unspents[i] = uu
		}
		uxtoUnspents[assetid] = append(uxtoUnspents[assetid], unspents[:]...)
	}

	return uxtoUnspents, nil
}

func (c *ChainStore) PersistUnspentWithProgramHash(batch IBatch, programHash Uint168, assetid Uint256, height uint32, unspents []*UTXO) error {
	prefix := []byte{byte(IX_Unspent_UTXO)}
	prefix = append(prefix, programHash.Bytes()...)
	prefix = append(prefix, assetid.Bytes()...)
	key := bytes.NewBuffer(prefix)
	if err := WriteUint32(key, height); err != nil {
		return err
	}

	if len(unspents) == 0 {
		batch.Delete(key.Bytes())
		return nil
	}

	listnum := len(unspents)
	w := bytes.NewBuffer(nil)
	WriteVarUint(w, uint64(listnum))
	for i := 0; i < listnum; i++ {
		unspents[i].Serialize(w)
	}

	// BATCH PUT VALUE
	batch.Put(key.Bytes(), w.Bytes())

	return nil
}

func (c *ChainStore) GetAssets() map[Uint256]*core.Asset {
	assets := make(map[Uint256]*core.Asset)

	iter := c.NewIterator([]byte{byte(ST_Info)})
	for iter.Next() {
		rk := bytes.NewReader(iter.Key())

		// read prefix
		_, _ = ReadBytes(rk, 1)
		var assetid Uint256
		assetid.Deserialize(rk)
		log.Tracef("[GetAssets] assetid: %x", assetid.Bytes())

		asset := new(core.Asset)
		r := bytes.NewReader(iter.Value())
		asset.Deserialize(r)

		assets[assetid] = asset
	}

	return assets
}
