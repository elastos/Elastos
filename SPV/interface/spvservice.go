package _interface

import (
	"bytes"
	"crypto/sha256"
	"errors"
	"fmt"
	"os"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/bloom"
	"github.com/elastos/Elastos.ELA.SPV/database"
	"github.com/elastos/Elastos.ELA.SPV/interface/iutil"
	"github.com/elastos/Elastos.ELA.SPV/interface/store"
	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA/core"
)

const (
	defaultDataDir = "./data_spv"

	// notifyTimeout is the duration to timeout a notify to the listener, and
	// resend the notify to the listener.
	notifyTimeout  = 10 * time.Second // 10 second
)

type spvservice struct {
	sdk.IService
	headers   store.HeaderStore
	db        store.DataStore
	rollback  func(height uint32)
	listeners map[common.Uint256]TransactionListener
}

func newSpvService(cfg *Config) (*spvservice, error) {
	if cfg.Foundation == "" {
		cfg.Foundation = "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta"
	}

	foundation, err := common.Uint168FromAddress(cfg.Foundation)
	if err != nil {
		return nil, fmt.Errorf("Parse foundation address error %s", err)
	}

	dataDir := defaultDataDir
	if len(cfg.DataDir) > 0 {
		dataDir = cfg.DataDir
	}
	_, err = os.Stat(dataDir)
	if os.IsNotExist(err) {
		err := os.MkdirAll(dataDir, os.ModePerm)
		if err != nil {
			return nil, fmt.Errorf("make data dir failed")
		}
	}

	headerStore, err := store.NewHeaderStore(dataDir, newBlockHeader)
	if err != nil {
		return nil, err
	}

	dataStore, err := store.NewDataStore(dataDir)
	if err != nil {
		return nil, err
	}

	service := &spvservice{
		headers:   headerStore,
		db:        dataStore,
		rollback:  cfg.OnRollback,
		listeners: make(map[common.Uint256]TransactionListener),
	}

	chainStore := database.NewDefaultChainDB(headerStore, service)

	serviceCfg := &sdk.Config{
		DataDir:        dataDir,
		Magic:          cfg.Magic,
		SeedList:       cfg.SeedList,
		DefaultPort:    cfg.DefaultPort,
		MaxPeers:       cfg.MaxConnections,
		GenesisHeader:  GenesisHeader(foundation),
		ChainStore:     chainStore,
		NewTransaction: newTransaction,
		NewBlockHeader: newBlockHeader,
		GetFilterData:  service.GetFilterData,
		StateNotifier:  service,
	}

	service.IService, err = sdk.NewService(serviceCfg)
	if err != nil {
		return nil, err
	}

	return service, nil
}

func (s *spvservice) RegisterTransactionListener(listener TransactionListener) error {
	address, err := common.Uint168FromAddress(listener.Address())
	if err != nil {
		return fmt.Errorf("address %s is not a valied address", listener.Address())
	}
	key := getListenerKey(listener)
	if _, ok := s.listeners[key]; ok {
		return fmt.Errorf("listener with address: %s type: %s flags: %d already registered",
			listener.Address(), listener.Type().Name(), listener.Flags())
	}
	s.listeners[key] = listener
	return s.db.Addrs().Put(address)
}

func (s *spvservice) SubmitTransactionReceipt(notifyId, txHash common.Uint256) error {
	return s.db.Que().Del(&notifyId, &txHash)
}

func (s *spvservice) VerifyTransaction(proof bloom.MerkleProof, tx core.Transaction) error {
	// Get Header from main chain
	header, err := s.headers.Get(&proof.BlockHash)
	if err != nil {
		return errors.New("can not get block from main chain")
	}

	// Check if merkleroot is match
	merkleBlock := msg.MerkleBlock{
		Header:       header.BlockHeader,
		Transactions: proof.Transactions,
		Hashes:       proof.Hashes,
		Flags:        proof.Flags,
	}
	txIds, err := bloom.CheckMerkleBlock(merkleBlock)
	if err != nil {
		return fmt.Errorf("check merkle branch failed, %s", err.Error())
	}
	if len(txIds) == 0 {
		return fmt.Errorf("invalid transaction proof, no transactions found")
	}

	// Check if transaction hash is match
	match := false
	for _, txId := range txIds {
		if *txId == tx.Hash() {
			match = true
			break
		}
	}
	if !match {
		return fmt.Errorf("transaction hash not match proof")
	}

	return nil
}

func (s *spvservice) SendTransaction(tx core.Transaction) error {
	return s.IService.SendTransaction(iutil.NewTx(&tx))
}

func (s *spvservice) GetTransaction(txId *common.Uint256) (*core.Transaction, error) {
	utx, err := s.db.Txs().Get(txId)
	if err != nil {
		return nil, err
	}

	var tx core.Transaction
	err = tx.Deserialize(bytes.NewReader(utx.RawData))
	if err != nil {
		return nil, err
	}

	return &tx, nil
}

func (s *spvservice) GetTransactionIds(height uint32) ([]*common.Uint256, error) {
	return s.db.Txs().GetIds(height)
}

func (s *spvservice) HeaderStore() database.Headers {
	return s.headers
}

func (s *spvservice) GetFilterData() ([]*common.Uint168, []*util.OutPoint) {
	ops, err := s.db.Ops().GetAll()
	if err != nil {
		log.Error("[SPV_SERVICE] GetData error ", err)
	}

	return s.db.Addrs().GetAll(), ops
}

// Batch returns a TxBatch instance for transactions batch
// commit, this can get better performance when commit a bunch
// of transactions within a block.
func (s *spvservice) Batch() database.TxBatch {
	return &txBatch{
		db:        s.db,
		batch:     s.db.Batch(),
		rollback:  s.rollback,
		listeners: s.listeners,
	}
}

// HaveTx returns if the transaction already saved in database
// by it's id.
func (s *spvservice) HaveTx(txId *common.Uint256) (bool, error) {
	tx, err := s.db.Txs().Get(txId)
	return tx != nil, err
}

// GetTxs returns all transactions within the given height.
func (s *spvservice) GetTxs(height uint32) ([]*util.Tx, error) {
	return nil, nil
}

// RemoveTxs delete all transactions on the given height.  Return
// how many transactions are deleted from database.
func (s *spvservice) RemoveTxs(height uint32) (int, error) {
	batch := s.db.Batch()
	if err := batch.DelAll(height); err != nil {
		return 0, batch.Rollback()
	}
	return 0, batch.Commit()
}

// TransactionAnnounce will be invoked when received a new announced transaction.
func (s *spvservice) TransactionAnnounce(tx util.Transaction) {}

// TransactionAccepted will be invoked after a transaction sent by
// SendTransaction() method has been accepted.  Notice: this method needs at
// lest two connected peers to work.
func (s *spvservice) TransactionAccepted(tx util.Transaction) {}

// TransactionRejected will be invoked if a transaction sent by SendTransaction()
// method has been rejected.
func (s *spvservice) TransactionRejected(tx util.Transaction) {}

// TransactionConfirmed will be invoked after a transaction sent by
// SendTransaction() method has been packed into a block.
func (s *spvservice) TransactionConfirmed(tx *util.Tx) {}

// BlockCommitted will be invoked when a block and transactions within it are
// successfully committed into database.
func (s *spvservice) BlockCommitted(block *util.Block) {
	// Look up for queued transactions
	items, err := s.db.Que().GetAll()
	if err != nil {
		return
	}
	for _, item := range items {
		// Check if the notify should be resend due to timeout.
		if time.Now().Before(item.LastNotify.Add(notifyTimeout)) {
			continue
		}

		//	Get header
		header, err := s.headers.GetByHeight(item.Height)
		if err != nil {
			log.Errorf("query merkle proof at height %d failed, %s", item.Height, err.Error())
			continue
		}

		//	Get transaction from db
		utx, err := s.db.Txs().Get(&item.TxId)
		if err != nil {
			log.Errorf("query transaction failed, txId %s", item.TxId.String())
			continue
		}

		var tx core.Transaction
		err = tx.Deserialize(bytes.NewReader(utx.RawData))
		if err != nil {
			continue
		}

		var proof = bloom.MerkleProof{
			BlockHash:    header.Hash(),
			Height:       header.Height,
			Transactions: header.NumTxs,
			Hashes:       header.Hashes,
			Flags:        header.Flags,
		}

		// Notify listeners
		if s.notifyTransaction(item.NotifyId, proof, tx, block.Height-item.Height) {
			item.LastNotify = time.Now()
			s.db.Que().Put(item)
		}
	}
}

func (s *spvservice) ClearData() error {
	if err := s.headers.Clear(); err != nil {
		log.Warnf("Clear header store error %s", err.Error())
	}
	if err := s.db.Clear(); err != nil {
		log.Warnf("Clear data store error %s", err.Error())
	}
	return nil
}

func (s *spvservice) Clear() error {
	return s.db.Clear()
}

func (s *spvservice) Close() error {
	return s.db.Close()
}

func (s *spvservice) queueMessageByListener(
	listener TransactionListener, tx *core.Transaction, height uint32) {
	// skip unpacked transaction
	if height == 0 {
		return
	}

	// skip transactions that not match the require type
	if listener.Type() != tx.TxType {
		return
	}

	// queue message
	s.db.Que().Put(&store.QueItem{
		NotifyId: getListenerKey(listener),
		TxId:     tx.Hash(),
		Height:   height,
	})
}

func (s *spvservice) notifyTransaction(notifyId common.Uint256,
	proof bloom.MerkleProof, tx core.Transaction, confirmations uint32) bool {

	listener, ok := s.listeners[notifyId]
	if !ok {
		return false
	}

	// Get transaction id
	txId := tx.Hash()

	// Remove notifications if FlagNotifyInSyncing not set
	if s.IService.IsCurrent() == false &&
		listener.Flags()&FlagNotifyInSyncing != FlagNotifyInSyncing {

		if listener.Flags()&FlagNotifyConfirmed == FlagNotifyConfirmed {
			if confirmations >= getConfirmations(tx) {
				s.db.Que().Del(&notifyId, &txId)
			}
		} else {
			s.db.Que().Del(&notifyId, &txId)
		}
		return false
	}

	// Notify listener
	if listener.Flags()&FlagNotifyConfirmed == FlagNotifyConfirmed {
		if confirmations >= getConfirmations(tx) {
			listener.Notify(notifyId, proof, tx)
			return true
		}
	} else {
		listener.Notify(notifyId, proof, tx)
		return true
	}

	return false
}

func getListenerKey(listener TransactionListener) common.Uint256 {
	buf := new(bytes.Buffer)
	addr, _ := common.Uint168FromAddress(listener.Address())
	common.WriteElements(buf, addr[:], listener.Type(), listener.Flags())
	return sha256.Sum256(buf.Bytes())
}

func getConfirmations(tx core.Transaction) uint32 {
	// TODO user can set confirmations attribute in transaction,
	// if the confirmation attribute is set, use it instead of default value
	if tx.TxType == core.CoinBase {
		return 100
	}
	return DefaultConfirmations
}

type txBatch struct {
	db        store.DataStore
	batch     store.DataBatch
	heights   []uint32
	rollback  func(height uint32)
	listeners map[common.Uint256]TransactionListener
}

// PutTx add a store transaction operation into batch, and return
// if it is a false positive and error.
func (b *txBatch) PutTx(utx util.Transaction, height uint32) (bool, error) {
	tx := utx.(*iutil.Tx)
	hits := make(map[common.Uint168]struct{})
	ops := make(map[*util.OutPoint]common.Uint168)
	for index, output := range tx.Outputs {
		if b.db.Addrs().GetFilter().ContainAddr(output.ProgramHash) {
			outpoint := util.NewOutPoint(tx.Hash(), uint16(index))
			ops[outpoint] = output.ProgramHash
			hits[output.ProgramHash] = struct{}{}
		}
	}

	for _, input := range tx.Inputs {
		op := input.Previous
		addr := b.db.Ops().HaveOp(util.NewOutPoint(op.TxID, op.Index))
		if addr != nil {
			hits[*addr] = struct{}{}
		}
	}

	if len(hits) == 0 {
		return true, nil
	}

	for op, addr := range ops {
		if err := b.batch.Ops().Put(op, addr); err != nil {
			return false, err
		}
	}

	for _, listener := range b.listeners {
		hash, _ := common.Uint168FromAddress(listener.Address())
		if _, ok := hits[*hash]; ok {
			// skip transactions that not match the require type
			if listener.Type() != tx.TxType {
				continue
			}

			// queue message
			b.batch.Que().Put(&store.QueItem{
				NotifyId: getListenerKey(listener),
				TxId:     tx.Hash(),
				Height:   height,
			})
		}
	}

	return false, b.batch.Txs().Put(util.NewTx(utx, height))
}

// DelTx add a delete transaction operation into batch.
func (b *txBatch) DelTx(txId *common.Uint256) error {
	utx, err := b.db.Txs().Get(txId)
	if err != nil {
		return err
	}

	var tx core.Transaction
	err = tx.Deserialize(bytes.NewReader(utx.RawData))
	if err != nil {
		return err
	}

	for index := range tx.Outputs {
		outpoint := util.NewOutPoint(utx.Hash, uint16(index))
		b.batch.Ops().Del(outpoint)
	}

	return b.batch.Txs().Del(txId)
}

// DelTxs add a delete transactions on given height operation.
func (b *txBatch) DelTxs(height uint32) error {
	if b.rollback != nil {
		b.heights = append(b.heights, height)
	}
	return b.batch.DelAll(height)
}

// Rollback cancel all operations in current batch.
func (b *txBatch) Rollback() error {
	return b.batch.Rollback()
}

// Commit the added transactions into database.
func (b *txBatch) Commit() error {
	err := b.batch.Commit()
	if err != nil {
		return err
	}

	go func(heights []uint32) {
		for _, height := range heights {
			b.rollback(height)
		}
	}(b.heights)

	return nil
}
