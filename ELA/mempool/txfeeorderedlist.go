package mempool

import (
	"fmt"
	"sort"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/errors"
)

type PopBackEvent func(common.Uint256)

var (
	addingTxExcluded = errors.SimpleWithMessage(errors.ErrTxPoolFailure,
		nil, "new tx excluded")
)

// txItem defines essential data about ordering a tx
type txItem struct {
	Hash    common.Uint256
	FeeRate float64
	Size    int
}

// txFeeOrderedList maintains a list of tx hashes order
type txFeeOrderedList struct {
	list      []txItem
	totalSize uint64
	maxSize   uint64
	onPopBack PopBackEvent
}

func (l *txFeeOrderedList) AddTx(tx *types.Transaction) errors.ELAError {
	size := tx.GetSize()
	if size <= 0 {
		return errors.SimpleWithMessage(errors.ErrTxPoolFailure, nil,
			fmt.Sprintf("tx %s got illegal size", tx.Hash().String()))
	}

	feeRate := float64(tx.Fee) / float64(size)
	overSize := l.OverSize(uint64(size))

	if overSize && len(l.list) > 0 &&
		// if fee rate of this tx less than the last one, then return directly
		feeRate < l.list[len(l.list)-1].FeeRate {
		return addingTxExcluded
	}

	l.compareAndInsert(txItem{
		Hash:    tx.Hash(),
		FeeRate: feeRate,
		Size:    size,
	})

	for overSize {
		item := l.popBack()
		l.totalSize -= uint64(item.Size)
		l.onPopBack(item.Hash)

		overSize = l.OverSize(0)
	}
	return nil
}

func (l *txFeeOrderedList) RemoveTx(hash common.Uint256, txSize uint64) {
	for i, v := range l.list {
		if v.Hash.IsEqual(hash) {
			copy(l.list[i:], l.list[i+1:])
			l.list = l.list[:len(l.list)-1]
			l.totalSize -= txSize
			return
		}
	}
}

func (l *txFeeOrderedList) GetSize() int {
	return len(l.list)
}

func (l *txFeeOrderedList) OverSize(size uint64) bool {
	return l.totalSize+size > l.maxSize
}

func (l *txFeeOrderedList) compareAndInsert(item txItem) {
	index := sort.Search(len(l.list), func(i int) bool {
		return l.list[i].FeeRate < item.FeeRate
	})
	l.list = append(l.list, txItem{})
	copy(l.list[index+1:], l.list[index:])
	l.list[index] = item

	l.totalSize += uint64(item.Size)
}

func (l *txFeeOrderedList) popBack() txItem {
	rtn := l.list[len(l.list)-1]
	l.list = l.list[:len(l.list)-1]
	return rtn
}

func newTxFeeOrderedList(onPopBack PopBackEvent,
	maxSize uint64) *txFeeOrderedList {
	return &txFeeOrderedList{
		list:      []txItem{},
		totalSize: 0,
		onPopBack: onPopBack,
		maxSize:   maxSize,
	}
}
