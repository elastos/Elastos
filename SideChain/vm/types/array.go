package types

import (
	"math/big"
)

type Array struct {
	items []StackItem
}

func NewArray(value []StackItem) *Array {
	var a Array
	a.items = value
	return &a
}

func (a *Array) Equals(other StackItem) bool {
	if _, ok := other.(*Array); !ok {
		return false
	}
	a1 := a.items
	a2 := other.GetArray()
	l1 := len(a1)
	l2 := len(a2)
	if l1 != l2 {
		return false
	}
	for i := 0; i < l1; i++ {
		if !a1[i].Equals(a2[i]) {
			return false
		}
	}
	return true

}

func (a *Array) GetBigInteger() *big.Int {
	if len(a.items) == 0 {
		return big.NewInt(0)
	}
	return a.items[0].GetBigInteger()
}

func (a *Array) GetBoolean() bool {
	if len(a.items) == 0 {
		return false
	}
	return a.items[0].GetBoolean()
}

func (a *Array) GetByteArray() []byte {
	return []byte{}
}

func (a *Array) GetInterface() {

}

func (a *Array) GetArray() []StackItem {
	return a.items
}
