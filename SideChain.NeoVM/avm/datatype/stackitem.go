package datatype

import (
	"math/big"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/interfaces"
)

type StackItem interface {
	Equals(other StackItem) bool
	GetBigInteger() *big.Int
	GetBoolean() bool
	GetByteArray() []byte
	GetInterface() interfaces.IGeneralInterface
	GetArray() []StackItem
	GetMap() map[StackItem]StackItem
	String() string
}
