// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package indexers

import (
	"bytes"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/database"
)

const (
	// utxoIndexName is the human-readable name for the index.
	utxoIndexName = "utxo index"
)

var (
	// utxoIndexKey is the key of the utxo index and the db bucket used
	// to house it.
	utxoIndexKey = []byte("utxobyhashidx")
)

// dbPutUtxoIndexEntry uses an existing database transaction to update the
// index of utxo.
func dbPutUtxoIndexEntry(dbTx database.Tx, programHash *common.Uint168, utxos []*types.UTXO) error {
	utxoIndex := dbTx.Metadata().Bucket(utxoIndexKey)
	var utxosWithoutZero []*types.UTXO
	for _, utxo := range utxos {
		if utxo.Value > 0 {
			utxosWithoutZero = append(utxosWithoutZero, utxo)
		}
	}
	w := new(bytes.Buffer)
	count := len(utxosWithoutZero)
	if err := common.WriteVarUint(w, uint64(count)); err != nil {
		return err
	}
	for _, utxo := range utxosWithoutZero {
		if err := utxo.Serialize(w); err != nil {
			return err
		}
	}
	return utxoIndex.Put(programHash[:], w.Bytes())
}

// dbFetchUtxoIndexEntry uses an existing database transaction to fetch its
// utxos. When there is no entry for the provided hash, nil will be returned
// for the both the index and the error.
func dbFetchUtxoIndexEntry(dbTx database.Tx, programHash *common.Uint168) (
	[]*types.UTXO, error) {
	// Load the record from the database and return now if it doesn't exist.
	utxoIndex := dbTx.Metadata().Bucket(utxoIndexKey)
	serializedData := utxoIndex.Get(programHash[:])
	if len(serializedData) == 0 {
		return nil, nil
	}

	r := bytes.NewReader(serializedData)
	count, err := common.ReadVarUint(r, 0)
	if err != nil {
		return nil, err
	}
	utxos := make([]*types.UTXO, 0, count)
	for i := 0; i < int(count); i++ {
		var utxo types.UTXO
		if err := utxo.Deserialize(r); err != nil {
			return nil, err
		}
		utxos = append(utxos, &utxo)
	}

	return utxos, nil
}

// UtxoIndex implements a utxo by tx hash index.
type UtxoIndex struct {
	db    database.DB
	store ITxStore
}

// Init initializes the hash-based utxo index. This is part of the Indexer
// interface.
func (idx *UtxoIndex) Init() error {
	return nil // Nothing to do.
}

// Key returns the database key to use for the index as a byte slice.
//
// This is part of the Indexer interface.
func (idx *UtxoIndex) Key() []byte {
	return utxoIndexKey
}

// Name returns the human-readable name of the index.
//
// This is part of the Indexer interface.
func (idx *UtxoIndex) Name() string {
	return utxoIndexName
}

// Create is invoked when the indexer manager determines the index needs
// to be created for the first time.  It creates the buckets for the utxo
// index.
//
// This is part of the Indexer interface.
func (idx *UtxoIndex) Create(dbTx database.Tx) error {
	meta := dbTx.Metadata()
	_, err := meta.CreateBucket(utxoIndexKey)
	return err
}

// ConnectBlock is invoked by the index manager when a new block has been
// connected to the main chain.  This indexer maintains a hash-to-utxo
// mapping for every transaction in the passed block.
//
// This is part of the Indexer interface.
func (idx *UtxoIndex) ConnectBlock(dbTx database.Tx, block *types.Block) error {
	for _, txn := range block.Transactions {
		txHash := txn.Hash()
		// output process
		for i, output := range txn.Outputs {
			toBeStore := make([]*types.UTXO, 0)
			utxos, err := dbFetchUtxoIndexEntry(dbTx, &output.ProgramHash)
			if err != nil {
				return err
			}
			if utxos != nil {
				toBeStore = append(toBeStore, utxos...)
			}
			toBeStore = append(toBeStore, &types.UTXO{TxID: txHash, Index: uint16(i), Value: output.Value})
			err = dbPutUtxoIndexEntry(dbTx, &output.ProgramHash, toBeStore)
			if err != nil {
				return err
			}
		}
		if txn.IsCoinBaseTx() {
			continue
		}
		// inputs process
		for _, input := range txn.Inputs {
			referTx, _, err := idx.store.FetchTx(input.Previous.TxID)
			if err != nil {
				return err
			}
			referOutput := referTx.Outputs[input.Previous.Index]
			// find the spent items and remove it
			utxos, err := dbFetchUtxoIndexEntry(dbTx, &referOutput.ProgramHash)
			if err != nil {
				return err
			}
			for i, utxo := range utxos {
				if utxo.TxID == input.Previous.TxID &&
					utxo.Index == input.Previous.Index {
					// swap and pop
					utxos[i] = utxos[len(utxos)-1]
					utxos = utxos[:len(utxos)-1]
					break
				}
			}
			err = dbPutUtxoIndexEntry(dbTx, &referOutput.ProgramHash, utxos)
			if err != nil {
				return err
			}
		}
	}

	return nil
}

// DisconnectBlock is invoked by the index manager when a block has been
// disconnected from the main chain.  This indexer removes the
// hash-to-utxos mapping for every transaction in the block.
//
// This is part of the Indexer interface.
func (idx *UtxoIndex) DisconnectBlock(dbTx database.Tx, block *types.Block) error {
	for _, txn := range block.Transactions {
		txHash := txn.Hash()
		// output process
		for index, output := range txn.Outputs {
			utxos, err := dbFetchUtxoIndexEntry(dbTx, &output.ProgramHash)
			if err != nil {
				return err
			}
			for i, utxo := range utxos {
				if utxo.TxID == txHash && utxo.Index == uint16(index) {
					// swap and pop
					utxos[i] = utxos[len(utxos)-1]
					utxos = utxos[:len(utxos)-1]
					break
				}
			}
			err = dbPutUtxoIndexEntry(dbTx, &output.ProgramHash, utxos)
			if err != nil {
				return err
			}
		}

		// inputs process
		if txn.IsCoinBaseTx() {
			continue
		}
		for _, input := range txn.Inputs {
			referTx, _, err := idx.store.FetchTx(input.Previous.TxID)
			if err != nil {
				return err
			}
			referOutput := referTx.Outputs[input.Previous.Index]
			utxos, err := dbFetchUtxoIndexEntry(dbTx, &referOutput.ProgramHash)
			if err != nil {
				return err
			}
			toBeStore := make([]*types.UTXO, 0)
			if utxos != nil {
				toBeStore = append(toBeStore, utxos...)
			}
			toBeStore = append(toBeStore, &types.UTXO{
				TxID:  input.Previous.TxID,
				Index: input.Previous.Index,
				Value: referOutput.Value,
			})
			err = dbPutUtxoIndexEntry(dbTx, &referOutput.ProgramHash, toBeStore)
			if err != nil {
				return err
			}
		}
	}

	return nil
}

// NewUtxoIndex returns a new instance of an indexer that is used to create a
// mapping of the program hashes of all addresses be used in the blockchain to
// the their utxo.
//
// It implements the Indexer interface which plugs into the IndexManager that in
// turn is used by the blockchain package.  This allows the index to be
// seamlessly maintained along with the chain.
func NewUtxoIndex(db database.DB, store ITxStore) *UtxoIndex {
	return &UtxoIndex{db, store}
}
