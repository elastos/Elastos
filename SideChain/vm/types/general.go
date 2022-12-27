package types

import (
	"github.com/elastos/Elastos.ELA.SideChain/vm/interfaces"
	"math/big"
)

type GeneralInterface struct {
	object interfaces.IGeneralInterface
}

func NewGeneralInterface(value interfaces.IGeneralInterface) *GeneralInterface {
	var ii GeneralInterface
	ii.object = value
	return &ii
}

func (ii *GeneralInterface) Equals() bool {
	return false
}

func (ii *GeneralInterface) GetBigInteger() big.Int {
	return big.Int{}
}

func (ii *GeneralInterface) GetBoolean() bool {
	if ii.object == nil {
		return false
	}
	return true
}

func (ii *GeneralInterface) GetByteArray() []byte {
	return ii.object.Bytes()
}

func (ii *GeneralInterface) GetInterface() {
}

func (ii *GeneralInterface) GetArray() []StackItem {
	return []StackItem{}
}
