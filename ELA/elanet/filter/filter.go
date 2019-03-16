package filter

import (
	"fmt"
	"sync"

	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/p2p/msg"
)

const (
	// FTBloom indicates the TxFilter's Filter is a bloom filter.
	FTBloom uint8 = iota

	// FTDPOS indicates the TxFilter's Filter is a DPOS filter.
	FTDPOS
)

// TxFilter indicates the methods a transaction filter should implement.
type TxFilter interface {
	// Load loads the transaction filter.
	Load(filter []byte) error

	// Add adds new data into filter.
	Add(data []byte) error

	// MatchConfirmed returns if a confirmed (packed into a block) transaction
	// matches the filter.
	MatchConfirmed(tx *types.Transaction) bool

	// MatchUnconfirmed returns if a unconfirmed (not packed into a block yet)
	// transaction matches the filter.
	MatchUnconfirmed(tx *types.Transaction) bool
}

type Filter struct {
	newFilter func(uint8) TxFilter

	mtx    sync.Mutex
	filter TxFilter
}

func (f *Filter) load(filter *msg.TxFilterLoad) error {
	filterType := filter.Type

	tf := f.newFilter(filterType)
	if tf == nil {
		return fmt.Errorf("unknown txfilter type %d", filterType)
	}

	err := tf.Load(filter.Data)
	if err != nil {
		return err
	}

	f.filter = tf

	return nil
}

func (f *Filter) Load(filter *msg.TxFilterLoad) error {
	f.mtx.Lock()
	err := f.load(filter)
	f.mtx.Unlock()
	return err
}

func (f *Filter) IsLoaded() bool {
	f.mtx.Lock()
	loaded := f.filter != nil
	f.mtx.Unlock()
	return loaded
}

func (f *Filter) Add(data []byte) error {
	f.mtx.Lock()
	err := f.filter.Add(data)
	f.mtx.Unlock()
	return err
}

func (f *Filter) Clear() {
	f.mtx.Lock()
	f.filter = nil
	f.mtx.Unlock()
}

func (f *Filter) MatchConfirmed(tx *types.Transaction) bool {
	f.mtx.Lock()
	match := f.filter.MatchConfirmed(tx)
	f.mtx.Unlock()
	return match
}

func (f *Filter) MatchUnconfirmed(tx *types.Transaction) bool {
	f.mtx.Lock()
	match := f.filter.MatchUnconfirmed(tx)
	f.mtx.Unlock()
	return match
}

func (f *Filter) Filter() TxFilter {
	f.mtx.Lock()
	filter := f.filter
	f.mtx.Unlock()
	return filter
}

func New(newFilter func(uint8) TxFilter) *Filter {
	return &Filter{newFilter: newFilter}
}
