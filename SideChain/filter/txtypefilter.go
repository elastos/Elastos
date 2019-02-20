package filter

import (
	"fmt"

	"github.com/elastos/Elastos.ELA.SideChain/types"
)

const (
	maxFilterTxTypes = 15
)

type txTypeFilter struct {
	txTypes map[types.TxType]struct{}
}

func (f *txTypeFilter) Load(filter []byte) error {
	size := len(filter)
	if size == 0 {
		return fmt.Errorf("txTypeFilter load filter is empty")
	}

	if size > maxFilterTxTypes {
		return fmt.Errorf("txTypeFilter load filter too large "+
			"[size %v, max %v]", size, maxFilterTxTypes)
	}

	f.txTypes = make(map[types.TxType]struct{})
	for i := 0; i < size; i++ {
		f.txTypes[types.TxType(filter[i])] = struct{}{}
	}

	return nil
}

func (f *txTypeFilter) Add(filter []byte) error {
	if f.txTypes == nil {
		return fmt.Errorf("txTypeFilter not loaded")
	}

	size := len(filter)
	if size == 0 {
		return fmt.Errorf("txTypeFilter add filter is empty")
	}

	if size != 1 {
		return fmt.Errorf("txTypeFilter add more than one txType")
	}

	if _, ok := f.txTypes[types.TxType(filter[0])]; ok {
		return fmt.Errorf("txTypeFilter add duplicate txType")
	}

	if len(f.txTypes)+1 > maxFilterTxTypes {
		return fmt.Errorf("txTypeFilter add filter too large "+
			"[size %v, max %v]", size, maxFilterTxTypes)
	}

	f.txTypes[types.TxType(filter[0])] = struct{}{}

	return nil
}

func (f *txTypeFilter) Match(tx *types.Transaction) bool {
	if f.txTypes == nil {
		return false
	}

	_, ok := f.txTypes[tx.TxType]
	return ok
}

func NewTxTypeFilter() TxFilter {
	return &txTypeFilter{}
}
