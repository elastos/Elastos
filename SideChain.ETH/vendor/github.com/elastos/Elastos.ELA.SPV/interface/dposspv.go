package _interface

import (
	"bytes"
	"encoding/hex"
	"fmt"
	"github.com/elastos/Elastos.ELA/common/config"
	"math"
	"os"

	"github.com/elastos/Elastos.ELA.SPV/bloom"
	"github.com/elastos/Elastos.ELA.SPV/database"
	"github.com/elastos/Elastos.ELA.SPV/interface/iutil"
	"github.com/elastos/Elastos.ELA.SPV/interface/store"
	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/dpos/state"
	"github.com/elastos/Elastos.ELA/elanet/filter"
	"github.com/elastos/Elastos.ELA/elanet/pact"
	"github.com/elastos/Elastos.ELA/p2p/msg"
)

// Ensure dposspv implement the DPOSSPVService interface.
var _ DPOSSPVService = (*dposspv)(nil)

// dposspv is the implementation of DPOSSPVService interface.
type dposspv struct {
	spvservice
	arbiters           state.Arbitrators
	db                 store.DPOSStore
	lastProducers      map[string]struct{}
	onProducersChanged func(sideProducerIDs [][]byte)
}

// Overwrite to return DPOS type filter.
func (d *dposspv) GetFilter() *msg.TxFilterLoad {
	addrs := d.db.Addrs().GetAll()
	f := bloom.NewFilter(uint32(len(addrs)), math.MaxUint32, 0)
	for _, address := range addrs {
		f.Add(address.Bytes())
	}
	return f.ToTxFilterMsg(filter.FTDPOS)
}

// Overwrite to do DPOS database rollback.
func (d *dposspv) DelTxs(height uint32) error {
	if err := d.spvservice.DelTxs(height); err != nil {
		return err
	}
	return d.db.Rollback()
}

// TransactionAnnounce will be invoked when received a new announced transaction.
func (d *dposspv) TransactionAnnounce(tx util.Transaction) {
	// Handle transactions that will change block producers immediately.
	current := d.GetHeight()
	payload := tx.(*iutil.Tx).Payload
	err := d.arbiters.ProcessSpecialTxPayload(payload, current)
	if err != nil {
		log.Errorf("precess special tx failed, %s", err)
	}

	// Update side producers by available main chain producers.
	d.archiveProducers(current, nil)

	// Notify side producers change.
	pubKeys, changed := d.isProducersChanged(current)
	if changed && d.onProducersChanged != nil {
		d.onProducersChanged(pubKeys)
	}
}

// archiveProducers archive the current producers.
func (d *dposspv) archiveProducers(height uint32, blockHash *common.Uint256) {
	crcs := d.arbiters.GetCRCArbitrators()
	producers := d.arbiters.GetArbitrators()
	candidates := d.arbiters.GetCandidates()

	pubKeys := make([][]byte, 0, len(crcs)+len(producers)+len(candidates))
	for _, crc := range crcs {
		pubKeys = append(pubKeys, crc.NodePublicKey())
	}
	log.Debugf("CRCs: %v", crcs)
	for _, p := range producers {
		pubKeys = append(pubKeys, p)
	}
	log.Debugf("Current: %v", producers)
	for _, c := range candidates {
		pubKeys = append(pubKeys, c)
	}
	log.Debugf("Candidates: %v", candidates)

	// Archive the block producers on the specific height.
	err := d.db.Archive(pubKeys, height, blockHash)
	if err != nil {
		log.Errorf("archive producers failed, %v", err)
	}
}

// isProducersChanged returns the current side producer public keys and if they
// have changed.
func (d *dposspv) isProducersChanged(height uint32) ([][]byte, bool) {
	pubKeys, _ := d.db.GetProducers(height)
	if len(d.lastProducers) != len(pubKeys) {
		return pubKeys, true
	}
	for _, pubKey := range pubKeys {
		_, ok := d.lastProducers[hex.EncodeToString(pubKey)]
		if ok {
			continue
		}
		return pubKeys, true
	}
	return pubKeys, false
}

// Overwrite to process arbiters by transactions.
func (d *dposspv) BlockCommitted(block *util.Block) {
	// Call supper class first.
	d.spvservice.BlockCommitted(block)

	var header = block.BlockHeader.(*iutil.DHeader)
	blk := &types.Block{Header: header.Header}
	blk.Transactions = make([]*types.Transaction, 0, len(block.Transactions))
	for _, tx := range block.Transactions {
		txn := tx.(*iutil.Tx).Transaction
		blk.Transactions = append(blk.Transactions, txn)

		if txn.TxType != types.TransferAsset {
			continue
		}

		// Save mapping by handle mapping output payload.
		for _, output := range txn.Outputs {
			if output.Type != types.OTMapping {
				continue
			}
			p := output.Payload.(*outputpayload.Mapping)
			err := d.db.Mapping(p.OwnerPublicKey, p.SideProducerID,
				block.Height)
			if err != nil {
				log.Errorf("mapping side producer ID failed, %s", err)
			}
		}
	}

	// Update main chain producers arbiters by processing block.
	d.arbiters.ProcessBlock(blk, &header.Confirm)

	// Update side producers by available main chain producers.
	blockHash := block.Hash()
	d.archiveProducers(block.Height, &blockHash)

	// Do not notify producers change until synced to current.
	if !d.spvservice.IsCurrent() {
		return
	}

	// Notify side producers change.
	pubKeys, changed := d.isProducersChanged(block.Height)
	if changed && d.onProducersChanged != nil {
		d.onProducersChanged(pubKeys)
	}
}

// GetProducersByHeight returns the side chain block producer IDs on the
// specific height.
func (d *dposspv) GetProducersByHeight(height uint32) [][]byte {
	producers, _ := d.db.GetProducers(height)
	return producers
}

// NewDPOSSPVService creates a new DPOSSPVService instance.
func NewDPOSSPVService(cfg *DPOSConfig, interrupt <-chan struct{}) (DPOSSPVService, error) {
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

	headerStore, err := store.NewHeaderStore(dataDir, newDBlockHeader)
	if err != nil {
		return nil, err
	}

	dposStore, err := store.NewDPOSStore(dataDir)
	if err != nil {
		return nil, err
	}

	arbiters, err := state.NewArbitrators(cfg.ChainParams, func() uint32 {
		best, err := headerStore.GetBest()
		if err != nil {
			return 0
		}
		return best.Height
	})
	if err != nil {
		return nil, err
	}

	service := &dposspv{
		spvservice: spvservice{
			headers:   headerStore,
			db:        dposStore,
			rollback:  cfg.OnRollback,
			listeners: make(map[common.Uint256]TransactionListener),
		},
		arbiters:           arbiters,
		db:                 dposStore,
		lastProducers:      make(map[string]struct{}),
		onProducersChanged: cfg.OnProducersChanged,
	}

	chainStore := database.NewChainDB(headerStore, service)

	serviceCfg := &sdk.Config{
		DataDir:     dataDir,
		Magic:       cfg.Magic,
		SeedList:    cfg.SeedList,
		DefaultPort: cfg.DefaultPort,
		MaxPeers:    cfg.MaxConnections,
		CandidateFlags: []uint64{
			uint64(pact.SFNodeNetwork),
			uint64(pact.SFNodeBloom),
		},
		GenesisHeader:  GenesisDHeader(foundation),
		ChainStore:     chainStore,
		NewTransaction: newTransaction,
		NewBlockHeader: newDBlockHeader,
		GetTxFilter:    service.GetFilter,
		StateNotifier:  service,
	}

	// Initialize state by process all blocks after vote start height.
	bestHeight := service.GetHeight()
	errChan := make(chan error)
	go func() {
		for i := cfg.ChainParams.VoteStartHeight; i <= bestHeight; i++ {
			hash, err := dposStore.GetBlockHash(i)
			if err != nil {
				errChan <- err
				continue
			}
			header, err := headerStore.Get(hash)
			if err != nil {
				errChan <- err
				continue
			}
			iHeader := header.BlockHeader.(*iutil.DHeader)
			txids, err := dposStore.Txs().GetIds(i)
			blk := types.Block{
				Header:       iHeader.Header,
				Transactions: make([]*types.Transaction, 0, len(txids)),
			}
			for _, txID := range txids {
				utx, err := dposStore.Txs().Get(txID)
				if err != nil {
					continue
				}
				var tx types.Transaction
				err = tx.Deserialize(bytes.NewReader(utx.RawData))
				if err != nil {
					errChan <- err
					continue
				}
				blk.Transactions = append(blk.Transactions, &tx)
			}
			arbiters.ProcessBlock(&blk, &iHeader.Confirm)
		}
		errChan <- nil
	}()
	select {
	case err := <-errChan:
		if err != nil {
			return nil, fmt.Errorf("initialize DPOS SPV service"+
				" failed, %s", err)
		}
	case <-interrupt:
		return nil, fmt.Errorf("initialize DPOS SPV service interrupted")
	}

	// Initialize last producers.
	ids, _ := dposStore.GetProducers(bestHeight)
	for _, id := range ids {
		service.lastProducers[hex.EncodeToString(id)] = struct{}{}
	}

	service.IService, err = sdk.NewService(serviceCfg)
	if err != nil {
		return nil, err
	}

	return service, nil
}

func newDBlockHeader() util.BlockHeader {
	return iutil.NewDHeader(&types.DPOSHeader{})
}

// GenesisHeader creates a specific genesis header by the given
// foundation address.
func GenesisDHeader(foundation *common.Uint168) util.BlockHeader {
	return iutil.NewDHeader(&types.DPOSHeader{
		Header: config.GenesisBlock(foundation).Header})
}
