package enumerators

import "github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/datatype"

type IEnumerator interface {
	Next() bool
	Value() datatype.StackItem
	Dispose()
}