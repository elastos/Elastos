package blockchain

import (
	"bytes"
	"container/list"
	"errors"
	"fmt"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA.SideChain/core"

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

	persistFunctions  map[StoreFuncName]func(batch IBatch, b *core.Block) error
	rollbackFunctions map[StoreFuncName]func(batch IBatch, b *core.Block) error
}

func NewChainStore(genesisBlock *core.Block) (*ChainStore, error) {
	// TODO: read config file decide which db to use.
	st, err := NewLevelDB("Chain")
	if err != nil {
		return nil, err
	}

	s := ChainStore{
		IStore:             st,
		headerIndex:        map[uint32]Uint256{},
		headerCache:        map[Uint256]*core.Header{},
		headerIdx:          list.New(),
		currentBlockHeight: 0,
		storedHeaderCount:  0,
		taskCh:             make(chan persistTask, TaskChanCap),
		quit:               make(chan chan bool, 1),
	}

	s.initWithGenesisBlock(genesisBlock)

	s.RegisterFunctions(true, StoreFuncNames.PersistTrimmedBlock, s.persistTrimmedBlock)
	s.RegisterFunctions(true, StoreFuncNames.PersistBlockHash, s.persistBlockHash)
	s.RegisterFunctions(true, StoreFuncNames.PersistCurrentBlock, s.persistCurrentBlock)
	s.RegisterFunctions(true, StoreFuncNames.PersistCurrentBlock, s.persistCurrentBlock)
	s.RegisterFunctions(true, StoreFuncNames.PersistTransactions, s.persistTransactions)
	s.RegisterFunctions(true, StoreFuncNames.PersistUnspend, s.persistUnspend)

	s.RegisterFunctions(false, StoreFuncNames.RollbackTrimmedBlock, s.rollbackTrimmedBlock)
	s.RegisterFunctions(false, StoreFuncNames.RollbackBlockHash, s.rollbackBlockHash)
	s.RegisterFunctions(false, StoreFuncNames.RollbackCurrentBlock, s.rollbackCurrentBlock)
	s.RegisterFunctions(false, StoreFuncNames.RollbackUnspendUTXOs, s.rollbackUnspendUTXOs)
	s.RegisterFunctions(false, StoreFuncNames.RollbackTransactions, s.rollbackTransactions)
	s.RegisterFunctions(false, StoreFuncNames.RollbackUnspend, s.rollbackUnspend)

	go s.taskHandler()

	return &s, nil
}

func (s *ChainStore) RegisterFunctions(isPersist bool, name StoreFuncName, function func(batch IBatch, b *core.Block) error) {
	if isPersist {
		s.persistFunctions[name] = function
	} else {
		s.rollbackFunctions[name] = function
	}
}

func (s *ChainStore) Close() {
	closed := make(chan bool)
	s.quit <- closed
	<-closed

	s.IStore.Close()
}

func (s *ChainStore) taskHandler() {
	for {
		select {
		case t := <-s.taskCh:
			now := time.Now()
			switch task := t.(type) {
			case *persistBlockTask:
				task.reply <- s.handlePersistBlockTask(task.block)
				tcall := float64(time.Now().Sub(now)) / float64(time.Second)
				log.Debugf("handle block exetime: %g num transactions:%d", tcall, len(task.block.Transactions))
			case *rollbackBlockTask:
				task.reply <- s.handleRollbackBlockTask(task.blockHash)
				tcall := float64(time.Now().Sub(now)) / float64(time.Second)
				log.Debugf("handle block rollback exetime: %g", tcall)
			}

		case closed := <-s.quit:
			closed <- true
			return
		}
	}
}

// can only be invoked by backend write goroutine
func (s *ChainStore) clearCache(b *core.Block) {
	s.mu.Lock()
	defer s.mu.Unlock()

	for e := s.headerIdx.Front(); e != nil; e = e.Next() {
		n := e.Value.(core.Header)
		h := n.Hash()
		if h.IsEqual(b.Hash()) {
			s.headerIdx.Remove(e)
		}
	}
}

func (s *ChainStore) initWithGenesisBlock(genesisBlock *core.Block) error {
	prefix := []byte{byte(CFG_Version)}
	version, err := s.Get(prefix)
	if err != nil {
		version = []byte{0x00}
	}

	if version[0] == 0x00 {
		// batch delete old data
		batch := s.NewBatch()
		iter := s.NewIterator(nil)
		for iter.Next() {
			batch.Delete(iter.Key())
		}
		iter.Release()

		err := batch.Commit()
		if err != nil {
			return err
		}

		// persist genesis block
		err = s.persistBlock(genesisBlock)
		if err != nil {
			return err
		}
		// put version to db
		err = s.Put(prefix, []byte{0x01})
		if err != nil {
			return err
		}
	}

	// GenesisBlock should exist in chain
	// Or the bookkeepers are not consistent with the chain
	hash := genesisBlock.Hash()
	if !s.IsBlockInStore(hash) {
		return errors.New("genesis block is not consistent with the chain")
	}

	// Get Current Block
	currentBlockPrefix := []byte{byte(SYS_CurrentBlock)}
	data, err := s.Get(currentBlockPrefix)
	if err != nil {
		return err
	}

	s.currentBlockHeight, err = ReadUint32(bytes.NewReader(data))
	return err
}

func (s *ChainStore) IsDuplicateTx(txId Uint256) bool {
	prefix := []byte{byte(DATA_Transaction)}
	_, err_get := s.Get(append(prefix, txId.Bytes()...))
	if err_get != nil {
		return false
	} else {
		return true
	}
}

func (s *ChainStore) IsDoubleSpend(txn *core.Transaction) bool {
	if len(txn.Inputs) == 0 {
		return false
	}

	unspentPrefix := []byte{byte(IX_Unspent)}
	for i := 0; i < len(txn.Inputs); i++ {
		txhash := txn.Inputs[i].Previous.TxID
		unspentValue, err_get := s.Get(append(unspentPrefix, txhash.Bytes()...))
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

func (s *ChainStore) IsDuplicateMainchainTx(mainchainTxHash Uint256) bool {
	prefix := []byte{byte(IX_MainChain_Tx)}
	_, err := s.Get(append(prefix, mainchainTxHash.Bytes()...))
	if err != nil {
		return false
	} else {
		return true
	}
}

func (s *ChainStore) GetBlockHash(height uint32) (Uint256, error) {
	queryKey := bytes.NewBuffer(nil)
	queryKey.WriteByte(byte(DATA_BlockHash))
	err := WriteUint32(queryKey, height)

	if err != nil {
		return Uint256{}, err
	}
	blockHash, err := s.Get(queryKey.Bytes())
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

func (s *ChainStore) getHeaderWithCache(hash Uint256) *core.Header {
	for e := s.headerIdx.Front(); e != nil; e = e.Next() {
		n := e.Value.(core.Header)
		eh := n.Hash()
		if eh.IsEqual(hash) {
			return &n
		}
	}

	h, _ := s.GetHeader(hash)

	return h
}

func (s *ChainStore) GetCurrentBlockHash() Uint256 {
	hash, err := s.GetBlockHash(s.currentBlockHeight)
	if err != nil {
		return Uint256{}
	}

	return hash
}

func (s *ChainStore) RollbackBlock(blockHash Uint256) error {
	reply := make(chan error)
	s.taskCh <- &rollbackBlockTask{blockHash: blockHash, reply: reply}
	return <-reply
}

func (s *ChainStore) GetHeader(hash Uint256) (*core.Header, error) {
	var h = new(core.Header)

	prefix := []byte{byte(DATA_Header)}
	data, err := s.Get(append(prefix, hash.Bytes()...))
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

func (s *ChainStore) PersistAsset(batch IBatch, assetId Uint256, asset core.Asset) error {
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

func (s *ChainStore) GetAsset(hash Uint256) (*core.Asset, error) {
	log.Debugf("GetAsset Hash: %s", hash.String())

	asset := new(core.Asset)
	prefix := []byte{byte(ST_Info)}
	data, err := s.Get(append(prefix, hash.Bytes()...))

	log.Debugf("GetAsset Data: %s", data)
	if err != nil {
		return nil, err
	}

	asset.Deserialize(bytes.NewReader(data))

	return asset, nil
}

func (s *ChainStore) PersistMainchainTx(batch IBatch, mainchainTxHash Uint256) {
	key := []byte{byte(IX_MainChain_Tx)}
	key = append(key, mainchainTxHash.Bytes()...)

	// PUT VALUE
	batch.Put(key, []byte{byte(ValueExist)})
}

func (s *ChainStore) GetMainchainTx(mainchainTxHash Uint256) (byte, error) {
	key := []byte{byte(IX_MainChain_Tx)}
	data, err := s.Get(append(key, mainchainTxHash.Bytes()...))
	if err != nil {
		return ValueNone, err
	}

	return data[0], nil
}

func (s *ChainStore) GetTransaction(txId Uint256) (*core.Transaction, uint32, error) {
	key := append([]byte{byte(DATA_Transaction)}, txId.Bytes()...)
	value, err := s.Get(key)
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

func (s *ChainStore) GetTxReference(tx *core.Transaction) (map[*core.Input]*core.Output, error) {
	if tx.TxType == core.RegisterAsset {
		return nil, nil
	}
	//UTXO input /  Outputs
	reference := make(map[*core.Input]*core.Output)
	// Key index，v UTXOInput
	for _, input := range tx.Inputs {
		transaction, _, err := s.GetTransaction(input.Previous.TxID)
		if err != nil {
			return nil, errors.New("GetTxReference failed, previous transaction not found")
		}
		index := input.Previous.Index
		if int(index) >= len(transaction.Outputs) {
			return nil, errors.New("GetTxReference failed, refIdx out of range.")
		}
		reference[input] = transaction.Outputs[index]
	}
	return reference, nil
}

func (s *ChainStore) PersistTransaction(batch IBatch, tx *core.Transaction, height uint32) error {
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

func (s *ChainStore) GetBlock(hash Uint256) (*core.Block, error) {
	var b = new(core.Block)
	prefix := []byte{byte(DATA_Header)}
	bHash, err := s.Get(append(prefix, hash.Bytes()...))
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
		tmp, _, err := s.GetTransaction(txn.Hash())
		if err != nil {
			return nil, err
		}
		b.Transactions[i] = tmp
	}

	return b, nil
}

// can only be invoked by backend write goroutine
func (s *ChainStore) addHeader(header *core.Header) {

	log.Debugf("addHeader(), Height=%d", header.Height)

	hash := header.Hash()

	s.mu.Lock()
	s.headerCache[header.Hash()] = header
	s.headerIndex[header.Height] = hash
	s.headerIdx.PushBack(*header)
	s.mu.Unlock()

	log.Debug("[addHeader]: finish, header height:", header.Height)
}

func (s *ChainStore) SaveBlock(b *core.Block) error {
	log.Debug("SaveBlock()")

	reply := make(chan error)
	s.taskCh <- &persistBlockTask{block: b, reply: reply}
	return <-reply
}

func (s *ChainStore) handlePersistBlockTask(block *core.Block) error {
	if block.Header.Height <= s.currentBlockHeight {
		return nil
	}

	if err := s.persistBlock(block); err != nil {
		return fmt.Errorf("persist block %s error %s", block.Hash().String(), err)
	}

	s.mu.Lock()
	s.currentBlockHeight = block.Header.Height
	s.mu.Unlock()

	s.clearCache(block)
	return nil
}

func (s *ChainStore) persistBlock(b *core.Block) error {
	batch := s.NewBatch()
	for _, persistFunc := range s.persistFunctions {
		if err := persistFunc(batch, b); err != nil {
			return err
		}
	}
	return batch.Commit()
}

func (s *ChainStore) handleRollbackBlockTask(blockHash Uint256) error {
	block, err := s.GetBlock(blockHash)
	if err != nil {
		return fmt.Errorf("block %s can't be found", blockHash.String())
	}

	if err := s.rollbackBlock(block); err != nil {
		return fmt.Errorf("rollback block %s error %s", blockHash.String(), err)
	}

	s.mu.Lock()
	s.currentBlockHeight = block.Header.Height - 1
	s.mu.Unlock()

	return nil
}

func (s *ChainStore) rollbackBlock(b *core.Block) error {
	batch := s.NewBatch()
	for _, rollbackFunc := range s.rollbackFunctions {
		rollbackFunc(batch, b)
	}
	return batch.Commit()
}

func (s *ChainStore) GetUnspent(txid Uint256, index uint16) (*core.Output, error) {
	if ok, _ := s.ContainsUnspent(txid, index); ok {
		tx, _, err := s.GetTransaction(txid)
		if err != nil {
			return nil, err
		}

		return tx.Outputs[index], nil
	}

	return nil, errors.New("[GetUnspent] NOT ContainsUnspent.")
}

func (s *ChainStore) ContainsUnspent(txid Uint256, index uint16) (bool, error) {
	unspentPrefix := []byte{byte(IX_Unspent)}
	unspentValue, err := s.Get(append(unspentPrefix, txid.Bytes()...))

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

func (s *ChainStore) GetHeight() uint32 {
	s.mu.RLock()
	defer s.mu.RUnlock()

	return s.currentBlockHeight
}

func (s *ChainStore) IsBlockInStore(hash Uint256) bool {
	var b = new(core.Block)
	prefix := []byte{byte(DATA_Header)}
	log.Debug("Get block key: ", BytesToHexString(append(prefix, hash.Bytes()...)))
	blockData, err := s.Get(append(prefix, hash.Bytes()...))
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

	if b.Header.Height > s.currentBlockHeight {
		log.Error("Header height", b.Header.Height, "greater then current height:", s.currentBlockHeight)
		return false
	}

	return true
}

func (s *ChainStore) GetUnspentElementFromProgramHash(programHash Uint168, assetid Uint256, height uint32) ([]*UTXO, error) {
	prefix := []byte{byte(IX_Unspent_UTXO)}
	prefix = append(prefix, programHash.Bytes()...)
	prefix = append(prefix, assetid.Bytes()...)

	key := bytes.NewBuffer(prefix)
	if err := WriteUint32(key, height); err != nil {
		return nil, err
	}
	unspentsData, err := s.Get(key.Bytes())
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

func (s *ChainStore) GetAssetUnspents(programHash Uint168, assetid Uint256) ([]*UTXO, error) {
	unspents := make([]*UTXO, 0)

	key := []byte{byte(IX_Unspent_UTXO)}
	key = append(key, programHash.Bytes()...)
	key = append(key, assetid.Bytes()...)
	iter := s.NewIterator(key)
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

func (s *ChainStore) GetUnspents(programHash Uint168) (map[Uint256][]*UTXO, error) {
	uxtoUnspents := make(map[Uint256][]*UTXO)

	prefix := []byte{byte(IX_Unspent_UTXO)}
	key := append(prefix, programHash.Bytes()...)
	iter := s.NewIterator(key)
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

func (s *ChainStore) PersistUnspentWithProgramHash(batch IBatch, programHash Uint168, assetid Uint256, height uint32, unspents []*UTXO) error {
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

func (s *ChainStore) GetAssets() map[Uint256]*core.Asset {
	assets := make(map[Uint256]*core.Asset)

	iter := s.NewIterator([]byte{byte(ST_Info)})
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
