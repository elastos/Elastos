package enumerators

import (
	"github.com/elastos/Elastos.ELA.SideChain/database"
)

type IteratorKeys struct {
	iter database.Iterator
}

func NewIteratorKeys(iter database.Iterator) *IteratorKeys {
	var iterKeys IteratorKeys
	iterKeys.iter = iter
	return &iterKeys
}

func (iter *IteratorKeys) Next() bool {
	return iter.iter.Next()
}

func (iter *IteratorKeys) Value() []byte {
	return iter.iter.Key()
}

func (iter *IteratorKeys) Dispose()  {
	iter.iter.Release()
}

func (iter *IteratorKeys) Bytes() []byte {
	return iter.iter.Key()
}