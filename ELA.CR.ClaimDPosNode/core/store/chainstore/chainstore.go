package chainstore

import (
	"fmt"
	"sync"
	"time"
	"bytes"
	"errors"
	"container/list"

	"Elastos.ELA/events"
	. "Elastos.ELA/common"
	"Elastos.ELA/common/log"
	. "Elastos.ELA/core/asset"
	. "Elastos.ELA/core/store"
	. "Elastos.ELA/core/ledger"
	tx "Elastos.ELA/core/transaction"
	"Elastos.ELA/common/serialize"
	. "Elastos.ELA/core/store/leveldbstore"
)

const TaskChanCap = 4

var (
	ErrDBNotFound = errors.New("leveldb: not found")
	zeroHash      = Uint256{}
)

type persistTask interface{}

type rollbackBlockTask struct {
	blockHash Uint256
	reply     chan bool
}
type persistBlockTask struct {
	block  *Block
	ledger *Ledger
	reply  chan bool
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
	ledger             *Ledger
}

func NewLedgerStore() (ILedgerStore, error) {
	// TODO: read config file decide which db to use.
	st, err := NewLevelDBStore("Chain")
	if err != nil {
		return nil, err
	}

	chain := &ChainStore{
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

	go chain.loop()

	return chain, nil
}

func (self *ChainStore) Close() {
	closed := make(chan bool)
	self.quit <- closed
	<-closed

	self.Close()
}

func (self *ChainStore) loop() {
	for {
		select {
		case t := <-self.taskCh:
			now := time.Now()
			switch task := t.(type) {
			case *persistBlockTask:
				self.handlePersistBlockTask(task.block, task.ledger)
				task.reply <- true
				tcall := float64(time.Now().Sub(now)) / float64(time.Second)
				log.Debugf("handle block exetime: %g num transactions:%d \n", tcall, len(task.block.Transactions))
			case *rollbackBlockTask:
				self.handleRollbackBlockTask(task.blockHash)
				task.reply <- true
				tcall := float64(time.Now().Sub(now)) / float64(time.Second)
				log.Debugf("handle block rollback exetime: %g \n", tcall)
			}

		case closed := <-self.quit:
			closed <- true
			return
		}
	}
}

// can only be invoked by backend write goroutine
func (self *ChainStore) clearCache(b *Block) {
	self.mu.Lock()
	defer self.mu.Unlock()

	for e := self.headerIdx.Front(); e != nil; e = e.Next() {
		n := e.Value.(Header)
		h := n.Hash()
		if h.IsEqual(b.Hash()) {
			self.headerIdx.Remove(e)
		}
	}
}

func (bd *ChainStore) InitLedgerStoreWithGenesisBlock(genesisBlock *Block) (uint32, error) {
	prefix := []byte{byte(CFG_Version)}
	version, err := bd.Get(prefix)
	if err != nil {
		version = []byte{0x00}
	}

	if version[0] == 0x00 {
		// batch delete old data
		bd.NewBatch()
		iter := bd.NewIterator(nil)
		for iter.Next() {
			bd.BatchDelete(iter.Key())
		}
		iter.Release()

		err := bd.BatchCommit()
		if err != nil {
			return 0, err
		}

		// persist genesis block
		bd.persist(genesisBlock)

		// put version to db
		err = bd.Put(prefix, []byte{0x01})
		if err != nil {
			return 0, err
		}

	}

	// GenesisBlock should exist in chain
	// Or the bookkeepers are not consistent with the chain
	hash := genesisBlock.Hash()
	if !bd.IsBlockInStore(hash) {
		return 0, errors.New("genesis block is not consistent with the chain")
	}
	bd.ledger.Blockchain.GenesisHash = hash
	//bd.headerIndex[0] = hash

	// Get Current Block
	currentBlockPrefix := []byte{byte(SYS_CurrentBlock)}
	data, err := bd.Get(currentBlockPrefix)
	if err != nil {
		return 0, err
	}

	r := bytes.NewReader(data)
	var blockHash Uint256
	blockHash.Deserialize(r)
	bd.currentBlockHeight, err = serialize.ReadUint32(r)
	endHeight := bd.currentBlockHeight

	startHeight := uint32(0)
	if endHeight > MinMemoryNodes {
		startHeight = endHeight - MinMemoryNodes
	}

	for start := startHeight; start <= endHeight; start++ {
		hash, err := bd.GetBlockHash(start)
		if err != nil {
			return 0, err
		}
		header, err := bd.GetHeader(hash)
		if err != nil {
			return 0, err
		}
		node, err := bd.ledger.Blockchain.LoadBlockNode(header, &hash)
		if err != nil {
			return 0, err
		}

		// This node is now the end of the best chain.
		bd.ledger.Blockchain.BestChain = node

	}
	//bd.ledger.Blockchain.DumpState()

	return bd.currentBlockHeight, nil

}

func (bd *ChainStore) InitLedgerStore(l *Ledger) error {
	// TODO: InitLedgerStore
	bd.ledger = l
	return nil
}

func (bd *ChainStore) IsTxHashDuplicate(txhash Uint256) bool {
	prefix := []byte{byte(DATA_Transaction)}
	_, err_get := bd.Get(append(prefix, txhash.Bytes()...))
	if err_get != nil {
		return false
	} else {
		return true
	}
}

func (bd *ChainStore) IsDoubleSpend(txn *tx.Transaction) bool {
	if len(txn.UTXOInputs) == 0 {
		return false
	}

	unspentPrefix := []byte{byte(IX_Unspent)}
	for i := 0; i < len(txn.UTXOInputs); i++ {
		txhash := txn.UTXOInputs[i].ReferTxID
		unspentValue, err_get := bd.Get(append(unspentPrefix, txhash.Bytes()...))
		if err_get != nil {
			return true
		}

		unspents, _ := GetUint16Array(unspentValue)
		findFlag := false
		for k := 0; k < len(unspents); k++ {
			if unspents[k] == txn.UTXOInputs[i].ReferTxOutputIndex {
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

func (bd *ChainStore) GetBlockHash(height uint32) (Uint256, error) {
	queryKey := bytes.NewBuffer(nil)
	queryKey.WriteByte(byte(DATA_BlockHash))
	err := serialize.WriteUint32(queryKey, height)

	if err != nil {
		return Uint256{}, err
	}
	blockHash, err := bd.Get(queryKey.Bytes())
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

func (bd *ChainStore) getHeaderWithCache(hash Uint256) *Header {
	for e := bd.headerIdx.Front(); e != nil; e = e.Next() {
		n := e.Value.(Header)
		eh := n.Hash()
		if eh.IsEqual(hash) {
			return &n
		}
	}

	h, _ := bd.GetHeader(hash)

	return h
}

func (bd *ChainStore) GetCurrentBlockHash() Uint256 {
	hash, err := bd.GetBlockHash(bd.currentBlockHeight)
	if err != nil {
		return Uint256{}
	}

	return hash
}

func (db *ChainStore) RollbackBlock(blockHash Uint256) error {

	reply := make(chan bool)
	db.taskCh <- &rollbackBlockTask{blockHash: blockHash, reply: reply}
	<-reply

	return nil
}

func (bd *ChainStore) GetHeader(hash Uint256) (*Header, error) {
	var h = new(Header)
	h = new(Header)

	prefix := []byte{byte(DATA_Header)}
	data, err_get := bd.Get(append(prefix, hash.Bytes()...))
	//log.Debug( "Get Header Data: %x\n",  data )
	if err_get != nil {
		//TODO: implement error process
		return nil, err_get
	}

	r := bytes.NewReader(data)
	// first 8 bytes is sys_fee
	sysfee, err := serialize.ReadUint64(r)
	if err != nil {
		return nil, err
	}
	_ = sysfee

	// Deserialize block data
	err = h.Deserialize(r)
	if err != nil {
		return nil, err
	}

	return h, err
}

func (bd *ChainStore) PersistAsset(assetId Uint256, asset *Asset) error {
	w := bytes.NewBuffer(nil)

	asset.Serialize(w)

	// generate key
	assetKey := bytes.NewBuffer(nil)
	// add asset prefix.
	assetKey.WriteByte(byte(ST_Info))
	// contact asset id
	assetId.Serialize(assetKey)

	log.Debug(fmt.Sprintf("asset key: %x\n", assetKey))

	// PUT VALUE
	err := bd.BatchPut(assetKey.Bytes(), w.Bytes())
	if err != nil {
		return err
	}

	return nil
}

func (bd *ChainStore) GetAsset(hash Uint256) (*Asset, error) {
	log.Debug(fmt.Sprintf("GetAsset Hash: %x\n", hash))

	asset := new(Asset)

	prefix := []byte{byte(ST_Info)}
	data, err_get := bd.Get(append(prefix, hash.Bytes()...))

	log.Debug(fmt.Sprintf("GetAsset Data: %x\n", data))
	if err_get != nil {
		//TODO: implement error process
		return nil, err_get
	}

	r := bytes.NewReader(data)
	asset.Deserialize(r)

	return asset, nil
}

func (bd *ChainStore) GetTransaction(hash Uint256) (*tx.Transaction, uint32, error) {
	key := append([]byte{byte(DATA_Transaction)}, hash.Bytes()...)
	value, err := bd.Get(key)
	if err != nil {
		return nil, 0, err
	}

	r := bytes.NewReader(value)
	height, err := serialize.ReadUint32(r)
	if err != nil {
		return nil, 0, err
	}

	var txn tx.Transaction
	if err := txn.Deserialize(r); err != nil {
		return nil, height, err
	}

	return &txn, height, nil
}

func (bd *ChainStore) PersistTransaction(tx *tx.Transaction, height uint32) error {
	//////////////////////////////////////////////////////////////
	// generate key with DATA_Transaction prefix
	txhash := bytes.NewBuffer(nil)
	// add transaction header prefix.
	txhash.WriteByte(byte(DATA_Transaction))
	// get transaction hash
	txHashValue := tx.Hash()
	txHashValue.Serialize(txhash)
	log.Debug(fmt.Sprintf("transaction header + hash: %x\n", txhash))

	// generate value
	w := bytes.NewBuffer(nil)
	serialize.WriteUint32(w, height)
	tx.Serialize(w)
	log.Debug(fmt.Sprintf("transaction tx data: %x\n", w))

	// put value
	err := bd.BatchPut(txhash.Bytes(), w.Bytes())
	if err != nil {
		return err
	}

	return nil
}

func (bd *ChainStore) GetBlock(hash Uint256) (*Block, error) {
	var b = new(Block)
	b.Header = new(Header)

	prefix := []byte{byte(DATA_Header)}
	bHash, err := bd.Get(append(prefix, hash.Bytes()...))
	if err != nil {
		return nil, err
	}

	r := bytes.NewReader(bHash)

	// first 8 bytes is sys_fee
	_, err = serialize.ReadUint64(r)
	if err != nil {
		return nil, err
	}

	// Deserialize block data
	if err := b.FromTrimmedData(r); err != nil {
		return nil, err
	}

	// Deserialize transaction
	for i, txn := range b.Transactions {
		tmp, _, err := bd.GetTransaction(txn.Hash())
		if err != nil {
			return nil, err
		}
		b.Transactions[i] = tmp
	}

	return b, nil
}

func (db *ChainStore) rollback(b *Block) error {
	db.BatchInit()
	db.RollbackTrimemedBlock(b)
	db.RollbackBlockHash(b)
	db.RollbackTransactions(b)
	db.RollbackUnspendUTXOs(b)
	db.RollbackUnspend(b)
	db.RollbackCurrentBlock(b)
	db.BatchFinish()

	db.ledger.Blockchain.UpdateBestHeight(b.Header.Height - 1)
	db.mu.Lock()
	db.currentBlockHeight = b.Header.Height - 1
	db.mu.Unlock()

	db.ledger.Blockchain.BCEvents.Notify(events.EventRollbackTransaction, b)

	return nil
}

func (db *ChainStore) persist(b *Block) error {
	//unspents := make(map[Uint256][]uint16)

	db.BatchInit()
	db.PersistTrimmedBlock(b)
	db.PersistBlockHash(b)
	db.PersistTransactions(b)
	db.PersistUnspendUTXOs(b)
	db.PersistUnspend(b)
	db.PersistCurrentBlock(b)
	db.BatchFinish()

	return nil
}

// can only be invoked by backend write goroutine
func (bd *ChainStore) addHeader(header *Header) {

	log.Debugf("addHeader(), Height=%d\n", header.Height)

	hash := header.Hash()

	bd.mu.Lock()
	bd.headerCache[header.Hash()] = header
	bd.headerIndex[header.Height] = hash
	bd.headerIdx.PushBack(*header)
	bd.mu.Unlock()

	log.Debug("[addHeader]: finish, header height:", header.Height)
}

func (self *ChainStore) SaveBlock(b *Block, ledger *Ledger) error {
	log.Debug("SaveBlock()")
	//log.Trace("validation.PowVerifyBlock(b, ledger, false)")
	//err := validation.PowVerifyBlock(b, ledger, false)
	//if err != nil {
	//	log.Error("PowVerifyBlock error!")
	//	return err
	//}
	//log.Trace("validation.PowVerifyBlock(b, ledger, false)222222")

	reply := make(chan bool)
	self.taskCh <- &persistBlockTask{block: b, ledger: ledger, reply: reply}
	<-reply

	return nil
}

func (db *ChainStore) handleRollbackBlockTask(blockHash Uint256) {
	block, err := db.GetBlock(blockHash)
	if err != nil {
		log.Errorf("block %x can't be found", BytesToHexString(blockHash.Bytes()))
		return
	}
	db.rollback(block)
}

func (self *ChainStore) handlePersistBlockTask(b *Block, ledger *Ledger) {

	if b.Header.Height <= self.currentBlockHeight {
		return
	}

	//	self.mu.Lock()
	//self.blockCache[b.Hash()] = b
	//self.mu.Unlock()

	//log.Trace(b.Header.Height)
	//log.Trace(b.Header)
	//log.Trace(b.Transactions[0])
	//if b.Header.Height < uint32(len(self.headerIndex)) {
	self.persistBlocks(b, ledger)

	//self.NewBatch()
	//storedHeaderCount := self.storedHeaderCount
	//for self.currentBlockHeight-storedHeaderCount >= HeaderHashListCount {
	//	hashBuffer := new(bytes.Buffer)
	//	serialize.WriteVarUint(hashBuffer, uint64(HeaderHashListCount))
	//	var hashArray []byte
	//	for i := 0; i < HeaderHashListCount; i++ {
	//		index := storedHeaderCount + uint32(i)
	//		thash := self.headerIndex[index]
	//		thehash := thash.ToArray()
	//		hashArray = append(hashArray, thehash...)
	//	}
	//	hashBuffer.Write(hashArray)

	//	hhlPrefix := bytes.NewBuffer(nil)
	//	hhlPrefix.WriteByte(byte(IX_HeaderHashList))
	//	serialize.WriteUint32(hhlPrefix, storedHeaderCount)

	//	self.BatchPut(hhlPrefix.Bytes(), hashBuffer.Bytes())
	//	storedHeaderCount += HeaderHashListCount
	//}

	//err := self.BatchCommit()
	//if err != nil {
	//	log.Error("failed to persist header hash list:", err)
	//	return
	//}
	//self.mu.Lock()
	//self.storedHeaderCount = storedHeaderCount
	//self.mu.Unlock()
	self.clearCache(b)
	//}
}

func (bd *ChainStore) persistBlocks(block *Block, ledger *Ledger) {
	//stopHeight := uint32(len(bd.headerIndex))
	//for h := bd.currentBlockHeight + 1; h <= stopHeight; h++ {
	//hash := bd.headerIndex[h]
	//block, ok := bd.blockCache[hash]
	//if !ok {
	//	break
	//}
	//log.Trace(block.Header)
	//log.Trace(block.Transactions[0])
	err := bd.persist(block)
	if err != nil {
		log.Fatal("[persistBlocks]: error to persist block:", err.Error())
		return
	}

	// PersistCompleted event
	//ledger.Blockchain.BlockHeight = block.Header.Height
	ledger.Blockchain.UpdateBestHeight(block.Header.Height)
	bd.mu.Lock()
	bd.currentBlockHeight = block.Header.Height
	bd.mu.Unlock()

	ledger.Blockchain.BCEvents.Notify(events.EventBlockPersistCompleted, block)
	//log.Tracef("The latest block height:%d, block hash: %x", block.Header.Height, hash)
	//}

}

func (bd *ChainStore) BlockInCache(hash Uint256) bool {
	//TODO mutex
	//_, ok := bd.ledger.Blockchain.Index[hash]
	//return ok
	return false
}

func (bd *ChainStore) GetUnspent(txid Uint256, index uint16) (*tx.TxOutput, error) {
	if ok, _ := bd.ContainsUnspent(txid, index); ok {
		Tx, _, err := bd.GetTransaction(txid)
		if err != nil {
			return nil, err
		}

		return Tx.Outputs[index], nil
	}

	return nil, errors.New("[GetUnspent] NOT ContainsUnspent.")
}

func (bd *ChainStore) ContainsUnspent(txid Uint256, index uint16) (bool, error) {
	unspentPrefix := []byte{byte(IX_Unspent)}
	unspentValue, err_get := bd.Get(append(unspentPrefix, txid.Bytes()...))

	if err_get != nil {
		return false, err_get
	}

	unspentArray, err_get := GetUint16Array(unspentValue)
	if err_get != nil {
		return false, err_get
	}

	for i := 0; i < len(unspentArray); i++ {
		if unspentArray[i] == index {
			return true, nil
		}
	}

	return false, nil
}

func (bd *ChainStore) RemoveHeaderListElement(hash Uint256) {
	for e := bd.headerIdx.Front(); e != nil; e = e.Next() {
		n := e.Value.(Header)
		h := n.Hash()
		if h.IsEqual(hash) {
			bd.headerIdx.Remove(e)
		}
	}
}

func (bd *ChainStore) GetHeight() uint32 {
	bd.mu.RLock()
	defer bd.mu.RUnlock()

	return bd.currentBlockHeight
}

func (bd *ChainStore) IsBlockInStore(hash Uint256) bool {
	var b *Block = new(Block)
	b.Header = new(Header)
	prefix := []byte{byte(DATA_Header)}
	blockData, err_get := bd.Get(append(prefix, hash.Bytes()...))
	if err_get != nil {
		return false
	}

	r := bytes.NewReader(blockData)

	// first 8 bytes is sys_fee
	_, err := serialize.ReadUint64(r)
	if err != nil {
		return false
	}

	// Deserialize block data
	err = b.FromTrimmedData(r)
	if err != nil {
		return false
	}

	if b.Header.Height > bd.currentBlockHeight {
		return false
	}

	return true
}

func (bd *ChainStore) GetUnspentElementFromProgramHash(programHash Uint168, assetid Uint256, height uint32) ([]*tx.UTXOUnspent, error) {
	prefix := []byte{byte(IX_Unspent_UTXO)}
	prefix = append(prefix, programHash.Bytes()...)
	prefix = append(prefix, assetid.Bytes()...)

	key := bytes.NewBuffer(prefix)
	if err := serialize.WriteUint32(key, height); err != nil {
		return nil, err
	}
	unspentsData, err := bd.Get(key.Bytes())
	if err != nil {
		return nil, err
	}
	r := bytes.NewReader(unspentsData)
	listNum, err := serialize.ReadVarUint(r, 0)
	if err != nil {
		return nil, err
	}

	// read unspent list in store
	unspents := make([]*tx.UTXOUnspent, listNum)
	for i := 0; i < int(listNum); i++ {
		uu := new(tx.UTXOUnspent)
		err := uu.Deserialize(r)
		if err != nil {
			return nil, err
		}

		unspents[i] = uu
	}

	return unspents, nil
}

func (bd *ChainStore) GetUnspentFromProgramHash(programHash Uint168, assetid Uint256) ([]*tx.UTXOUnspent, error) {
	unspents := make([]*tx.UTXOUnspent, 0)

	key := []byte{byte(IX_Unspent_UTXO)}
	key = append(key, programHash.Bytes()...)
	key = append(key, assetid.Bytes()...)
	iter := bd.NewIterator(key)
	for iter.Next() {
		r := bytes.NewReader(iter.Value())
		listNum, err := serialize.ReadVarUint(r, 0)
		if err != nil {
			return nil, err
		}

		for i := 0; i < int(listNum); i++ {
			uu := new(tx.UTXOUnspent)
			err := uu.Deserialize(r)
			if err != nil {
				return nil, err
			}

			unspents = append(unspents, uu)
		}

	}

	return unspents, nil

}
func (bd *ChainStore) GetUnspentsFromProgramHash(programHash Uint168) (map[Uint256][]*tx.UTXOUnspent, error) {
	uxtoUnspents := make(map[Uint256][]*tx.UTXOUnspent)

	prefix := []byte{byte(IX_Unspent_UTXO)}
	key := append(prefix, programHash.Bytes()...)
	iter := bd.NewIterator(key)
	for iter.Next() {
		rk := bytes.NewReader(iter.Key())

		// read prefix
		_, _ = serialize.ReadBytes(rk, 1)
		var ph Uint168
		ph.Deserialize(rk)
		var assetid Uint256
		assetid.Deserialize(rk)

		r := bytes.NewReader(iter.Value())
		listNum, err := serialize.ReadVarUint(r, 0)
		if err != nil {
			return nil, err
		}

		// read unspent list in store
		unspents := make([]*tx.UTXOUnspent, listNum)
		for i := 0; i < int(listNum); i++ {
			uu := new(tx.UTXOUnspent)
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

func (bd *ChainStore) PersistUnspentWithProgramHash(programHash Uint168, assetid Uint256, height uint32, unspents []*tx.UTXOUnspent) error {
	prefix := []byte{byte(IX_Unspent_UTXO)}
	prefix = append(prefix, programHash.Bytes()...)
	prefix = append(prefix, assetid.Bytes()...)
	key := bytes.NewBuffer(prefix)
	if err := serialize.WriteUint32(key, height); err != nil {
		return err
	}

	if len(unspents) == 0 {
		bd.BatchDelete(key.Bytes())
		return nil
	}

	listnum := len(unspents)
	w := bytes.NewBuffer(nil)
	serialize.WriteVarUint(w, uint64(listnum))
	for i := 0; i < listnum; i++ {
		unspents[i].Serialize(w)
	}

	// BATCH PUT VALUE
	if err := bd.BatchPut(key.Bytes(), w.Bytes()); err != nil {
		return err
	}

	return nil
}

func (bd *ChainStore) GetAssets() map[Uint256]*Asset {
	assets := make(map[Uint256]*Asset)

	iter := bd.NewIterator([]byte{byte(ST_Info)})
	for iter.Next() {
		rk := bytes.NewReader(iter.Key())

		// read prefix
		_, _ = serialize.ReadBytes(rk, 1)
		var assetid Uint256
		assetid.Deserialize(rk)
		log.Tracef("[GetAssets] assetid: %x\n", assetid.Bytes())

		asset := new(Asset)
		r := bytes.NewReader(iter.Value())
		asset.Deserialize(r)

		assets[assetid] = asset
	}

	return assets
}
