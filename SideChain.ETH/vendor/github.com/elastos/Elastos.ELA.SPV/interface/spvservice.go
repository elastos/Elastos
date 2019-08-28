package _interface

import (
	"bytes"
	"crypto/sha256"
	"errors"
	"fmt"
	"math"
	"os"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/bloom"
	"github.com/elastos/Elastos.ELA.SPV/database"
	"github.com/elastos/Elastos.ELA.SPV/interface/iutil"
	"github.com/elastos/Elastos.ELA.SPV/interface/store"
	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/elanet/filter"
	"github.com/elastos/Elastos.ELA/elanet/pact"
	"github.com/elastos/Elastos.ELA/p2p/msg"
)

const (
	defaultDataDir = "./data_spv"

	// notifyTimeout is the duration to timeout a notify to the listener, and
	// resend the notify to the listener.
	notifyTimeout = 10 * time.Second // 10 second
)

type spvservice struct {
	sdk.IService
	headers   store.HeaderStore
	db        store.DataStore
	rollback  func(height uint32)
	listeners map[common.Uint256]TransactionListener
}

// NewSPVService creates a new SPV service instance.
func NewSPVService(cfg *Config) (*spvservice, error) {
	dataDir := defaultDataDir
	if len(cfg.DataDir) > 0 {
		dataDir = cfg.DataDir
	}
	_, err := os.Stat(dataDir)
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

	chainStore := database.NewChainDB(headerStore, service)

	serviceCfg := &sdk.Config{
		DataDir:        dataDir,
		ChainParams:    cfg.ChainParams,
		PermanentPeers: cfg.PermanentPeers,
		CandidateFlags: []uint64{
			uint64(pact.SFNodeNetwork),
			uint64(pact.SFNodeBloom),
		},
		GenesisHeader:  GenesisHeader(cfg.ChainParams.GenesisBlock),
		ChainStore:     chainStore,
		NewTransaction: newTransaction,
		NewBlockHeader: newBlockHeader,
		GetTxFilter:    service.GetFilter,
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

func (s *spvservice) VerifyTransaction(proof bloom.MerkleProof, tx types.Transaction) error {
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

func (s *spvservice) SendTransaction(tx types.Transaction) error {
	return s.IService.SendTransaction(iutil.NewTx(&tx))
}

func (s *spvservice) GetTransaction(txId *common.Uint256) (*types.Transaction, error) {
	utx, err := s.db.Txs().Get(txId)
	if err != nil {
		return nil, err
	}

	var tx types.Transaction
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

func (s *spvservice) GetFilter() *msg.TxFilterLoad {
	addrs := s.db.Addrs().GetAll()
	f := bloom.NewFilter(uint32(len(addrs)), math.MaxUint32, 0)
	for _, address := range addrs {
		f.Add(address.Bytes())
	}
	return f.ToTxFilterMsg(filter.FTBloom)
}

func (s *spvservice) putTx(batch store.DataBatch, utx util.Transaction,
	height uint32) (bool, error) {

	tx := utx.(*iutil.Tx)
	hits := make(map[common.Uint168]struct{})
	ops := make(map[*util.OutPoint]common.Uint168)
	for index, output := range tx.Outputs {
		if s.db.Addrs().GetFilter().ContainAddr(output.ProgramHash) {
			outpoint := util.NewOutPoint(tx.Hash(), uint16(index))
			ops[outpoint] = output.ProgramHash
			hits[output.ProgramHash] = struct{}{}
		}
	}

	for _, input := range tx.Inputs {
		op := input.Previous
		addr := s.db.Ops().HaveOp(util.NewOutPoint(op.TxID, op.Index))
		if addr != nil {
			hits[*addr] = struct{}{}
		}
	}

	if len(hits) == 0 {
		return true, nil
	}

	for op, addr := range ops {
		if err := batch.Ops().Put(op, addr); err != nil {
			return false, err
		}
	}

	for _, listener := range s.listeners {
		hash, _ := common.Uint168FromAddress(listener.Address())
		if _, ok := hits[*hash]; ok {
			// skip transactions that not match the require type
			if listener.Type() != tx.TxType {
				continue
			}

			// queue message
			batch.Que().Put(&store.QueItem{
				NotifyId: getListenerKey(listener),
				TxId:     tx.Hash(),
				Height:   height,
			})
		}
	}

	return false, batch.Txs().Put(util.NewTx(utx, height))
}

// PutTxs persists the main chain transactions into database and can be
// queried by GetTxs(height).  Returns the false positive transaction count
// and error.
func (s *spvservice) PutTxs(txs []util.Transaction, height uint32) (uint32, error) {
	fps := uint32(0)
	batch := s.db.Batch()
	defer batch.Rollback()
	for _, tx := range txs {
		fp, err := s.putTx(batch, tx, height)
		if err != nil {
			return 0, err
		}
		if fp {
			fps++
		}
	}
	if err := batch.Commit(); err != nil {
		return 0, err
	}
	return fps, nil
}

// PutForkTxs persists the fork chain transactions into database with the
// fork block hash and can be queried by GetForkTxs(hash).
func (s *spvservice) PutForkTxs(txs []util.Transaction, hash *common.Uint256) error {
	ftxs := make([]*util.Tx, 0, len(txs))
	for _, utx := range txs {
		ftxs = append(ftxs, util.NewTx(utx, 0))
	}
	return s.db.Txs().PutForkTxs(ftxs, hash)
}

// HaveTx returns if the transaction already saved in database
// by it's id.
func (s *spvservice) HaveTx(txId *common.Uint256) (bool, error) {
	tx, err := s.db.Txs().Get(txId)
	return tx != nil, err
}

// GetTxs returns all transactions in main chain within the given height.
func (s *spvservice) GetTxs(height uint32) ([]util.Transaction, error) {
	txIDs, err := s.db.Txs().GetIds(height)
	if err != nil {
		return nil, err
	}

	txs := make([]util.Transaction, 0, len(txIDs))
	for _, txID := range txIDs {
		tx := newTransaction()
		utx, err := s.db.Txs().Get(txID)
		if err != nil {
			return nil, err
		}
		err = tx.Deserialize(bytes.NewReader(utx.RawData))
		if err != nil {
			return nil, err
		}
		txs = append(txs, tx)
	}
	return txs, nil
}

// GetForkTxs returns all transactions within the fork block hash.
func (s *spvservice) GetForkTxs(hash *common.Uint256) ([]util.Transaction, error) {
	ftxs, err := s.db.Txs().GetForkTxs(hash)
	if err != nil {
		return nil, err
	}

	txs := make([]util.Transaction, 0, len(ftxs))
	for _, ftx := range ftxs {
		tx := newTransaction()
		err = tx.Deserialize(bytes.NewReader(ftx.RawData))
		if err != nil {
			return nil, err
		}
		txs = append(txs, tx)
	}
	return txs, nil
}

// DelTxs remove all transactions in main chain within the given height.
func (s *spvservice) DelTxs(height uint32) error {
	// Delete transactions, outpoints and queued items.
	batch := s.db.Batch()
	defer batch.Rollback()
	if err := batch.DelAll(height); err != nil {
		return err
	}
	if err := batch.Commit(); err != nil {
		return err
	}

	// Invoke main chain rollback.
	if s.rollback != nil {
		s.rollback(height)
	}
	return nil
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

		var tx types.Transaction
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
		listener, ok := s.notifyTransaction(item.NotifyId, proof, tx, block.Height-item.Height)
		if ok {
			item.LastNotify = time.Now()
			s.db.Que().Put(item)
			listener.Notify(item.NotifyId, proof, tx)
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
	listener TransactionListener, tx *types.Transaction, height uint32) {
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
	proof bloom.MerkleProof, tx types.Transaction,
	confirmations uint32) (TransactionListener, bool) {

	listener, ok := s.listeners[notifyId]
	if !ok {
		return nil, false
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
		return nil, false
	}

	// Notify listener
	if listener.Flags()&FlagNotifyConfirmed == FlagNotifyConfirmed {
		if confirmations >= getConfirmations(tx) {
			return listener, true
		}
	} else {
		listener.Notify(notifyId, proof, tx)
		return listener, true
	}

	return nil, false
}

func getListenerKey(listener TransactionListener) common.Uint256 {
	buf := new(bytes.Buffer)
	addr, _ := common.Uint168FromAddress(listener.Address())
	common.WriteElements(buf, addr[:], listener.Type(), listener.Flags())
	return sha256.Sum256(buf.Bytes())
}

func getConfirmations(tx types.Transaction) uint32 {
	// TODO user can set confirmations attribute in transaction,
	// if the confirmation attribute is set, use it instead of default value
	if tx.TxType == types.CoinBase {
		return 100
	}
	return DefaultConfirmations
}

func newBlockHeader() util.BlockHeader {
	return iutil.NewHeader(&types.Header{})
}

func newTransaction() util.Transaction {
	return iutil.NewTx(&types.Transaction{})
}

// GenesisHeader creates a specific genesis header by the given
// foundation address.
func GenesisHeader(genesisBlock *types.Block) util.BlockHeader {
	return iutil.NewHeader(&genesisBlock.Header)
}
