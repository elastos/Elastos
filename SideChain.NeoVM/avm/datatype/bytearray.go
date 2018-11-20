package datatype

import (
	"math/big"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/interfaces"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type ByteArray struct {
	value []byte
}

func NewByteArray(value []byte) *ByteArray {
	var ba ByteArray
	ba.value = value
	return &ba
}

func (ba *ByteArray) Equals(other StackItem) bool {
	if _, ok := other.(*ByteArray); !ok {
		return false
	}
	a1 := ba.value
	a2 := other.GetByteArray()
	l1 := len(a1)
	l2 := len(a2)
	if l1 != l2 {
		return false
	}
	for i:=0; i<l1; i++ {
		if a1[i] != a2[i] {
			return false
		}
	}
	return true
}

func (ba *ByteArray) GetBigInteger() *big.Int {
	var bi big.Int
	tempBytes := make([]byte, len(ba.value))
	copy(tempBytes, ba.value)
	return bi.SetBytes(common.BytesReverse(tempBytes))
}

func (ba *ByteArray) GetBoolean() bool{
	for _, b := range ba.value {
		if b != 0 {
			return true
		}
	}
	return false
}

func (ba *ByteArray) GetByteArray() []byte {
	return ba.value
}

func (ba *ByteArray) GetInterface() interfaces.IGeneralInterface {
	return nil
}

func (ba *ByteArray) GetArray() []StackItem {
	return []StackItem{ba}
}

func (ba *ByteArray) Bytes() []byte {
	return ba.GetByteArray()
}

func (ba *ByteArray) GetMap() map[StackItem]StackItem {
	return nil
}