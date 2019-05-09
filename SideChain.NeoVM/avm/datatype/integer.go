package datatype

import (
	"math/big"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/interfaces"

	"github.com/elastos/Elastos.ELA/common"
)

type Integer struct {
	value *big.Int
}

func NewInteger(value *big.Int) *Integer{
	var  i Integer
	i.value = value
	return &i
}

func (i *Integer) Equals(other StackItem) bool{
	if _, ok := other.(*Integer); !ok {
		return false
	}
	if i.value.Cmp(other.GetBigInteger()) != 0 {
		return false
	}
	return true
}

func (i *Integer) GetBigInteger() *big.Int {
	v := new(big.Int)
	v.Set(i.value)
	return v
}


func (i *Integer) GetBoolean() bool {
	if i.value.Cmp(big.NewInt(0)) == 0 {
		return false
	}
	return true
}

func (i *Integer) GetByteArray() []byte{
	return common.BytesReverse(i.value.Bytes())
}

func (i *Integer) GetInterface() interfaces.IGeneralInterface {
	return nil
}

func (i *Integer) GetArray() []StackItem {
	return []StackItem{i}
}

func (i *Integer) GetMap() map[StackItem]StackItem {
	return nil
}

func (i *Integer) String() string {
	return i.value.Text(10)
}