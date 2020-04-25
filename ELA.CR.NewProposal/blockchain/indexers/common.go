// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

// Copyright (c) 2016 The btcsuite developers
// Copyright (c) 2017-2019 Elastos Foundation
// Use of this source code is governed by an ISC
// license that can be found in the LICENSE file.

/*
Package indexers implements optional block chain indexes.
*/
package indexers

import (
	"encoding/binary"
	"errors"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/database"
)

var (
	// byteOrder is the preferred byte order used for serializing numeric
	// fields for storage in the database.
	byteOrder = binary.LittleEndian

	// errInterruptRequested indicates that an operation was cancelled due
	// to a user-requested interrupt.
	errInterruptRequested = errors.New("interrupt requested")
)

type IChain interface {
	MainChainHasBlock(height uint32, hash *common.Uint256) bool
	GetBlockByHeight(height uint32) (*types.Block, error)
	GetHeight() uint32
}

// IndexManager provides a generic interface that the is called when blocks are
// connected and disconnected to and from the tip of the main chain for the
// purpose of supporting optional indexes.
type IndexManager interface {
	// Init is invoked during chain initialize in order to allow the index
	// manager to initialize itself and any indexes it is managing.  The
	// channel parameter specifies a channel the caller can close to signal
	// that the process should be interrupted.  It can be nil if that
	// behavior is not desired.
	Init(IChain, <-chan struct{}) error

	// ConnectBlock is invoked when a new block has been connected to the
	// main chain.
	ConnectBlock(database.Tx, *types.Block) error

	// DisconnectBlock is invoked when a block has been disconnected from
	// the main chain.
	DisconnectBlock(database.Tx, *types.Block) error

	// FetchTx retrieval a transaction and a block hash where it
	// located by transaction hash
	FetchTx(txID common.Uint256) (*types.Transaction, uint32, error)

	// FetchUnspent retrieval the unspent set of transaction by its hash
	FetchUnspent(txID common.Uint256) ([]uint16, error)

	// FetchUTXO retrieval the utxo set of a account address
	FetchUTXO(programHash *common.Uint168) ([]*types.UTXO, error)

	// IsTx3Exist use to find if tx3 exist in db
	IsTx3Exist(txHash *common.Uint256) bool
}

// Indexer provides a generic interface for an indexer that is managed by an
// index manager such as the Manager type provided by this package.
type Indexer interface {
	// Key returns the key of the index as a byte slice.
	Key() []byte

	// Name returns the human-readable name of the index.
	Name() string

	// Create is invoked when the indexer manager determines the index needs
	// to be created for the first time.
	Create(dbTx database.Tx) error

	// Init is invoked when the index manager is first initializing the
	// index.  This differs from the Create method in that it is called on
	// every load, including the case the index was just created.
	Init() error

	// ConnectBlock is invoked when a new block has been connected to the
	// main chain.
	ConnectBlock(database.Tx, *types.Block) error

	// DisconnectBlock is invoked when a block has been disconnected from
	// the main chain.
	DisconnectBlock(database.Tx, *types.Block) error
}

type ITxStore interface {
	// FetchTx retrieval a transaction and a block hash where it
	// located by transaction hash
	FetchTx(txID common.Uint256) (*types.Transaction, uint32, error)
}

// AssertError identifies an error that indicates an internal code consistency
// issue and should be treated as a critical and unrecoverable error.
type AssertError string

// Error returns the assertion error as a huma-readable string and satisfies
// the error interface.
func (e AssertError) Error() string {
	return "assertion failed: " + string(e)
}

// errDeserialize signifies that a problem was encountered when deserializing
// data.
type errDeserialize string

// Error implements the error interface.
func (e errDeserialize) Error() string {
	return string(e)
}

// interruptRequested returns true when the provided channel has been closed.
// This simplifies early shutdown slightly since the caller can just use an if
// statement instead of a select.
func interruptRequested(interrupted <-chan struct{}) bool {
	select {
	case <-interrupted:
		return true
	default:
	}

	return false
}
