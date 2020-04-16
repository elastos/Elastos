package blockchain

import (
	"bytes"
	"container/list"
	"errors"
	"fmt"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA.SideChain/database"
	"github.com/elastos/Elastos.ELA.SideChain/types"

	"github.com/elastos/Elastos.ELA/common"
)

// EntryPrefix
type EntryPrefix byte

const (
	// DATA
	DATA_BlockHash   EntryPrefix = 0x00
	DATA_Header      EntryPrefix = 0x01
	DATA_Transaction EntryPrefix = 0x02

	// INDEX
	IX_HeaderHashList EntryPrefix = 0x80
	IX_Unspent        EntryPrefix = 0x90
	IX_Unspent_UTXO   EntryPrefix = 0x91
	IX_SideChain_Tx   EntryPrefix = 0x92
	IX_MainChain_Tx   EntryPrefix = 0x93
	IX_Identification EntryPrefix = 0x94

	// ASSET
	ST_Info EntryPrefix = 0xc0

	//NEOVM
	ST_Contract   EntryPrefix = 0xc1
	ST_Storage    EntryPrefix = 0xc2
	ST_Account    EntryPrefix = 0xc3
	ST_AssetState EntryPrefix = 0xc4

	//SYSTEM
	SYS_CurrentBlock      EntryPrefix = 0x40
	SYS_CurrentBookKeeper EntryPrefix = 0x42

	//CONFIG
	CFG_Version EntryPrefix = 0xf0
)

const (
	ValueNone   = 0
	ValueExist  = 1
	TaskChanCap = 4
)

type persistTask interface{}

type rollbackBlockTask struct {
	blockHash common.Uint256
	reply     chan error
}

type persistBlockTask struct {
	block *types.Block
	reply chan error
}

type action struct {
	Name    StoreFuncName
	Handler func(batch database.Batch, b *types.Block) error
}

type ChainStore struct {
	database.Database

	taskCh chan persistTask
	quit   chan chan bool

	mu          sync.RWMutex // guard the following var
	headerIndex map[uint32]common.Uint256
	headerCache map[common.Uint256]*types.Header
	headerIdx   *list.List

	currentBlockHeight uint32
	storedHeaderCount  uint32

	persistFunctions  []*action
	rollbackFunctions []*action
}

func NewChainStore(path string, genesisBlock *types.Block) (*ChainStore, error) {
	// TODO: read config file decide which db to use.
	levelDB, err := database.NewLevelDB(path)
	if err != nil {
		return nil, err
	}

	s := ChainStore{
		Database:           levelDB,
		headerIndex:        map[uint32]common.Uint256{},
		headerCache:        map[common.Uint256]*types.Header{},
		headerIdx:          list.New(),
		currentBlockHeight: 0,
		storedHeaderCount:  0,
		taskCh:             make(chan persistTask, TaskChanCap),
		quit:               make(chan chan bool, 1),
	}

	s.RegisterFunctions(true, StoreFuncNames.PersistTrimmedBlock, s.persistTrimmedBlock)
	s.RegisterFunctions(true, StoreFuncNames.PersistBlockHash, s.persistBlockHash)
	s.RegisterFunctions(true, StoreFuncNames.PersistCurrentBlock, s.persistCurrentBlock)
	s.RegisterFunctions(true, StoreFuncNames.PersistUnspendUTXOs, s.persistUnspendUTXOs)
	s.RegisterFunctions(true, StoreFuncNames.PersistTransactions, s.persistTransactions)
	s.RegisterFunctions(true, StoreFuncNames.PersistUnspend, s.persistUnspend)

	s.RegisterFunctions(false, StoreFuncNames.RollbackTrimmedBlock, s.rollbackTrimmedBlock)
	s.RegisterFunctions(false, StoreFuncNames.RollbackBlockHash, s.rollbackBlockHash)
	s.RegisterFunctions(false, StoreFuncNames.RollbackCurrentBlock, s.rollbackCurrentBlock)
	s.RegisterFunctions(false, StoreFuncNames.RollbackUnspendUTXOs, s.rollbackUnspendUTXOs)
	s.RegisterFunctions(false, StoreFuncNames.RollbackTransactions, s.rollbackTransactions)
	s.RegisterFunctions(false, StoreFuncNames.RollbackUnspend, s.rollbackUnspend)

	go s.taskHandler()

	return &s, s.initWithGenesisBlock(genesisBlock)
}

func (s *ChainStore) RegisterFunctions(isPersist bool, name StoreFuncName, handler func(batch database.Batch, b *types.Block) error) {
	if isPersist {
		for _, action := range s.persistFunctions {
			if action.Name == name {
				action.Handler = handler
				return
			}
		}
		s.persistFunctions = append(s.persistFunctions, &action{Name: name, Handler: handler})
	} else {
		for _, action := range s.rollbackFunctions {
			if action.Name == name {
				action.Handler = handler
				return
			}
		}
		s.rollbackFunctions = append(s.rollbackFunctions, &action{Name: name, Handler: handler})
	}
}

func (s *ChainStore) Close() {
	closed := make(chan bool)
	s.quit <- closed
	<-closed

	s.Database.Close()
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
func (s *ChainStore) clearCache(b *types.Block) {
	s.mu.Lock()
	defer s.mu.Unlock()

	for e := s.headerIdx.Front(); e != nil; e = e.Next() {
		n := e.Value.(types.Header)
		h := n.Hash()
		if h.IsEqual(b.Hash()) {
			s.headerIdx.Remove(e)
		}
	}
}

func (s *ChainStore) initWithGenesisBlock(genesisBlock *types.Block) error {
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
	if !s.IsBlockInStore(&hash) {
		return errors.New("genesis block is not consistent with the chain")
	}

	// Get Current Block
	currentBlockPrefix := []byte{byte(SYS_CurrentBlock)}
	data, err := s.Get(currentBlockPrefix)
	if err != nil {
		return err
	}

	r := bytes.NewReader(data)
	var blockHash common.Uint256
	blockHash.Deserialize(r)
	s.currentBlockHeight, err = common.ReadUint32(r)
	return err
}

func (s *ChainStore) IsDuplicateTx(txId common.Uint256) bool {
	prefix := []byte{byte(DATA_Transaction)}
	_, err := s.Get(append(prefix, txId.Bytes()...))
	if err != nil {
		return false
	} else {
		return true
	}
}

func (s *ChainStore) IsDoubleSpend(txn *types.Transaction) bool {
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

func (s *ChainStore) IsDuplicateMainchainTx(mainchainTxHash common.Uint256) bool {
	prefix := []byte{byte(IX_MainChain_Tx)}
	_, err := s.Get(append(prefix, mainchainTxHash.Bytes()...))
	if err != nil {
		return false
	} else {
		return true
	}
}

func (s *ChainStore) GetBlockHash(height uint32) (common.Uint256, error) {
	queryKey := bytes.NewBuffer(nil)
	queryKey.WriteByte(byte(DATA_BlockHash))
	err := common.WriteUint32(queryKey, height)

	if err != nil {
		return common.Uint256{}, err
	}
	blockHash, err := s.Get(queryKey.Bytes())
	if err != nil {
		//TODO: implement error process
		return common.Uint256{}, err
	}
	blockHash256, err := common.Uint256FromBytes(blockHash)
	if err != nil {
		return common.Uint256{}, err
	}

	return *blockHash256, nil
}

func (s *ChainStore) getHeaderWithCache(hash common.Uint256) *types.Header {
	for e := s.headerIdx.Front(); e != nil; e = e.Next() {
		n := e.Value.(types.Header)
		eh := n.Hash()
		if eh.IsEqual(hash) {
			return &n
		}
	}

	h, _ := s.GetHeader(hash)

	return h
}

func (s *ChainStore) GetCurrentBlockHash() common.Uint256 {
	hash, err := s.GetBlockHash(s.currentBlockHeight)
	if err != nil {
		return common.Uint256{}
	}

	return hash
}

func (s *ChainStore) RollbackBlock(blockHash common.Uint256) error {
	reply := make(chan error)
	s.taskCh <- &rollbackBlockTask{blockHash: blockHash, reply: reply}
	return <-reply
}

func (s *ChainStore) GetHeader(hash common.Uint256) (*types.Header, error) {
	var h = new(types.Header)

	prefix := []byte{byte(DATA_Header)}
	data, err := s.Get(append(prefix, hash.Bytes()...))
	if err != nil {
		//TODO: implement error process
		return nil, err
	}

	r := bytes.NewReader(data)
	// first 8 bytes is sys_fee
	_, err = common.ReadUint64(r)
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

func (s *ChainStore) PersistAsset(batch database.Batch, assetId common.Uint256, asset types.Asset) error {
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

func (s *ChainStore) GetAsset(hash common.Uint256) (*types.Asset, error) {
	log.Debugf("GetAsset Hash: %s", hash.String())

	asset := new(types.Asset)
	prefix := []byte{byte(ST_Info)}
	data, err := s.Get(append(prefix, hash.Bytes()...))

	log.Debugf("GetAsset Data: %s", data)
	if err != nil {
		return nil, err
	}

	asset.Deserialize(bytes.NewReader(data))

	return asset, nil
}

func (s *ChainStore) PersistMainchainTx(batch database.Batch, mainchainTxHash common.Uint256) {
	key := []byte{byte(IX_MainChain_Tx)}
	key = append(key, mainchainTxHash.Bytes()...)

	// PUT VALUE
	batch.Put(key, []byte{byte(ValueExist)})
}

func (s *ChainStore) GetMainchainTx(mainchainTxHash common.Uint256) (byte, error) {
	key := []byte{byte(IX_MainChain_Tx)}
	data, err := s.Get(append(key, mainchainTxHash.Bytes()...))
	if err != nil {
		return ValueNone, err
	}

	return data[0], nil
}

func (s *ChainStore) GetTransaction(txId common.Uint256) (*types.Transaction, uint32, error) {
	key := append([]byte{byte(DATA_Transaction)}, txId.Bytes()...)
	value, err := s.Get(key)
	if err != nil {
		return nil, 0, err
	}

	r := bytes.NewReader(value)
	height, err := common.ReadUint32(r)
	if err != nil {
		return nil, 0, err
	}

	var txn types.Transaction
	if err := txn.Deserialize(r); err != nil {
		return nil, height, err
	}

	return &txn, height, nil
}

func (s *ChainStore) GetTxReference(tx *types.Transaction) (map[*types.Input]*types.Output, error) {
	//UTXO input /  Outputs
	reference := make(map[*types.Input]*types.Output)
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

func (s *ChainStore) GetBlock(hash common.Uint256) (*types.Block, error) {
	var b = types.NewBlock()
	prefix := []byte{byte(DATA_Header)}
	bHash, err := s.Get(append(prefix, hash.Bytes()...))
	if err != nil {
		return nil, err
	}

	r := bytes.NewReader(bHash)

	// first 8 bytes is sys_fee
	_, err = common.ReadUint64(r)
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
func (s *ChainStore) addHeader(header *types.Header) {

	log.Debugf("addHeader(), Height=%d", header.Base.Height)

	hash := header.Hash()

	s.mu.Lock()
	s.headerCache[header.Hash()] = header
	s.headerIndex[header.Base.Height] = hash
	s.headerIdx.PushBack(*header)
	s.mu.Unlock()

	log.Debug("[addHeader]: finish, header height:", header.Base.Height)
}

func (s *ChainStore) SaveBlock(b *types.Block) error {
	log.Debug("SaveBlock()")

	reply := make(chan error)
	s.taskCh <- &persistBlockTask{block: b, reply: reply}
	return <-reply
}

func (s *ChainStore) handlePersistBlockTask(block *types.Block) error {
	if block.Header.GetHeight() <= s.currentBlockHeight {
		return nil
	}

	if err := s.persistBlock(block); err != nil {
		return fmt.Errorf("persist block %s error %s", block.Hash().String(), err)
	}

	s.mu.Lock()
	s.currentBlockHeight = block.Header.GetHeight()
	s.mu.Unlock()

	s.clearCache(block)
	return nil
}

func (s *ChainStore) persistBlock(b *types.Block) error {
	batch := s.NewBatch()
	for _, persistFunc := range s.persistFunctions {
		if err := persistFunc.Handler(batch, b); err != nil {
			return err
		}
	}
	return batch.Commit()
}

func (s *ChainStore) handleRollbackBlockTask(blockHash common.Uint256) error {
	block, err := s.GetBlock(blockHash)
	if err != nil {
		return fmt.Errorf("block %s can't be found", blockHash.String())
	}

	if err := s.rollbackBlock(block); err != nil {
		return fmt.Errorf("rollback block %s error %s", blockHash.String(), err)
	}

	s.mu.Lock()
	s.currentBlockHeight = block.Header.GetHeight() - 1
	s.mu.Unlock()

	return nil
}

func (s *ChainStore) rollbackBlock(b *types.Block) error {
	batch := s.NewBatch()
	for _, rollbackFunc := range s.rollbackFunctions {
		rollbackFunc.Handler(batch, b)
	}
	return batch.Commit()
}

func (s *ChainStore) GetUnspent(txid common.Uint256, index uint16) (*types.Output, error) {
	if ok, _ := s.ContainsUnspent(txid, index); ok {
		tx, _, err := s.GetTransaction(txid)
		if err != nil {
			return nil, err
		}

		return tx.Outputs[index], nil
	}

	return nil, errors.New("[GetUnspent] NOT ContainsUnspent.")
}

func (s *ChainStore) ContainsUnspent(txid common.Uint256, index uint16) (bool, error) {
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

func (s *ChainStore) IsBlockInStore(hash *common.Uint256) bool {
	var b = types.NewBlock()
	prefix := []byte{byte(DATA_Header)}
	blockData, err := s.Get(append(prefix, hash.Bytes()...))
	if err != nil {
		return false
	}

	r := bytes.NewReader(blockData)

	// first 8 bytes is sys_fee
	_, err = common.ReadUint64(r)
	if err != nil {
		log.Error("Read sys_fee failed: ", err)
		return false
	}

	// Deserialize block header data
	err = b.Header.Deserialize(r)
	if err != nil {
		log.Error("Get block header failed: ", err)
		return false
	}

	if b.Header.GetHeight() > s.currentBlockHeight {
		log.Error("Header height", b.Header.GetHeight(), "greater than current height:", s.currentBlockHeight)
		return false
	}

	return true
}

func (s *ChainStore) GetUnspentElementFromProgramHash(programHash common.Uint168, assetid common.Uint256, height uint32) ([]*types.UTXO, error) {
	prefix := []byte{byte(IX_Unspent_UTXO)}
	prefix = append(prefix, programHash.Bytes()...)
	prefix = append(prefix, assetid.Bytes()...)

	key := bytes.NewBuffer(prefix)
	if err := common.WriteUint32(key, height); err != nil {
		return nil, err
	}
	unspentsData, err := s.Get(key.Bytes())
	if err != nil {
		return nil, err
	}
	r := bytes.NewReader(unspentsData)
	listNum, err := common.ReadVarUint(r, 0)
	if err != nil {
		return nil, err
	}

	// read unspent list in store
	unspents := make([]*types.UTXO, listNum)
	for i := 0; i < int(listNum); i++ {
		var utxo types.UTXO
		err := utxo.Deserialize(r)
		if err != nil {
			return nil, err
		}

		unspents[i] = &utxo
	}

	return unspents, nil
}

func (s *ChainStore) GetAssetUnspents(programHash common.Uint168, assetid common.Uint256) ([]*types.UTXO, error) {
	unspents := make([]*types.UTXO, 0)

	key := []byte{byte(IX_Unspent_UTXO)}
	key = append(key, programHash.Bytes()...)
	key = append(key, assetid.Bytes()...)
	iter := s.NewIterator(key)
	defer iter.Release()
	for iter.Next() {
		r := bytes.NewReader(iter.Value())
		listNum, err := common.ReadVarUint(r, 0)
		if err != nil {
			return nil, err
		}

		for i := 0; i < int(listNum); i++ {
			var utxo types.UTXO
			err := utxo.Deserialize(r)
			if err != nil {
				return nil, err
			}

			unspents = append(unspents, &utxo)
		}
	}

	return unspents, nil
}

func (s *ChainStore) GetUnspents(programHash common.Uint168) (map[common.Uint256][]*types.UTXO, error) {
	uxtoUnspents := make(map[common.Uint256][]*types.UTXO)

	prefix := []byte{byte(IX_Unspent_UTXO)}
	key := append(prefix, programHash.Bytes()...)
	iter := s.NewIterator(key)
	defer iter.Release()
	for iter.Next() {
		rk := bytes.NewReader(iter.Key())

		// read prefix
		_, _ = common.ReadBytes(rk, 1)
		var ph common.Uint168
		ph.Deserialize(rk)
		var assetid common.Uint256
		assetid.Deserialize(rk)

		r := bytes.NewReader(iter.Value())
		listNum, err := common.ReadVarUint(r, 0)
		if err != nil {
			return nil, err
		}

		// read unspent list in store
		unspents := make([]*types.UTXO, listNum)
		for i := 0; i < int(listNum); i++ {
			var utxo types.UTXO
			err := utxo.Deserialize(r)
			if err != nil {
				return nil, err
			}

			unspents[i] = &utxo
		}
		uxtoUnspents[assetid] = append(uxtoUnspents[assetid], unspents[:]...)
	}

	return uxtoUnspents, nil
}

func (s *ChainStore) PersistUnspentWithProgramHash(batch database.Batch, programHash common.Uint168, assetid common.Uint256, height uint32, unspents []*types.UTXO) error {
	prefix := []byte{byte(IX_Unspent_UTXO)}
	prefix = append(prefix, programHash.Bytes()...)
	prefix = append(prefix, assetid.Bytes()...)
	key := bytes.NewBuffer(prefix)
	if err := common.WriteUint32(key, height); err != nil {
		return err
	}

	if len(unspents) == 0 {
		batch.Delete(key.Bytes())
		return nil
	}

	listnum := len(unspents)
	w := bytes.NewBuffer(nil)
	common.WriteVarUint(w, uint64(listnum))
	for i := 0; i < listnum; i++ {
		unspents[i].Serialize(w)
	}

	// BATCH PUT VALUE
	batch.Put(key.Bytes(), w.Bytes())

	return nil
}

func (s *ChainStore) GetAssets() map[common.Uint256]*types.Asset {
	assets := make(map[common.Uint256]*types.Asset)

	iter := s.NewIterator([]byte{byte(ST_Info)})
	defer iter.Release()
	for iter.Next() {
		rk := bytes.NewReader(iter.Key())

		// read prefix
		_, _ = common.ReadBytes(rk, 1)
		var assetid common.Uint256
		assetid.Deserialize(rk)

		asset := new(types.Asset)
		r := bytes.NewReader(iter.Value())
		asset.Deserialize(r)

		assets[assetid] = asset
	}

	return assets
}
