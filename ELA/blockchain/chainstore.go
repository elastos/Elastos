package blockchain

import (
	"bytes"
	"container/list"
	"errors"
	"fmt"
	"sync"
	"time"

	. "github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/events"
	"github.com/elastos/Elastos.ELA/log"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

const ValueNone = 0
const ValueExist = 1

const TaskChanCap = 4

var (
	ErrDBNotFound = errors.New("leveldb: not found")
)

type persistTask interface{}

type rollbackBlockTask struct {
	blockHash Uint256
	reply     chan bool
}
type persistBlockTask struct {
	block *Block
	reply chan bool
}

type ChainStore struct {
	IStore

	taskCh chan persistTask
	quit   chan chan bool

	mu          sync.RWMutex // guard the following var
	headerIndex map[uint32]Uint256
	headerCache map[Uint256]*Header
	//blockCache  map[Uint256]*Block
	headerIdx *list.List

	currentBlockHeight uint32
	storedHeaderCount  uint32
}

func NewChainStore() (IChainStore, error) {
	st, err := NewLevelDB("Chain")
	if err != nil {
		return nil, err
	}

	store := &ChainStore{
		IStore:      st,
		headerIndex: map[uint32]Uint256{},
		//blockCache:         map[Uint256]*Block{},
		headerCache:        map[Uint256]*Header{},
		headerIdx:          list.New(),
		currentBlockHeight: 0,
		storedHeaderCount:  0,
		taskCh:             make(chan persistTask, TaskChanCap),
		quit:               make(chan chan bool, 1),
	}

	go store.loop()

	return store, nil
}

func (c *ChainStore) Close() {
	closed := make(chan bool)
	c.quit <- closed
	<-closed
	c.IStore.Close()
}

func (c *ChainStore) loop() {
	for {
		select {
		case t := <-c.taskCh:
			now := time.Now()
			switch task := t.(type) {
			case *persistBlockTask:
				c.handlePersistBlockTask(task.block)
				task.reply <- true
				tcall := float64(time.Now().Sub(now)) / float64(time.Second)
				log.Debugf("handle block exetime: %g num transactions:%d", tcall, len(task.block.Transactions))
			case *rollbackBlockTask:
				c.handleRollbackBlockTask(task.blockHash)
				task.reply <- true
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
func (c *ChainStore) clearCache(b *Block) {
	c.mu.Lock()
	defer c.mu.Unlock()

	for e := c.headerIdx.Front(); e != nil; e = e.Next() {
		n := e.Value.(Header)
		h := n.Hash()
		if h.IsEqual(b.Hash()) {
			c.headerIdx.Remove(e)
		}
	}
}

func (c *ChainStore) InitWithGenesisBlock(genesisBlock *Block) (uint32, error) {
	prefix := []byte{byte(CFG_Version)}
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
			return 0, err
		}

		// persist genesis block
		err = c.persist(genesisBlock)
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
	DefaultLedger.Blockchain.GenesisHash = hash
	//c.headerIndex[0] = hash

	// Get Current Block
	currentBlockPrefix := []byte{byte(SYS_CurrentBlock)}
	data, err := c.Get(currentBlockPrefix)
	if err != nil {
		return 0, err
	}

	r := bytes.NewReader(data)
	var blockHash Uint256
	blockHash.Deserialize(r)
	c.currentBlockHeight, err = ReadUint32(r)
	endHeight := c.currentBlockHeight

	startHeight := uint32(0)
	if endHeight > MinMemoryNodes {
		startHeight = endHeight - MinMemoryNodes
	}

	for start := startHeight; start <= endHeight; start++ {
		hash, err := c.GetBlockHash(start)
		if err != nil {
			return 0, err
		}
		header, err := c.GetHeader(hash)
		if err != nil {
			return 0, err
		}
		node, err := DefaultLedger.Blockchain.LoadBlockNode(header, &hash)
		if err != nil {
			return 0, err
		}

		// This node is now the end of the best chain.
		DefaultLedger.Blockchain.BestChain = node

	}
	//c.ledger.Blockchain.DumpState()

	return c.currentBlockHeight, nil

}

func (c *ChainStore) IsTxHashDuplicate(txhash Uint256) bool {
	prefix := []byte{byte(DATA_Transaction)}
	_, err := c.Get(append(prefix, txhash.Bytes()...))
	if err != nil {
		return false
	} else {
		return true
	}
}

func (c *ChainStore) IsSidechainTxHashDuplicate(sidechainTxHash string) bool {
	prefix := []byte{byte(IX_SideChain_Tx)}
	_, err := c.Get(append(prefix, []byte(sidechainTxHash)...))
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

	unspentPrefix := []byte{byte(IX_Unspent)}
	for i := 0; i < len(txn.Inputs); i++ {
		txId := txn.Inputs[i].Previous.TxID
		unspentValue, err := c.Get(append(unspentPrefix, txId.Bytes()...))
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

func (c *ChainStore) getHeaderWithCache(hash Uint256) *Header {
	for e := c.headerIdx.Front(); e != nil; e = e.Next() {
		n := e.Value.(Header)
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

	reply := make(chan bool)
	c.taskCh <- &rollbackBlockTask{blockHash: blockHash, reply: reply}
	<-reply

	return nil
}

func (c *ChainStore) GetHeader(hash Uint256) (*Header, error) {
	var h = new(Header)

	prefix := []byte{byte(DATA_Header)}
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

func (c *ChainStore) PersistAsset(assetId Uint256, asset Asset) error {
	w := new(bytes.Buffer)

	asset.Serialize(w)

	// generate key
	assetKey := new(bytes.Buffer)
	// add asset prefix.
	assetKey.WriteByte(byte(ST_Info))
	// contact asset id
	if err := assetId.Serialize(assetKey); err != nil {
		return err
	}

	log.Debug(fmt.Sprintf("asset key: %x", assetKey))

	// PUT VALUE
	c.BatchPut(assetKey.Bytes(), w.Bytes())
	return nil
}

func (c *ChainStore) GetAsset(hash Uint256) (*Asset, error) {
	log.Debugf("GetAsset Hash: %s", hash.String())

	asset := new(Asset)
	prefix := []byte{byte(ST_Info)}
	data, err := c.Get(append(prefix, hash.Bytes()...))
	if err != nil {
		return nil, err
	}
	log.Debugf("GetAsset Data: %s", data)

	err = asset.Deserialize(bytes.NewReader(data))
	if err != nil {
		return nil, err
	}

	return asset, nil
}

func (c *ChainStore) PersistSidechainTx(sidechainTxHash string) {
	key := []byte{byte(IX_SideChain_Tx)}
	key = append(key, []byte(sidechainTxHash)...)

	// PUT VALUE
	c.BatchPut(key, []byte{byte(ValueExist)})
}

func (c *ChainStore) GetSidechainTx(sidechainTxHash string) (byte, error) {
	key := []byte{byte(IX_SideChain_Tx)}
	data, err := c.Get(append(key, []byte(sidechainTxHash)...))
	if err != nil {
		return ValueNone, err
	}

	return data[0], nil
}

func (c *ChainStore) GetTransaction(txId Uint256) (*Transaction, uint32, error) {
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
	//UTXO input /  Outputs
	reference := make(map[*Input]*Output)
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

func (c *ChainStore) PersistTransaction(tx *Transaction, height uint32) error {
	// generate key with DATA_Transaction prefix
	key := new(bytes.Buffer)
	// add transaction header prefix.
	key.WriteByte(byte(DATA_Transaction))
	// get transaction hash
	hash := tx.Hash()
	if err := hash.Serialize(key); err != nil {
		return err
	}
	log.Debug(fmt.Sprintf("transaction header + hash: %x", key))

	// generate value
	value := new(bytes.Buffer)
	if err := WriteUint32(value, height); err != nil {
		return err
	}
	if err := tx.Serialize(value); err != nil {
		return err
	}
	log.Debug(fmt.Sprintf("transaction tx data: %x", value))

	// put value
	c.BatchPut(key.Bytes(), value.Bytes())
	return nil
}

func (c *ChainStore) GetBlock(hash Uint256) (*Block, error) {
	var b = new(Block)
	prefix := []byte{byte(DATA_Header)}
	bHash, err := c.Get(append(prefix, hash.Bytes()...))
	if err != nil {
		return nil, err
	}

	reader := bytes.NewReader(bHash)

	// first 8 bytes is sys_fee
	_, err = ReadUint64(reader)
	if err != nil {
		return nil, err
	}

	// Deserialize block data
	if err := b.FromTrimmedData(reader); err != nil {
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

func (c *ChainStore) rollback(b *Block) error {
	c.NewBatch()
	c.RollbackTrimmedBlock(b)
	c.RollbackBlockHash(b)
	c.RollbackTransactions(b)
	c.RollbackUnspendUTXOs(b)
	c.RollbackUnspend(b)
	c.RollbackCurrentBlock(b)
	c.BatchCommit()

	DefaultLedger.Blockchain.UpdateBestHeight(b.Header.Height - 1)
	c.mu.Lock()
	c.currentBlockHeight = b.Header.Height - 1
	c.mu.Unlock()

	DefaultLedger.Blockchain.BCEvents.Notify(events.EventRollbackTransaction, b)

	return nil
}

func (c *ChainStore) persist(b *Block) error {
	c.NewBatch()
	if err := c.PersistTrimmedBlock(b); err != nil {
		return err
	}
	if err := c.PersistBlockHash(b); err != nil {
		return err
	}
	if err := c.PersistTransactions(b); err != nil {
		return err
	}
	if err := c.PersistUnspendUTXOs(b); err != nil {
		return err
	}
	if err := c.PersistUnspend(b); err != nil {
		return err
	}
	if err := c.PersistCurrentBlock(b); err != nil {
		return err
	}
	return c.BatchCommit()
}

// can only be invoked by backend write goroutine
func (c *ChainStore) addHeader(header *Header) {

	log.Debugf("addHeader(), Height=%d", header.Height)

	hash := header.Hash()

	c.mu.Lock()
	c.headerCache[header.Hash()] = header
	c.headerIndex[header.Height] = hash
	c.headerIdx.PushBack(*header)
	c.mu.Unlock()

	log.Debug("[addHeader]: finish, header height:", header.Height)
}

func (c *ChainStore) SaveBlock(b *Block) error {
	log.Debug("SaveBlock()")
	//log.Trace("validation.PowVerifyBlock(b, ledger, false)")
	//err := validation.PowVerifyBlock(b, ledger, false)
	//if err != nil {
	//	log.Error("PowVerifyBlock error!")
	//	return err
	//}
	//log.Trace("validation.PowVerifyBlock(b, ledger, false)222222")

	reply := make(chan bool)
	c.taskCh <- &persistBlockTask{block: b, reply: reply}
	<-reply

	return nil
}

func (c *ChainStore) handleRollbackBlockTask(blockHash Uint256) {
	block, err := c.GetBlock(blockHash)
	if err != nil {
		log.Errorf("block %x can't be found", BytesToHexString(blockHash.Bytes()))
		return
	}
	c.rollback(block)
}

func (c *ChainStore) handlePersistBlockTask(b *Block) {

	if b.Header.Height <= c.currentBlockHeight {
		return
	}

	//	c.mu.Lock()
	//c.blockCache[b.Hash()] = b
	//c.mu.Unlock()

	//log.Trace(b.Blockdata.Height)
	//log.Trace(b.Blockdata)
	//log.Trace(b.Transactions[0])
	//if b.Blockdata.Height < uint32(len(c.headerIndex)) {
	c.persistBlock(b)

	//c.NewBatch()
	//storedHeaderCount := c.storedHeaderCount
	//for c.currentBlockHeight-storedHeaderCount >= HeaderHashListCount {
	//	hashBuffer := new(bytes.Buffer)
	//	serialize.WriteVarUint(hashBuffer, uint64(HeaderHashListCount))
	//	var hashArray []byte
	//	for i := 0; i < HeaderHashListCount; i++ {
	//		index := storedHeaderCount + uint32(i)
	//		thash := c.headerIndex[index]
	//		thehash := thash.ToArray()
	//		hashArray = append(hashArray, thehash...)
	//	}
	//	hashBuffer.Write(hashArray)

	//	hhlPrefix := new(bytes.Buffer)
	//	hhlPrefix.WriteByte(byte(IX_HeaderHashList))
	//	serialize.WriteUint32(hhlPrefix, storedHeaderCount)

	//	c.BatchPut(hhlPrefix.Bytes(), hashBuffer.Bytes())
	//	storedHeaderCount += HeaderHashListCount
	//}

	//err := c.BatchCommit()
	//if err != nil {
	//	log.Error("failed to persist header hash list:", err)
	//	return
	//}
	//c.mu.Lock()
	//c.storedHeaderCount = storedHeaderCount
	//c.mu.Unlock()
	c.clearCache(b)
	//}
}

func (c *ChainStore) persistBlock(block *Block) {
	//stopHeight := uint32(len(c.headerIndex))
	//for h := c.currentBlockHeight + 1; h <= stopHeight; h++ {
	//hash := c.headerIndex[h]
	//block, ok := c.blockCache[hash]
	//if !ok {
	//	break
	//}
	//log.Trace(block.Blockdata)
	//log.Trace(block.Transactions[0])
	err := c.persist(block)
	if err != nil {
		log.Fatal("[persistBlocks]: error to persist block:", err.Error())
		return
	}

	// PersistCompleted event
	//ledger.Blockchain.BlockHeight = block.Blockdata.Height
	DefaultLedger.Blockchain.UpdateBestHeight(block.Header.Height)
	c.mu.Lock()
	c.currentBlockHeight = block.Header.Height
	c.mu.Unlock()

	DefaultLedger.Blockchain.BCEvents.Notify(events.EventBlockPersistCompleted, block)
	//log.Tracef("The latest block height:%d, block hash: %x", block.Blockdata.Height, hash)
	//}

}

func (c *ChainStore) BlockInCache(hash Uint256) bool {
	//TODO mutex
	//_, ok := c.ledger.Blockchain.Index[hash]
	//return ok
	return false
}

func (c *ChainStore) GetUnspent(txid Uint256, index uint16) (*Output, error) {
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

func (c *ChainStore) RemoveHeaderListElement(hash Uint256) {
	for e := c.headerIdx.Front(); e != nil; e = e.Next() {
		n := e.Value.(Header)
		h := n.Hash()
		if h.IsEqual(hash) {
			c.headerIdx.Remove(e)
		}
	}
}

func (c *ChainStore) GetHeight() uint32 {
	c.mu.RLock()
	defer c.mu.RUnlock()

	return c.currentBlockHeight
}

func (c *ChainStore) IsBlockInStore(hash Uint256) bool {
	var b = new(Block)
	prefix := []byte{byte(DATA_Header)}
	blockData, err := c.Get(append(prefix, hash.Bytes()...))
	if err != nil {
		return false
	}

	r := bytes.NewReader(blockData)

	// first 8 bytes is sys_fee
	_, err = ReadUint64(r)
	if err != nil {
		return false
	}

	// Deserialize block data
	err = b.FromTrimmedData(r)
	if err != nil {
		return false
	}

	if b.Header.Height > c.currentBlockHeight {
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

func (c *ChainStore) GetUnspentFromProgramHash(programHash Uint168, assetid Uint256) ([]*UTXO, error) {
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
func (c *ChainStore) GetUnspentsFromProgramHash(programHash Uint168) (map[Uint256][]*UTXO, error) {
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

func (c *ChainStore) PersistUnspentWithProgramHash(programHash Uint168, assetid Uint256, height uint32, unspents []*UTXO) error {
	prefix := []byte{byte(IX_Unspent_UTXO)}
	prefix = append(prefix, programHash.Bytes()...)
	prefix = append(prefix, assetid.Bytes()...)
	key := bytes.NewBuffer(prefix)
	if err := WriteUint32(key, height); err != nil {
		return err
	}

	if len(unspents) == 0 {
		c.BatchDelete(key.Bytes())
		return nil
	}

	listnum := len(unspents)
	w := new(bytes.Buffer)
	WriteVarUint(w, uint64(listnum))
	for i := 0; i < listnum; i++ {
		unspents[i].Serialize(w)
	}

	// BATCH PUT VALUE
	c.BatchPut(key.Bytes(), w.Bytes())
	return nil
}

func (c *ChainStore) GetAssets() map[Uint256]*Asset {
	assets := make(map[Uint256]*Asset)

	iter := c.NewIterator([]byte{byte(ST_Info)})
	for iter.Next() {
		rk := bytes.NewReader(iter.Key())

		// read prefix
		_, _ = ReadBytes(rk, 1)
		var assetid Uint256
		assetid.Deserialize(rk)
		log.Tracef("[GetAssets] assetid: %x", assetid.Bytes())

		asset := new(Asset)
		r := bytes.NewReader(iter.Value())
		asset.Deserialize(r)

		assets[assetid] = asset
	}

	return assets
}
