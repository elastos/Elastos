package enumerators

import (
	"io"

	"github.com/elastos/Elastos.ELA/common"

	"github.com/elastos/Elastos.ELA.SideChain/database"
)

type Iterator struct {
	iter database.Iterator
}

func NewIterator(iter database.Iterator) *Iterator {
	var iterator Iterator
	iterator.iter = iter
	return &iterator
}

func (iter *Iterator) Prev() bool {
	return iter.iter.Prev()
}

func (iter *Iterator) Seek(key []byte) bool {
	return iter.iter.Seek(key)
}

func (iter *Iterator) Last() bool {
	return iter.iter.Last()
}

func (iter *Iterator) First() bool {
	return iter.iter.First()
}

func (iter *Iterator) Next() bool {
	return iter.iter.Next()
}

func (iter *Iterator) Key() []byte {
	return iter.iter.Key()
}

func (iter *Iterator) Value() []byte {
	return iter.iter.Value()
}

func (iter *Iterator) Dispose()  {
	iter.iter.Release()
}

func (iter *Iterator) Release()  {
	iter.iter.Release()
}

func (iter *Iterator) Serialize(w io.Writer) error {
	common.WriteVarBytes(w, iter.Key())
	common.WriteVarBytes(w, iter.Value())
	return nil
}
func (iter *Iterator) Deserialize(r io.Reader) error {
	common.ReadVarBytes(r, common.MaxVarStringLength, "Iterator Deserialize")
	common.ReadVarBytes(r, common.MaxVarStringLength, "Iterator Deserialize")
	return nil
}