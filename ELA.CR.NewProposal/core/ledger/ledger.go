package ledger

import (
	. "github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/asset"
	tx "github.com/elastos/Elastos.ELA/core/transaction"
	"errors"
)

const (
	FoundationAddress = "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta"
)

var DefaultLedger *Ledger

// Ledger - the struct for ledger
type Ledger struct {
	Blockchain *Blockchain
	Store      ILedgerStore
}

//check weather the transaction contains the doubleSpend.
func (l *Ledger) IsDoubleSpend(Tx *tx.Transaction) bool {
	return DefaultLedger.Store.IsDoubleSpend(Tx)
}

//Get the DefaultLedger.
//Note: the later version will support the mutiLedger.So this func mybe expired later.

//Get the Asset from store.
func (l *Ledger) GetAsset(assetId Uint256) (*asset.Asset, error) {
	asset, err := l.Store.GetAsset(assetId)
	if err != nil {
		return nil, errors.New("[Ledger],GetAsset failed with assetId =" + assetId.String())
	}
	return asset, nil
}

//Get Block With Height.
func (l *Ledger) GetBlockWithHeight(height uint32) (*Block, error) {
	temp, err := l.Store.GetBlockHash(height)
	if err != nil {
		return nil, errors.New("[Ledger],GetBlockWithHeight failed with height=" + string(height))
	}
	bk, err := DefaultLedger.Store.GetBlock(temp)
	if err != nil {
		return nil, errors.New("[Ledger],GetBlockWithHeight failed with hash=" + temp.String())
	}
	return bk, nil
}

//Get block with block hash.
func (l *Ledger) GetBlockWithHash(hash Uint256) (*Block, error) {
	bk, err := l.Store.GetBlock(hash)
	if err != nil {
		return nil, errors.New("[Ledger],GetBlockWithHeight failed with hash=" + hash.String())
	}
	return bk, nil
}

//BlockInLedger checks if the block existed in ledger
func (l *Ledger) BlockInLedger(hash Uint256) bool {
	return l.Store.IsBlockInStore(hash)
}

//Get transaction with hash.
func (l *Ledger) GetTransactionWithHash(hash Uint256) (*tx.Transaction, error) {
	tx, _, err := l.Store.GetTransaction(hash)
	if err != nil {
		return nil, errors.New("[Ledger],GetTransactionWithHash failed with hash=" + hash.String())
	}
	return tx, nil
}

//Get local block chain height.
func (l *Ledger) GetLocalBlockChainHeight() uint32 {
	return l.Blockchain.GetBestHeight()

}
