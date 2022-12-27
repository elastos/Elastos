package datatype

import (
	"math/big"
	"bytes"

	"github.com/elastos/Elastos.ELA/common"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/interfaces"
)

type GeneralInterface struct {
	object interfaces.IGeneralInterface
}

func NewGeneralInterface(value interfaces.IGeneralInterface) *GeneralInterface {
	var ii GeneralInterface
	ii.object = value
	return &ii
}

func (ii *GeneralInterface) Equals(other StackItem) bool {
	return ii.object == other.(interfaces.IGeneralInterface)
}

func (ii *GeneralInterface) GetBigInteger() *big.Int {
	data := big.NewInt(0)
	data.SetBytes(ii.GetByteArray())
	return data
}

func (ii *GeneralInterface) GetBoolean() bool {
	if ii.object == nil {
		return false
	}
	return true
}

func (ii *GeneralInterface) GetByteArray() []byte {
	if ii.object == nil {
		return []byte{}
	}
	buffer := new(bytes.Buffer)
	ii.object.Serialize(buffer)
	return buffer.Bytes()
}

func (ii *GeneralInterface) GetInterface() interfaces.IGeneralInterface {
	return ii.object
}

func (ii *GeneralInterface) GetArray() []StackItem {
	return []StackItem{}
}

func (ii *GeneralInterface) GetMap() map[StackItem]StackItem {
	return nil
}

func (ii *GeneralInterface) String() string {
	return common.BytesToHexString(ii.GetByteArray())
}