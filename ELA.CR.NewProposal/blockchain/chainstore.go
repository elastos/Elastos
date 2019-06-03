package blockchain

import (
	"bytes"
	"errors"
	"path/filepath"
	"sync/atomic"
	"time"

	. "github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/log"
	. "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
)

const (
	TaskChanCap = 4
)

type ProducerState byte

type ProducerInfo struct {
	Payload   *payload.ProducerInfo
	RegHeight uint32
	Vote      Fixed64
}

type persistTask interface{}

type rollbackBlockTask struct {
	blockHash Uint256
	reply     chan bool
}

type persistBlockTask struct {
	block   *Block
	confirm *payload.Confirm
	reply   chan bool
}

type ChainStore struct {
	IStore

	taskCh chan persistTask
	quit   chan chan bool

	currentBlockHeight uint32
}

func NewChainStore(dataDir string, genesisBlock *Block) (IChainStore, error) {
	db, err := NewLevelDB(filepath.Join(dataDir, "chain"))
	if err != nil {
		return nil, err
	}

	s := &ChainStore{
		IStore: db,
		taskCh: make(chan persistTask, TaskChanCap),
		quit:   make(chan chan bool, 1),
	}

	go s.taskHandler()

	s.init(genesisBlock)

	return s, nil
}

func (c *ChainStore) Close() {
	closed := make(chan bool)
	c.quit <- closed
	<-closed
	c.IStore.Close()
}

func (c *ChainStore) taskHandler() {
	for {
		select {
		case t := <-c.taskCh:
			now := time.Now()
			switch task := t.(type) {
			case *persistBlockTask:
				c.handlePersistBlockTask(task.block, task.confirm)
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
		err = c.persist(genesisBlock, nil)
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

func (c *ChainStore) IsTxHashDuplicate(txhash Uint256) bool {
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

func (c *ChainStore) rollback(b *Block) error {
	c.NewBatch()
	c.RollbackTrimmedBlock(b)
	c.RollbackBlockHash(b)
	c.RollbackTransactions(b)
	c.RollbackUnspendUTXOs(b)
	c.RollbackUnspend(b)
	c.RollbackCurrentBlock(b)
	c.RollbackConfirm(b)
	c.BatchCommit()

	atomic.StoreUint32(&c.currentBlockHeight, b.Height-1)

	return nil
}

func (c *ChainStore) persist(b *Block, confirm *payload.Confirm) error {
	c.NewBatch()
	if err := c.persistTrimmedBlock(b); err != nil {
		return err
	}
	if err := c.persistBlockHash(b); err != nil {
		return err
	}
	if err := c.PersistTransactions(b); err != nil {
		return err
	}
	if err := c.persistUTXOs(b); err != nil {
		return err
	}
	if err := c.persistUnspend(b); err != nil {
		return err
	}
	if err := c.persistCurrentBlock(b); err != nil {
		return err
	}
	if err := c.persistConfirm(confirm); err != nil {
		return err
	}
	return c.BatchCommit()
}

func (c *ChainStore) SaveBlock(b *Block, confirm *payload.Confirm) error {
	log.Debug("SaveBlock()")

	reply := make(chan bool)
	c.taskCh <- &persistBlockTask{block: b, confirm: confirm, reply: reply}
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

func (c *ChainStore) handlePersistBlockTask(b *Block, confirm *payload.Confirm) {
	if b.Header.Height <= c.currentBlockHeight {
		return
	}

	c.persistBlock(b, confirm)
}

func (c *ChainStore) persistBlock(block *Block, confirm *payload.Confirm) {
	err := c.persist(block, confirm)
	if err != nil {
		log.Fatal("[persistBlocks]: error to persist block:", err.Error())
		return
	}

	atomic.StoreUint32(&c.currentBlockHeight, block.Height)
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
				return nil, err
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
	WriteVarUint(w, uint64(listnum))
	for i := 0; i < listnum; i++ {
		if unspents[i].Value > 0 {
			storeCount++
			unspents[i].Serialize(w)
		}
	}
	if storeCount == 0 {
		c.BatchDelete(key.Bytes())
		return nil
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
