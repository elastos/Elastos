package avm

import (
	"errors"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/datatype"
)

func opArraySize(e *ExecutionEngine) (VMState, error) {
	if e.evaluationStack.Count() < 1 {
		return FAULT, nil
	}
	itemArr := PopStackItem(e)
	if _, ok := itemArr.(*datatype.Array); ok {
		arr := itemArr.GetArray()
		err := pushData(e, len(arr))
		if err != nil {
			return FAULT, err
		}
	} else if _, ok := itemArr.(*datatype.ByteArray); ok {
		arr := itemArr.GetByteArray()
		err := pushData(e, len(arr))
		if err != nil {
			return FAULT, err
		}
	}
	return NONE, nil
}

func opPack(e *ExecutionEngine) (VMState, error) {
	if e.evaluationStack.Count() < 1 {
		return FAULT, nil
	}
	size := int(AssertStackItem(e.evaluationStack.Pop()).GetBigInteger().Int64())
	if size < 0 || size > e.evaluationStack.Count() {
		return FAULT, nil
	}
	items := NewStackItems()
	for {
		if size == 0 {
			break
		}
		items = append(items, AssertStackItem(e.evaluationStack.Pop()))
		size--
	}
	err := pushData(e, items)
	if err != nil {
		return FAULT, err
	}
	return NONE, nil
}

func opUnpack(e *ExecutionEngine) (VMState, error) {
	if e.evaluationStack.Count() < 1 {
		return FAULT, nil
	}
	arr := AssertStackItem(e.evaluationStack.Pop()).GetArray()
	l := len(arr)
	for i := l - 1; i >= 0; i-- {
		e.evaluationStack.Push(arr[i])
	}
	err := pushData(e, l)
	if err != nil {
		return FAULT, err
	}
	return NONE, nil
}

func opPickItem(e *ExecutionEngine) (VMState, error) {
	key := PopStackItem(e)
	itemArr := PopStackItem(e)
	if _, ok := itemArr.(*datatype.Array); ok {
		index := key.GetBigInteger()
		items := itemArr.GetArray()
		PushData(e, items[index.Int64()])
	}else if _,ok := itemArr.(*datatype.Dictionary); ok {
		items := itemArr.(*datatype.Dictionary).GetValue(key)
		PushData(e, items)
	} else {
		//put bytearray, if not the data is error. some publickey
		items := itemArr.GetByteArray()
		PushData(e, items)
	}
	return NONE, nil
}

func opSetItem(e *ExecutionEngine) (VMState, error) {
	if e.evaluationStack.Count() < 3 {
		return FAULT, errors.New("evaluationStack error")
	}
	newItem := PopStackItem(e)
	key := PopStackItem(e)
	itemArr := PopStackItem(e)
	if _, ok := itemArr.(*datatype.Array); ok {
		index := key.GetBigInteger()
		items := itemArr.GetArray()
		items[index.Int64()] = newItem
	} else if _,ok := itemArr.(*datatype.Dictionary); ok {
		itemArr.(*datatype.Dictionary).PutStackItem(key, newItem)
	} else {
		items := itemArr.GetByteArray()
		index := key.GetBigInteger()
		items[index.Int64()] = newItem.GetByteArray()[0]
	}
	return NONE, nil
}

func opNewArray(e *ExecutionEngine) (VMState, error) {
	count := PopInt(e)
	items := NewStackItems()
	for i := 0; i < count; i++ {
		items = append(items, datatype.NewBoolean(false))
	}
	PushData(e, items)
	return NONE, nil
}

func opAppend(e *ExecutionEngine) (VMState, error) {
	newItem := PopStackItem(e)
	itemArr := PopStackItem(e)
	if _, ok := itemArr.(*datatype.Array); ok {
		items := itemArr.GetArray()
		items = append(items, newItem)
	} else {
		return  FAULT, errors.New("opAppend data error")
	}
	return NONE, nil
}

func opReverse(e *ExecutionEngine) (VMState, error) {
	items := PopStackItem(e)
	if _, ok := items.(*datatype.Array); ok {
		items.(*datatype.Array).Reverse()
	} else {
		return FAULT, errors.New("opReverse type error")
	}
	return NONE, nil
}

func opRemove(e *ExecutionEngine) (VMState, error) {
	key := PopStackItem(e)
	itemArr := PopStackItem(e)
	if _, ok := itemArr.(*datatype.Array); ok {
		items := itemArr.(*datatype.Array).GetArray()
		index := key.GetBigInteger().Int64()
		if index < 0 || int(index) >= len(items) {
			return FAULT, errors.New("opRemove index error")
		}
		items = append(items[:index], items[index + 1:]...)
	} else if _,ok := itemArr.(*datatype.Dictionary); ok {
		items := itemArr.(*datatype.Dictionary)
		items.Remove(key)
	} else {
		return FAULT, errors.New("opRemove type error")
	}
	return NONE, nil
}

func opHasKey(e *ExecutionEngine) (VMState, error) {
	key := PopStackItem(e)
	itemArr := PopStackItem(e)
	if _, ok := itemArr.(*datatype.Array); ok {
		index := key.GetBigInteger().Int64()
		if index < 0 {
			return FAULT, errors.New("opHasKey index error")
		}
		items := itemArr.(*datatype.Array).GetArray()
		pushData(e, int(index) < len(items))
	} else if _,ok := itemArr.(*datatype.Dictionary); ok {
		items := itemArr.(*datatype.Dictionary)
		pushData(e, items.GetValue(key) != nil)
	} else {
		return FAULT, errors.New("opHasKey type error")
	}
	return NONE, nil
}