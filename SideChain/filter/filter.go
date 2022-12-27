package filter

import (
	"fmt"
	"sync"

	"github.com/elastos/Elastos.ELA.SideChain/types"

	"github.com/elastos/Elastos.ELA/p2p/msg"
)

type TxFilterType uint8

const (
	// FTBloom indicates the TxFilter's Filter is a bloom filter.
	FTBloom = iota

	// FTTxType indicates the TxFilter's Filter is a transaction type filter.
	FTTxType
)

// Map of tx filter types back to their constant names for pretty printing.
var tftStrings = map[TxFilterType]string{
	FTBloom:  "FTBloom",
	FTTxType: "FTTxType",
}

// String returns the TxFilterType in human-readable form.
func (f TxFilterType) String() string {
	s, ok := tftStrings[f]
	if ok {
		return s
	}
	return fmt.Sprintf("FTType%d", f)
}

// TxFilter indicates the methods a transaction filter should implement.
type TxFilter interface {
	// Load loads the transaction filter.
	Load(filter []byte) error

	// Add adds new data into filter.
	Add(data []byte) error

	// Match returns if a transaction matches the filter.
	Match(tx *types.Transaction) bool
}

type Filter struct {
	newFilter func(TxFilterType) TxFilter

	mtx    sync.Mutex
	filter TxFilter
}

func (f *Filter) load(filter *msg.TxFilterLoad) error {
	filterType := TxFilterType(filter.Type)

	tf := f.newFilter(filterType)
	if tf == nil {
		return fmt.Errorf("unknown txfilter type %s", filterType)
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

func (f *Filter) Match(tx *types.Transaction) bool {
	f.mtx.Lock()
	match := f.filter.Match(tx)
	f.mtx.Unlock()
	return match
}

func New(newFilter func(filterType TxFilterType) TxFilter) *Filter {
	return &Filter{newFilter: newFilter}
}
