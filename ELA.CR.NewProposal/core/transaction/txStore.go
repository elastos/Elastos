package transaction

import (
	. "Elastos.ELA/common"
)

// ILedgerStore provides func with store package.
type ILedgerStore interface {
	GetTransaction(hash Uint256) (*Transaction, uint32, error)
	//GetQuantityIssued(AssetId Uint256) (Fixed64, error)
}
