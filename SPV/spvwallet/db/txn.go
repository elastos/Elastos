package db

import . "github.com/elastos/Elastos.ELA.SPV/common"

type Txn struct {
	// Transaction ID
	TxId Uint256

	// The height at which it was mined
	Height uint32

	// Raw transaction bytes
	RawData []byte
}
