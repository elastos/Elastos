package sdk

import (
	"github.com/elastos/Elastos.ELA.SPV/bloom"
	"github.com/elastos/Elastos.ELA.SPV/core/transaction"
	"github.com/elastos/Elastos.ELA.SPV/common"
)

// Create a new bloom filter instance
// elements are how many elements will be added to this filter.
func NewBloomFilter(elements uint32) *bloom.Filter {
	return bloom.NewFilter(elements, 0, 0.00003)
}

// Add a address into the given bloom filter
func FilterAddress(filter *bloom.Filter, address *common.Uint168) {
	filter.Add(address.ToArray())
}

// Add a account into the given bloom filter
func FilterAccount(filter *bloom.Filter, account *Account) {
	filter.Add(account.programHash.ToArray())
}

// Add a outpoint into the given bloom filter
func FilterOutpoint(filter *bloom.Filter, op *transaction.OutPoint) {
	filter.AddOutPoint(op)
}
