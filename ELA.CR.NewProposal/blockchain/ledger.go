// Copyright (c) 2017-2019 Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package blockchain

import (
	"errors"

	. "github.com/elastos/Elastos.ELA/common"
	. "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	crstate "github.com/elastos/Elastos.ELA/cr/state"
	"github.com/elastos/Elastos.ELA/dpos/state"
)

var (
	EnableUtxoDB      bool
	FoundationAddress Uint168
	DefaultLedger     *Ledger
)

// Ledger - the struct for ledger
type Ledger struct {
	Blockchain  *BlockChain
	Store       IChainStore
	Arbitrators state.Arbitrators
	Committee   *crstate.Committee
}

//check weather the transaction contains the doubleSpend.
func (l *Ledger) IsDoubleSpend(Tx *Transaction) bool {
	return DefaultLedger.Store.IsDoubleSpend(Tx)
}

//Get the DefaultLedger.
//Note: the later version will support the mutiLedger.So this func mybe expired later.

//Get the Asset from store.
func (l *Ledger) GetAsset(assetID Uint256) (*payload.Asset, error) {
	asset, err := l.Store.GetAsset(assetID)
	if err != nil {
		return nil, errors.New("[Ledger],GetAsset failed with assetID =" + assetID.String())
	}
	return asset, nil
}

//Get Block With Height.
func (l *Ledger) GetBlockWithHeight(height uint32) (*Block, error) {
	temp, err := l.Blockchain.GetBlockHash(height)
	if err != nil {
		return nil, errors.New("[Ledger],GetBlockWithHeight failed with height=" + string(height))
	}
	bk, err := DefaultLedger.Store.GetFFLDB().GetBlock(temp)
	if err != nil {
		return nil, errors.New("[Ledger],GetBlockWithHeight failed with hash=" + temp.String())
	}
	return bk, nil
}

//Get block with block hash.
func (l *Ledger) GetBlockWithHash(hash Uint256) (*Block, error) {
	bk, err := l.Store.GetFFLDB().GetBlock(hash)
	if err != nil {
		return nil, errors.New("[Ledger],GetBlockWithHeight failed with hash=" + hash.String())
	}
	return bk, nil
}

//Get transaction with hash.
func (l *Ledger) GetTransactionWithHash(hash Uint256) (*Transaction, error) {
	tx, _, err := l.Store.GetTransaction(hash)
	if err != nil {
		return nil, errors.New("[Ledger],GetTransactionWithHash failed with hash=" + hash.String())
	}
	return tx, nil
}

//Get local block chain height.
func (l *Ledger) GetLocalBlockChainHeight() uint32 {
	return l.Blockchain.GetHeight()
}

//Get blocks and confirms by given height range, if end equals zero will be treat as current highest block height
func (l *Ledger) GetDposBlocks(start, end uint32) ([]*DposBlock, error) {
	//todo complete me
	return nil, nil
}

//Append blocks and confirms directly
func (l *Ledger) AppendDposBlocks(confirms []*DposBlock) error {
	//todo complete me
	return nil
}
