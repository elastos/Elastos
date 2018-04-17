package ChainStore

import (
	"fmt"
	"sync"
	"time"
	"bytes"
	"errors"
	"container/list"

	. "github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/events"
	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/log"
	. "github.com/elastos/Elastos.ELA.Utility/core/asset"
	. "github.com/elastos/Elastos.ELA/store"
	. "github.com/elastos/Elastos.ELA.Utility/core/ledger"
	tx "github.com/elastos/Elastos.ELA.Utility/core/transaction"
	. "github.com/elastos/Elastos.ELA/store/leveldb"
	"github.com/elastos/Elastos.ELA.Utility/common/serialize"
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
	st, err := NewLevelDB("Chain")
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

func (c *ChainStore) Close() {
	closed := make(chan bool)
	c.quit <- closed
	<-closed

	c.Close()
}

func (c *ChainStore) loop() {
	for {
		select {
		case t := <-c.taskCh:
			now := time.Now()
			switch task := t.(type) {
			case *persistBlockTask:
				c.handlePersistBlockTask(task.block, task.ledger)
				task.reply <- true
				tcall := float64(time.Now().Sub(now)) / float64(time.Second)
				log.Debugf("handle block exetime: %g num transactions:%d \n", tcall, len(task.block.Transactions))
			case *rollbackBlockTask:
				c.handleRollbackBlockTask(task.blockHash)
				task.reply <- true
				tcall := float64(time.Now().Sub(now)) / float64(time.Second)
				log.Debugf("handle block rollback exetime: %g \n", tcall)
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

func (c *ChainStore) InitLedgerStoreWithGenesisBlock(genesisBlock *Block) (uint32, error) {
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
		c.persist(genesisBlock)

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
	c.ledger.Blockchain.GenesisHash = hash
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
	c.currentBlockHeight, err = serialize.ReadUint32(r)
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
		node, err := c.ledger.Blockchain.LoadBlockNode(header, &hash)
		if err != nil {
			return 0, err
		}

		// This node is now the end of the best chain.
		c.ledger.Blockchain.BestChain = node

	}
	//c.ledger.Blockchain.DumpState()

	return c.currentBlockHeight, nil

}

func (c *ChainStore) InitLedgerStore(l *Ledger) error {
	// TODO: InitLedgerStore
	c.ledger = l
	return nil
}

func (c *ChainStore) IsTxHashDuplicate(txhash Uint256) bool {
	prefix := []byte{byte(DATA_Transaction)}
	_, err_get := c.Get(append(prefix, txhash.Bytes()...))
	if err_get != nil {
		return false
	} else {
		return true
	}
}

func (c *ChainStore) IsDoubleSpend(txn *tx.Transaction) bool {
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

func (c *ChainStore) GetBlockHash(height uint32) (Uint256, error) {
	queryKey := bytes.NewBuffer(nil)
	queryKey.WriteByte(byte(DATA_BlockHash))
	err := serialize.WriteUint32(queryKey, height)

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
	data, err_get := c.Get(append(prefix, hash.Bytes()...))
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

func (c *ChainStore) PersistAsset(assetId Uint256, asset *Asset) error {
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
	err := c.BatchPut(assetKey.Bytes(), w.Bytes())
	if err != nil {
		return err
	}

	return nil
}

func (c *ChainStore) GetAsset(hash Uint256) (*Asset, error) {
	log.Debug(fmt.Sprintf("GetAsset Hash: %x\n", hash))

	asset := new(Asset)

	prefix := []byte{byte(ST_Info)}
	data, err_get := c.Get(append(prefix, hash.Bytes()...))

	log.Debug(fmt.Sprintf("GetAsset Data: %x\n", data))
	if err_get != nil {
		//TODO: implement error process
		return nil, err_get
	}

	r := bytes.NewReader(data)
	asset.Deserialize(r)

	return asset, nil
}

func (c *ChainStore) GetTransaction(hash Uint256) (*tx.Transaction, uint32, error) {
	key := append([]byte{byte(DATA_Transaction)}, hash.Bytes()...)
	value, err := c.Get(key)
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

func (c *ChainStore) PersistTransaction(tx *tx.Transaction, height uint32) error {
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
	err := c.BatchPut(txhash.Bytes(), w.Bytes())
	if err != nil {
		return err
	}

	return nil
}

func (c *ChainStore) GetBlock(hash Uint256) (*Block, error) {
	var b = new(Block)

	b.Header = new(Header)

	prefix := []byte{byte(DATA_Header)}
	bHash, err := c.Get(append(prefix, hash.Bytes()...))
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
		tmp, _, err := c.GetTransaction(txn.Hash())
		if err != nil {
			return nil, err
		}
		b.Transactions[i] = tmp
	}

	return b, nil
}

func (c *ChainStore) rollback(b *Block) error {
	c.BatchInit()
	c.RollbackTrimemedBlock(b)
	c.RollbackBlockHash(b)
	c.RollbackTransactions(b)
	c.RollbackUnspendUTXOs(b)
	c.RollbackUnspend(b)
	c.RollbackCurrentBlock(b)
	c.BatchFinish()

	c.ledger.Blockchain.UpdateBestHeight(b.Header.Height - 1)
	c.mu.Lock()
	c.currentBlockHeight = b.Header.Height - 1
	c.mu.Unlock()

	c.ledger.Blockchain.BCEvents.Notify(events.EventRollbackTransaction, b)

	return nil
}

func (c *ChainStore) persist(b *Block) error {
	//unspents := make(map[Uint256][]uint16)

	c.BatchInit()
	c.PersistTrimmedBlock(b)
	c.PersistBlockHash(b)
	c.PersistTransactions(b)
	c.PersistUnspendUTXOs(b)
	c.PersistUnspend(b)
	c.PersistCurrentBlock(b)
	c.BatchFinish()

	return nil
}

// can only be invoked by backend write goroutine
func (c *ChainStore) addHeader(header *Header) {

	log.Debugf("addHeader(), Height=%d\n", header.Height)

	hash := header.Hash()

	c.mu.Lock()
	c.headerCache[header.Hash()] = header
	c.headerIndex[header.Height] = hash
	c.headerIdx.PushBack(*header)
	c.mu.Unlock()

	log.Debug("[addHeader]: finish, header height:", header.Height)
}

func (c *ChainStore) SaveBlock(b *Block, ledger *Ledger) error {
	log.Debug("SaveBlock()")
	//log.Trace("validation.PowVerifyBlock(b, ledger, false)")
	//err := validation.PowVerifyBlock(b, ledger, false)
	//if err != nil {
	//	log.Error("PowVerifyBlock error!")
	//	return err
	//}
	//log.Trace("validation.PowVerifyBlock(b, ledger, false)222222")

	reply := make(chan bool)
	c.taskCh <- &persistBlockTask{block: b, ledger: ledger, reply: reply}
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

func (c *ChainStore) handlePersistBlockTask(b *Block, ledger *Ledger) {

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
	c.persistBlocks(b, ledger)

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

	//	hhlPrefix := bytes.NewBuffer(nil)
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

func (c *ChainStore) persistBlocks(block *Block, ledger *Ledger) {
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
	ledger.Blockchain.UpdateBestHeight(block.Header.Height)
	c.mu.Lock()
	c.currentBlockHeight = block.Header.Height
	c.mu.Unlock()

	ledger.Blockchain.BCEvents.Notify(events.EventBlockPersistCompleted, block)
	//log.Tracef("The latest block height:%d, block hash: %x", block.Blockdata.Height, hash)
	//}

}

func (c *ChainStore) BlockInCache(hash Uint256) bool {
	//TODO mutex
	//_, ok := c.ledger.Blockchain.Index[hash]
	//return ok
	return false
}

func (c *ChainStore) GetUnspent(txid Uint256, index uint16) (*tx.Output, error) {
	if ok, _ := c.ContainsUnspent(txid, index); ok {
		Tx, _, err := c.GetTransaction(txid)
		if err != nil {
			return nil, err
		}

		return Tx.Outputs[index], nil
	}

	return nil, errors.New("[GetUnspent] NOT ContainsUnspent.")
}

func (c *ChainStore) ContainsUnspent(txid Uint256, index uint16) (bool, error) {
	unspentPrefix := []byte{byte(IX_Unspent)}
	unspentValue, err_get := c.Get(append(unspentPrefix, txid.Bytes()...))

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
	b.Header = new(Header)
	prefix := []byte{byte(DATA_Header)}
	blockData, err_get := c.Get(append(prefix, hash.Bytes()...))
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

	if b.Header.Height > c.currentBlockHeight {
		return false
	}

	return true
}

func (c *ChainStore) GetUnspentElementFromProgramHash(programHash Uint168, assetid Uint256, height uint32) ([]*tx.UTXOUnspent, error) {
	prefix := []byte{byte(IX_Unspent_UTXO)}
	prefix = append(prefix, programHash.Bytes()...)
	prefix = append(prefix, assetid.Bytes()...)

	key := bytes.NewBuffer(prefix)
	if err := serialize.WriteUint32(key, height); err != nil {
		return nil, err
	}
	unspentsData, err := c.Get(key.Bytes())
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

func (c *ChainStore) GetUnspentFromProgramHash(programHash Uint168, assetid Uint256) ([]*tx.UTXOUnspent, error) {
	unspents := make([]*tx.UTXOUnspent, 0)

	key := []byte{byte(IX_Unspent_UTXO)}
	key = append(key, programHash.Bytes()...)
	key = append(key, assetid.Bytes()...)
	iter := c.NewIterator(key)
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
func (c *ChainStore) GetUnspentsFromProgramHash(programHash Uint168) (map[Uint256][]*tx.UTXOUnspent, error) {
	uxtoUnspents := make(map[Uint256][]*tx.UTXOUnspent)

	prefix := []byte{byte(IX_Unspent_UTXO)}
	key := append(prefix, programHash.Bytes()...)
	iter := c.NewIterator(key)
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

func (c *ChainStore) PersistUnspentWithProgramHash(programHash Uint168, assetid Uint256, height uint32, unspents []*tx.UTXOUnspent) error {
	prefix := []byte{byte(IX_Unspent_UTXO)}
	prefix = append(prefix, programHash.Bytes()...)
	prefix = append(prefix, assetid.Bytes()...)
	key := bytes.NewBuffer(prefix)
	if err := serialize.WriteUint32(key, height); err != nil {
		return err
	}

	if len(unspents) == 0 {
		c.BatchDelete(key.Bytes())
		return nil
	}

	listnum := len(unspents)
	w := bytes.NewBuffer(nil)
	serialize.WriteVarUint(w, uint64(listnum))
	for i := 0; i < listnum; i++ {
		unspents[i].Serialize(w)
	}

	// BATCH PUT VALUE
	if err := c.BatchPut(key.Bytes(), w.Bytes()); err != nil {
		return err
	}

	return nil
}

func (c *ChainStore) GetAssets() map[Uint256]*Asset {
	assets := make(map[Uint256]*Asset)

	iter := c.NewIterator([]byte{byte(ST_Info)})
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
