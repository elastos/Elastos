package filter

import (
	"fmt"
	"sync"

	"github.com/elastos/Elastos.ELA.SideChain/types"

	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

type TxFilter interface {
	Load(filter []byte) error
	Add(filter []byte) error
	Match(tx *types.Transaction) bool
}

type Filter struct {
	mtx               sync.Mutex
	registeredFilters map[msg.TxFilterType]func() TxFilter
	filters           map[msg.TxFilterType]TxFilter
}

func (f *Filter) RegisterTxFilter(filterType msg.TxFilterType, newFilter func() TxFilter) {
	f.mtx.Lock()
	f.registeredFilters[filterType] = newFilter
	f.mtx.Unlock()
}

func (f *Filter) update(filter *msg.TxFilter) error {
	if filter.Op == msg.OpClearAll {
		if len(f.filters) == 0 {
			return fmt.Errorf("filter not loaded")
		}

		f.filters = make(map[msg.TxFilterType]TxFilter)
		return nil
	}

	switch filter.Op {
	case msg.OpFilterLoad:
		newFilter, ok := f.registeredFilters[filter.Type]
		if !ok {
			return fmt.Errorf("unknown txfilter type %s", filter.Type)
		}

		tf := newFilter()
		err := tf.Load(filter.Data)
		if err != nil {
			return err
		}

		f.filters[filter.Type] = tf

	case msg.OpFilterAdd:
		tf, ok := f.filters[filter.Type]
		if !ok {
			return fmt.Errorf("filter %s not loaded", filter.Type)
		}
		return tf.Add(filter.Data)

	case msg.OpFilterClear:
		_, ok := f.filters[filter.Type]
		if !ok {
			return fmt.Errorf("filter %s not loaded", filter.Type)
		}

		delete(f.filters, filter.Type)
	}

	return fmt.Errorf("unknown txfilter op %s", filter.Op)
}

func (f *Filter) Update(filter *msg.TxFilter) error {
	f.mtx.Lock()
	err := f.update(filter)
	f.mtx.Unlock()
	return err
}

func (f *Filter) match(tx *types.Transaction) bool {
	for _, tf := range f.filters {
		if tf.Match(tx) {
			return true
		}
	}
	return false
}

func (f *Filter) IsLoaded() bool {
	f.mtx.Lock()
	loaded := len(f.filters) > 0
	f.mtx.Unlock()
	return loaded
}

func (f *Filter) Match(tx *types.Transaction) bool {
	f.mtx.Lock()
	match := f.match(tx)
	f.mtx.Unlock()
	return match
}

func New() *Filter {
	return &Filter{
		registeredFilters: make(map[msg.TxFilterType]func() TxFilter),
		filters:           make(map[msg.TxFilterType]TxFilter),
	}
}
