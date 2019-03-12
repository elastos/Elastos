/*
Side filter is a filter of for SideChain SPV module, it filters transactions
that will change DPOS producers state like RegisterProducer, CancelProducer etc.
and also the transactions related to the SideChain addresses.
*/

package sidefilter

import (
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/dpos/state"
	"github.com/elastos/Elastos.ELA/elanet/bloom"
	"github.com/elastos/Elastos.ELA/elanet/filter"
)

// Ensure Filter implements the TxFilter interface.
var _ filter.TxFilter = (*Filter)(nil)

// Filter defines the side filter instance, it implements the TxFilter
// interface.
type Filter struct {
	bloom.TxFilter
	state *state.State
}

// Load loads the transaction filter.
func (f *Filter) Load(filter []byte) error {
	return f.TxFilter.Load(filter)
}

// Add adds new data into filter.
func (f *Filter) Add(data []byte) error {
	return f.TxFilter.Add(data)
}

// Match returns if a transaction matches the filter.
func (f *Filter) Match(tx *types.Transaction) bool {
	return f.TxFilter.Match(tx) || f.state.IsDPOSTransaction(tx)
}

// New returns a new Filter instance.
func New(state *state.State) *Filter {
	return &Filter{state: state}
}
