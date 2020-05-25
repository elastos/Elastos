// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package blockchain

import (
	"github.com/syndtr/goleveldb/leveldb/iterator"
)

type Iterator struct {
	iter iterator.Iterator
}

func (it *Iterator) Next() bool {
	return it.iter.Next()
}

func (it *Iterator) Prev() bool {
	return it.iter.Prev()
}

func (it *Iterator) First() bool {
	return it.iter.First()
}

func (it *Iterator) Last() bool {
	return it.iter.Last()
}

func (it *Iterator) Seek(key []byte) bool {
	return it.iter.Seek(key)
}

func (it *Iterator) Key() []byte {
	return it.iter.Key()
}

func (it *Iterator) Value() []byte {
	return it.iter.Value()
}

func (it *Iterator) Release() {
	it.iter.Release()
}
