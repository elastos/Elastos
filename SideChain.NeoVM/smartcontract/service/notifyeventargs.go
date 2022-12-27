package service

import (
	"github.com/elastos/Elastos.ELA/common"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/interfaces"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/datatype"
)

type NotifyEventArgs struct {
	container interfaces.IDataContainer
	scriptHash common.Uint168
	stateItem datatype.StackItem
}

func (ent *NotifyEventArgs) GetContainer() interfaces.IDataContainer {
	return ent.container
}

func (ent *NotifyEventArgs) GetScriptHash() common.Uint168{
	return ent.scriptHash
}

func (ent *NotifyEventArgs) GetStateItem() datatype.StackItem{
	return ent.stateItem
}