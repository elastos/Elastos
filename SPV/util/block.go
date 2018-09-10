package util

// Block represent a block that stored into
// blockchain database.
type Block struct {
	// header of this block.
	Header

	// Transactions of this block.
	Transactions []*Tx
}
