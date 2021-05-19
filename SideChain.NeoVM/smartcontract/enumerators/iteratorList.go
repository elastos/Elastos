package enumerators

import (
	"io"
	"container/list"

	"github.com/elastos/Elastos.ELA/common"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/datatype"
)

type ListIterator struct {
	keylist   *list.List
	valueList *list.List
	keycur    *list.Element
	valuecur  *list.Element
}

func NewListIterator() *ListIterator {
	iter := &ListIterator{nil, nil, nil, nil}
	iter.keylist = list.New()
	iter.valueList = list.New()
	iter.Add(nil, nil)
	return iter
}

func (iter *ListIterator) HashNext() bool {
	if iter.keycur.Next() != nil {
		iter.keycur.Prev()
		return true
	}
	iter.keycur.Prev()
	return false
}

func (iter *ListIterator) Add(key interface{}, value interface{}) {
	iter.keylist.PushBack(key)
	iter.valueList.PushBack(value)
	if iter.keycur == nil {
		iter.keycur = iter.keylist.Front()
	}
	if iter.valuecur == nil {
		iter.valuecur = iter.valueList.Front()
	}
}

func (iter *ListIterator) Next() bool {
	if iter.keycur.Next() != nil {
		iter.keycur = iter.keycur.Next()
		iter.valuecur = iter.valuecur.Next()
		return true
	}
	return false
}

func (iter *ListIterator) Prev() bool {
	if iter.keycur.Prev() != nil {
		iter.keycur = iter.keycur.Prev()
		iter.valuecur = iter.valuecur.Prev()
		return true
	}
	return false
}

func (iter *ListIterator) First() bool {
	if iter.keylist.Front() != nil {
		iter.keycur = iter.keylist.Front()
		iter.valuecur = iter.valueList.Front()
		return true
	}
	return false
}

func (iter *ListIterator) Last() bool {
	if iter.keylist.Back() != nil {
		iter.keycur = iter.keylist.Back()
		iter.valuecur = iter.valueList.Back()
		return true
	}
	return false
}

func (iter *ListIterator) Seek(key []byte) bool {
	for re := iter.First() ; re ; re = iter.Next(){
		if string(iter.Key()) == string(key) {
			return true
		}
	}
	return false
}

func (iter *ListIterator) Key() []byte {
	if iter.keycur != nil {
		stackItem, err := avm.NewStackItem(iter.keycur.Value)
		if err != nil {
			return nil
		}
		return stackItem.GetByteArray()
	}
	return nil
}
func (iter *ListIterator) Value() []byte {
	if iter.valuecur != nil {
		value := iter.valuecur.Value
		stackItem, err := avm.NewStackItem(value)
		if err != nil {
			return nil
		}
		byteData := stackItem.GetByteArray()
		switch value.(type) {
		case *datatype.Integer:
			byteData = common.BytesReverse(byteData)
		}
		return byteData
	}
	return nil
}

func (iter *ListIterator) Release() {
	var next *list.Element
	for e := iter.keylist.Front(); e != nil; e = next {
		next = e.Next()
		iter.keylist.Remove(e)
	}
	for e := iter.valueList.Front(); e != nil; e = next {
		next = e.Next()
		iter.valueList.Remove(e)
	}
	iter.keycur = nil
	iter.valuecur = nil
}

func (iter *ListIterator) Serialize(w io.Writer) error {
	common.WriteVarBytes(w, iter.Value())
	return nil
}
func (iter *ListIterator) Deserialize(r io.Reader) error {
	common.ReadVarBytes(r, common.MaxVarStringLength, "IteratorKeys Deserialize")
	return nil
}