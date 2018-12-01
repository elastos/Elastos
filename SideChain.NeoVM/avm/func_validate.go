package avm

import (
	"bytes"
	"encoding/binary"
	"math/big"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/errors"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/datatype"
)

func validatorPushData4(e *ExecutionEngine) error {
	index := e.context.GetInstructionPointer()
	if index + 4 >= len(e.context.Script) {
		return errors.ErrOverCodeLen
	}
	bytesBuffer := bytes.NewBuffer(e.context.Script[index: index + 4])
	var l uint32
	binary.Read(bytesBuffer, binary.LittleEndian, &l)
	if l > MaxItemSize {
		return errors.ErrOverMaxItemSize
	}
	return nil
}

func validateCall(e *ExecutionEngine) error {
	if err := validateInvocationStack(e); err != nil {
		return err
	}
	return nil
}

func validateAppCall(e *ExecutionEngine) error {
	if err := validateInvocationStack(e); err != nil {
		return err
	}
	if e.table == nil {
		return errors.ErrTableIsNil
	}
	return nil
}

func validateSysCall(e *ExecutionEngine) error {
	if e.service == nil {
		return errors.ErrServiceIsNil
	}
	return nil
}

func validateInvocationStack(e *ExecutionEngine) error {
	if uint32(e.invocationStack.Count()) > MAXInvocationStackSize {
		return errors.ErrOverStackLen
	}
	return nil
}

func validateCat(e *ExecutionEngine) error {
	if EvaluationStackCount(e) < 2 {
		return errors.ErrUnderStackLen
	}
	l := len(PeekNByteArray(0, e)) + len(PeekNByteArray(1, e))
	if uint32(l) > MaxItemSize {
		return errors.ErrOverMaxItemSize
	}
	return nil
}

func validateSubStr(e *ExecutionEngine) error {
	if EvaluationStackCount(e) < 3 {
		return errors.ErrUnderStackLen
	}
	count := PeekNInt(0, e)
	if count < 0 {
		return errors.ErrBadValue
	}
	index := PeekNInt(1, e)
	if index < 0 {
		return errors.ErrBadValue
	}
	arr := PeekNByteArray(2, e)
	if len(arr) < index + count {
		return errors.ErrOverMaxArraySize
	}
	return nil
}

func validateLeft(e *ExecutionEngine) error {
	if EvaluationStackCount(e) < 2 {
		return errors.ErrUnderStackLen
	}
	count := PeekNInt(0, e)
	if count < 0 {
		return errors.ErrBadValue
	}
	arr := PeekNByteArray(1, e)
	if len(arr) < count {
		return errors.ErrOverMaxArraySize
	}
	return nil
}

func validateRight(e *ExecutionEngine) error {
	if EvaluationStackCount(e) < 2 {
		return errors.ErrUnderStackLen
	}
	count := PeekNInt(0, e)
	if count < 0 {
		return errors.ErrBadValue
	}
	arr := PeekNByteArray(1, e)
	if len(arr) < count {
		return errors.ErrOverMaxArraySize
	}
	return nil
}

func validatorBigIntComp(e *ExecutionEngine) error {
	if EvaluationStackCount(e) < 2 {
		return errors.ErrUnderStackLen
	}
	return nil
}

func validatePickItem(e *ExecutionEngine) error {
	if EvaluationStackCount(e) < 2 {
		return errors.ErrUnderStackLen
	}
	key := PeekNStackItem(0, e)
	if key == nil {
		return errors.ErrBadValue
	}
	item :=PeekNStackItem(1, e)
	if item == nil {
		return errors.ErrBadValue
	}
	size := 0
	if _,ok := item.(*datatype.Array); ok {
		index := key.GetBigInteger().Int64()
		size = len(item.GetArray())
		if index >= int64(size) {
			return errors.ErrOverMaxArraySize
		}
	} else if dic, ok := item.(*datatype.Dictionary); ok {
		if dic.GetValue(key) != nil {
			return nil
		}
		size = len(dic.GetMap())
	} else if _, ok := item.(*datatype.ByteArray); ok {
		index := key.GetBigInteger().Int64()
		size = len(item.GetByteArray())
		if index >= int64(size) {
			return errors.ErrOverMaxArraySize
		}
		if size == 0 {
			return errors.ErrBadValue
		}
	} else {
		return errors.ErrBadValue
	}

	if uint32(size) > MaxArraySize {
		return errors.ErrOverMaxArraySize
	}
	return nil
}

func validatorSetItem(e *ExecutionEngine) error {
	if EvaluationStackCount(e) < 3 {
		return errors.ErrUnderStackLen
	}
	newItem := PeekN(0, e)
	if newItem == nil {
		return errors.ErrBadValue
	}
	size := 0
	key := PeekN(1, e).(datatype.StackItem)
	if key == nil {
		return errors.ErrBadValue
	}
	arrItem := PeekN(2, e).(datatype.StackItem)
	if arrItem == nil {
		return errors.ErrBadValue
	}
	if _,ok := arrItem.(*datatype.Array); ok {
		index := key.GetBigInteger().Int64()
		size = len(arrItem.GetArray())
		if index >= int64(size) {
			return errors.ErrOverMaxArraySize
		}
	} else if dic, ok := arrItem.(*datatype.Dictionary); ok {
		if dic.GetValue(key) != nil {
			return nil
		}
		size = len(dic.GetMap())
	} else if _, ok := arrItem.(*datatype.ByteArray); ok {
		index := key.GetBigInteger().Int64()
		size = len(arrItem.GetByteArray())
		if index >= int64(size) {
			return errors.ErrOverMaxArraySize
		}
		if size == 0 {
			return errors.ErrBadValue
		}
	} else {
		return errors.ErrBadValue
	}

	if uint32(size) > MaxArraySize {
		return errors.ErrOverMaxArraySize
	}
	return nil
}

func validateNewArray(e *ExecutionEngine) error {
	if e.evaluationStack.Count() == 0 {
		return errors.ErrOverStackLen
	}
	count := PeekInt(e)
	if uint32(count) > MaxArraySize {
		return errors.ErrOverMaxArraySize
	}
	return nil
}

func validatePack(e *ExecutionEngine) error {
	if e.evaluationStack.Count() == 0 {
		return errors.ErrOverStackLen
	}
	count := PeekInt(e)
	if uint32(count) > MaxArraySize {
		return errors.ErrOverMaxArraySize
	}
	if count > EvaluationStackCount(e) {
		return errors.ErrOverStackLen
	}
	return nil
}

func validateAppend(e *ExecutionEngine) error {
	if e.evaluationStack.Count() < 2 {
		return errors.ErrOverStackLen
	}
	item := PeekNStackItem(1, e)
	array, ok := item.(*datatype.Array)
	if !ok {
		return errors.ErrBadValue
	}
	size := len(array.GetArray()) + 1
	if uint32(size) > MaxArraySize {
		return errors.ErrOverMaxArraySize
	}
	return nil
}

func validateSHL(e *ExecutionEngine) error {
	ishift := PeekNBigInt(0, e).Int64()
	if ishift > MAX_SHL_SHR || ishift < Min_SHL_SHR {
		return errors.ErrBadValue
	}
	x := PeekNBigInt(1, e).Uint64()
	v := x << (uint64(ishift))
	num := new(big.Int)
	num.SetUint64(v)
	if ok, err := checkBigInteger(num); !ok {
		return err
	}
	return nil
}

func validateSHR(e *ExecutionEngine) error {
	ishift := PeekNBigInt(0, e).Int64()
	if ishift > MAX_SHL_SHR || ishift < Min_SHL_SHR {
		return errors.ErrBadValue
	}
	x := PeekNBigInt(1, e).Uint64()
	v := x >> (uint64(ishift))
	num := new(big.Int)
	num.SetUint64(v)
	if ok, err := checkBigInteger(num); !ok {
		return err
	}
	return nil
}

func validateOpStack(e *ExecutionEngine) error {
	if EvaluationStackCount(e) < 1 {
		return errors.ErrUnderStackLen
	}
	index := PeekNInt(0, e)
	if index < 0 {
		return errors.ErrBadValue
	}

	return nil
}

func validateXDrop(e *ExecutionEngine) error {
	if err := validateOpStack(e); err != nil {
		return err
	}
	return nil
}

func validateXSwap(e *ExecutionEngine) error {
	if err := validateOpStack(e); err != nil {
		return err
	}
	return nil
}

func validateXTuck(e *ExecutionEngine) error {
	if err := validateOpStack(e); err != nil {
		return err
	}
	return nil
}

func validatePick(e *ExecutionEngine) error {
	if err := validateOpStack(e); err != nil {
		return err
	}
	return nil
}

func validateRoll(e *ExecutionEngine) error {
	if EvaluationStackCount(e) < 1 {
		return errors.ErrUnderStackLen
	}
	index := PeekNInt(0, e)
	if index < 0 {
		return errors.ErrBadValue
	}
	return nil
}