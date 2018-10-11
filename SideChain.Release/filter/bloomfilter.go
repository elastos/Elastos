package filter

import (
	"bytes"
	"fmt"

	"github.com/elastos/Elastos.ELA.SideChain/bloom"
	"github.com/elastos/Elastos.ELA.SideChain/types"

	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

// Ensure bloomFilter implement TxFilter interface.
var _ TxFilter = (*bloomFilter)(nil)

type bloomFilter struct {
	filter *bloom.Filter
}

func (f *bloomFilter) Load(filter []byte) error {
	var fl msg.FilterLoad
	err := fl.Deserialize(bytes.NewReader(filter))
	if err != nil {
		return err
	}

	f.filter = bloom.LoadFilter(&fl)

	return nil
}

func (f *bloomFilter) Add(filter []byte) error {
	if f.filter == nil || !f.filter.IsLoaded() {
		return fmt.Errorf("bloomFilter not loaded")
	}

	f.filter.Add(filter)

	return nil
}

func (f *bloomFilter) Match(tx *types.Transaction) bool {
	return f.filter.MatchTxAndUpdate(tx)
}

func newBloomFilter() *bloomFilter {
	return &bloomFilter{}
}
