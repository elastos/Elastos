package blockchain

import (
	"bytes"
	"errors"
	"sync"
	"time"

	. "github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/contract"
	. "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	. "github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/events"
)

const (
	TaskChanCap = 4

	ProducerUnRegistered ProducerState = 0x00
	ProducerRegistered   ProducerState = 0x01
	ProducerRegistering  ProducerState = 0x02
)

type ProducerState byte

type ProducerInfo struct {
	Payload   *PayloadRegisterProducer
	RegHeight uint32
	Vote      Fixed64
}

type persistTask interface{}

type rollbackBlockTask struct {
	blockHash Uint256
	reply     chan bool
}

type persistBlockTask struct {
	block *Block
	reply chan bool
}

type persistConfirmTask struct {
	confirm *DPosProposalVoteSlot
	reply   chan bool
}

type ChainStore struct {
	IStore

	taskCh chan persistTask
	quit   chan chan bool

	mu sync.RWMutex // guard the following var

	currentBlockHeight uint32

	producerVotes    map[string]*ProducerInfo
	producerAddress  map[string]string // key: address  value: public key
	dirty            map[outputpayload.VoteType]bool
	orderedProducers []*PayloadRegisterProducer
}

func NewChainStore(filePath string) (IChainStore, error) {
	st, err := NewLevelDB(filePath)
	if err != nil {
		return nil, err
	}

	store := &ChainStore{
		IStore:             st,
		currentBlockHeight: 0,
		taskCh:             make(chan persistTask, TaskChanCap),
		quit:               make(chan chan bool, 1),
		producerVotes:      make(map[string]*ProducerInfo),
		producerAddress:    make(map[string]string),
		dirty:              make(map[outputpayload.VoteType]bool),
		orderedProducers:   make([]*PayloadRegisterProducer, 0),
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
			case *persistConfirmTask:
				c.handlePersistConfirmTask(task.confirm)
				task.reply <- true
				tcall := float64(time.Now().Sub(now)) / float64(time.Second)
				log.Debugf("handle confirm exetime: %g block hash:%s", tcall, task.confirm.Hash.String())
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

func (c *ChainStore) InitProducerVotes() error {
	publicKeys, err := c.getRegisteredProducers()
	if err != nil {
		return err
	}
	for _, pk := range publicKeys {
		height, payload, err := c.getProducerInfo(pk)
		if err != nil {
			return err
		}
		p, ok := payload.(*PayloadRegisterProducer)
		if !ok {
			return errors.New("invalid register producer payload")
		}

		vote, _ := c.getVoteByPublicKey(outputpayload.Delegate, pk)
		c.producerVotes[BytesToHexString(pk)] = &ProducerInfo{
			Payload:   p,
			RegHeight: height,
			Vote:      vote,
		}
		programHash, err := contract.PublicKeyToStandardProgramHash(pk)
		if err != nil {
			return err
		}
		addr, err := programHash.ToAddress()
		if err != nil {
			return err
		}
		c.producerAddress[addr] = BytesToHexString(pk)
		for _, t := range outputpayload.VoteTypes {
			c.dirty[t] = true
		}
	}
	return nil
}

func (c *ChainStore) InitWithGenesisBlock(genesisBlock *Block) (uint32, error) {
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
	currentBlockPrefix := []byte{byte(SYSCurrentBlock)}
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

func (c *ChainStore) PersistAsset(assetID Uint256, asset Asset) error {
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

func (c *ChainStore) GetAsset(hash Uint256) (*Asset, error) {
	asset := new(Asset)
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

func (c *ChainStore) PersistTransaction(tx *Transaction, height uint32) error {
	// generate key with DATA_Transaction prefix
	key := new(bytes.Buffer)
	// add transaction header prefix.
	key.WriteByte(byte(DATATransaction))
	// get transaction hash
	hash := tx.Hash()
	if err := hash.Serialize(key); err != nil {
		return err
	}
	log.Debugf("transaction header + hash: %x", key)

	// generate value
	value := new(bytes.Buffer)
	if err := WriteUint32(value, height); err != nil {
		return err
	}
	if err := tx.Serialize(value); err != nil {
		return err
	}
	log.Debugf("transaction tx data: %x", value)

	// put value
	c.BatchPut(key.Bytes(), value.Bytes())
	return nil
}

func (c *ChainStore) GetBlock(hash Uint256) (*Block, error) {
	var b = new(Block)
	prefix := []byte{byte(DATAHeader)}
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
	c.RollbackConfirm(b)
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

func (c *ChainStore) SaveBlock(b *Block) error {
	log.Debug("SaveBlock()")

	reply := make(chan bool)
	c.taskCh <- &persistBlockTask{block: b, reply: reply}
	<-reply

	return nil
}

func (c *ChainStore) SaveConfirm(confirm *DPosProposalVoteSlot) error {
	log.Debug("SaveConfirm()")

	reply := make(chan bool)
	c.taskCh <- &persistConfirmTask{confirm: confirm, reply: reply}
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

	c.persistBlock(b)
}

func (c *ChainStore) handlePersistConfirmTask(confirm *DPosProposalVoteSlot) {
	c.persistConfirm(confirm)
}

func (c *ChainStore) persistBlock(block *Block) {
	err := c.persist(block)
	if err != nil {
		log.Fatal("[persistBlocks]: error to persist block:", err.Error())
		return
	}

	DefaultLedger.Blockchain.UpdateBestHeight(block.Header.Height)
	c.mu.Lock()
	c.currentBlockHeight = block.Header.Height
	c.mu.Unlock()
	DefaultLedger.Blockchain.BCEvents.Notify(events.EventBlockPersistCompleted, block)
}

func (c *ChainStore) persistConfirm(confirm *DPosProposalVoteSlot) {
	c.NewBatch()
	if err := c.PersistConfirm(confirm); err != nil {
		log.Fatal("[persistConfirm]: error to persist confirm:", err.Error())
		return
	}
	if err := c.BatchCommit(); err != nil {
		log.Fatal("[persistConfirm]: error to commit confirm:", err.Error())
		return
	}
}

func (c *ChainStore) GetConfirm(hash Uint256) (*DPosProposalVoteSlot, error) {
	var confirm = new(DPosProposalVoteSlot)
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
	c.mu.RLock()
	defer c.mu.RUnlock()

	return c.currentBlockHeight
}

func (c *ChainStore) IsBlockInStore(hash Uint256) bool {
	var b = new(Block)
	prefix := []byte{byte(DATAHeader)}
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

	iter := c.NewIterator([]byte{byte(STInfo)})
	for iter.Next() {
		rk := bytes.NewReader(iter.Key())

		// read prefix
		_, _ = ReadBytes(rk, 1)
		var assetid Uint256
		assetid.Deserialize(rk)
		log.Debugf("[GetAssets] assetid: %x", assetid.Bytes())

		asset := new(Asset)
		r := bytes.NewReader(iter.Value())
		asset.Deserialize(r)

		assets[assetid] = asset
	}

	return assets
}

func (c *ChainStore) OnIllegalBlockTxnReceived(txn *Transaction) {
	illegalBlock, ok := txn.Payload.(*PayloadIllegalBlock)
	if !ok {
		return
	}
	if err := c.PersistIllegalBlock(illegalBlock); err != nil {
		log.Error("Persist illegal block tx error: ", err.Error())
	}
	if illegalBlock.CoinType == ELACoin {
		if err := DefaultLedger.Arbitrators.ForceChange(); err != nil {
			log.Error("OnIllegalBlockTxnReceived force change failed:", err.Error())
		}
	}
}
