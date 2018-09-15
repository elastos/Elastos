package util

import "github.com/elastos/Elastos.ELA/core"

// Block represent a block that stored in the
// blockchain database.
type Block struct {
	// header of this block.
	Header

	// Transactions of this block.
	Transactions []*core.Transaction
}
