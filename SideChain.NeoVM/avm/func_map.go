package avm

import (
	"errors"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/datatype"
)

func opNewMap(e *ExecutionEngine) (VMState, error) {
	e.GetEvaluationStack().Push(datatype.NewDictionary())
	return NONE, nil
}

func opKeys(e *ExecutionEngine) (VMState, error) {
	items := PopStackItem(e)
	if _,ok := items.(*datatype.Dictionary); ok {
		PushData(e, items.(*datatype.Dictionary).GetKeys())
	} else {
		return FAULT, errors.New("opKeys type error")
	}
	return NONE, nil
}

func opValues(e *ExecutionEngine) (VMState, error) {
	itemArr := PopStackItem(e)
	values := make([]datatype.StackItem, 0)
	if _, ok := itemArr.(*datatype.Array); ok {
		items := itemArr.(*datatype.Array).GetArray()
		values = append(values, items...)
	}else if _,ok := itemArr.(*datatype.Dictionary); ok {
		items := itemArr.(*datatype.Dictionary).GetValues()
		values = append(values, items.GetArray()...)
	} else {
		return FAULT, errors.New("opValues type error")
	}
	PushData(e, values)
	return NONE, nil
}