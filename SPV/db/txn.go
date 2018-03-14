package db

import (
	"SPVWallet/core"
)

type Txn struct {
	// Transaction ID
	TxId core.Uint256

	// The height at which it was mined
	Height uint32

	// Raw transaction bytes
	RawData []byte
}
