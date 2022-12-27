package datatype

import (
	"math/big"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/interfaces"
)

type Dictionary struct {
	dic map[StackItem]StackItem
}

func NewDictionary() *Dictionary {
	var dictionary Dictionary
	dictionary.dic = make(map[StackItem]StackItem)
	return &dictionary
}

func (dic *Dictionary) GetValue(key StackItem) StackItem {
	for temp := range dic.dic {
		if temp.Equals(key) {
			return dic.dic[temp]
		}
	}
	return nil
}

func (dic *Dictionary) Remove(key StackItem) {
	for temp := range dic.dic {
		if temp.Equals(key) {
			delete(dic.dic, temp)
		}
	}
}

func (dic *Dictionary) GetKeys() *Array {
	items := make([]StackItem, 0)
	for temp := range dic.dic {
		items = append(items, temp)
	}
	return NewArray(items)
}

func (dic *Dictionary) GetValues() *Array  {
	items := make([]StackItem, 0)
	for temp := range dic.dic {
		items = append(items, dic.dic[temp])
	}
	return NewArray(items)
}

func (dic *Dictionary) Equals(other StackItem) bool {
	otherMap := other.GetMap()
	for key := range dic.dic {
		if dic.dic[key].Equals(otherMap[key]) == false {
			return false
		}
	}
	return true
}

func (dic *Dictionary) GetMap() map[StackItem]StackItem {
	return dic.dic
}

func (dic *Dictionary) PutStackItem(key, value StackItem)  {
	dic.dic[key] = value
}

func (dic *Dictionary) GetBoolean() bool {
	return false
}

func (dic *Dictionary) GetByteArray() []byte {
	return []byte{}
}

func (dic *Dictionary) GetInterface() interfaces.IGeneralInterface {
	return nil
}

func (dic *Dictionary) GetArray() []StackItem {
	return nil
}

func (dic *Dictionary) GetBigInteger() *big.Int {
	return big.NewInt(0)
}

func (dic *Dictionary) String() string {
	str := "{"
	for temp := range dic.dic {
		str += temp.String() + ":" + dic.dic[temp].String() + ","
	}
	str += "}"
	return str
}