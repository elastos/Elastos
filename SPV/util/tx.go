package util

import "github.com/elastos/Elastos.ELA/core"

// Tx is a data structure used in database.
type Tx struct {
	// The origin transaction data.
	core.Transaction

	// The block height that this transaction
	// belongs to.
	Height uint32
}