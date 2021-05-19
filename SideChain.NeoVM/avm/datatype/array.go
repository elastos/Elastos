package datatype

import (
	"math/big"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/interfaces"
)

type Array struct {
	items []StackItem
}

func NewArray(value []StackItem) *Array{
	var a Array
	a.items = value
	return &a
}

func (a *Array) Equals(other StackItem) bool{
	if _, ok := other.(*Array); !ok {
		return false
	}
	a1 := a.items
	a2 := other.GetArray()
	l1 := len(a1)
	l2 := len(a2)
	if l1 != l2 { return false }
	for i := 0; i<l1; i++ {
		if !a1[i].Equals(a2[i]) {
			return false
		}
	}
	return true

}

func (a *Array) GetBigInteger() *big.Int{
	data := big.NewInt(0)
	if len(a.items) == 0 {
		return data
	}
	data.SetBytes(a.GetByteArray())
	return data
}

func (a *Array) GetBoolean() bool{
	if len(a.items) == 0 { return false }
	return true
}

func (a *Array) GetByteArray() []byte{
	data := []byte{}
	for i := 0; i < len(a.items); i++ {
		data = append(data, a.items[i].GetByteArray()...)
	}
	return data
}

func (a *Array) GetInterface() interfaces.IGeneralInterface {
	return nil
}

func (a *Array) GetArray() []StackItem{
	return a.items
}

func (a *Array) GetMap() map[StackItem]StackItem {
	return nil
}

func (a *Array) Reverse() {
	l := len(a.items)
	items :=  make([]StackItem,0)
	for i := l -1; i >= 0; i-- {
		items = append(items, a.items[i])
	}
	a.items = items
}

func (a *Array) String() string {
	len := len(a.items)
	data := "["
	for i := 0; i < len; i++ {
		data += a.items[i].String()
		if i < len - 1 {
			data +=  ","
		}
	}
	data += "]"
	return data
}