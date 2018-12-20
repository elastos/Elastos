package avm

import (
	"math/big"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/errors"
)

func opBigInt(e *ExecutionEngine) (VMState, error) {
	if e.evaluationStack.Count() < 1 {
		return FAULT, errors.ErrUnderStackLen
	}
	x := AssertStackItem(e.evaluationStack.Pop()).GetBigInteger()
	if ok, err := checkBigInteger(x); !ok {
		return FAULT, err
	}
	result := BigIntOp(x, e.opCode)
	if ok, err := checkBigInteger(result); !ok {
		return FAULT, err
	}
	err := pushData(e, result)
	if err != nil {
		return FAULT, err
	}
	return NONE, nil
}

func opNot(e *ExecutionEngine) (VMState, error) {
	if e.evaluationStack.Count() < 1 {
		return FAULT, errors.ErrUnderStackLen
	}
	x := AssertStackItem(e.evaluationStack.Pop()).GetBoolean()
	err := pushData(e, !x)
	if err != nil {
		return FAULT, err
	}
	return NONE, nil
}

func opNz(e *ExecutionEngine) (VMState, error) {
	if e.evaluationStack.Count() < 1 {
		return FAULT, errors.ErrUnderStackLen
	}
	x := AssertStackItem(e.evaluationStack.Pop()).GetBigInteger()
	if ok, err := checkBigInteger(x); !ok {
		return FAULT, err
	}
	err := pushData(e, BigIntComp(x, e.opCode))
	if err != nil {
		return FAULT, err
	}
	return NONE, nil
}

func opBigIntZip(e *ExecutionEngine) (VMState, error) {
	if e.evaluationStack.Count() < 2 {
		return FAULT, errors.ErrUnderStackLen
	}
	x2 := AssertStackItem(e.evaluationStack.Pop()).GetBigInteger()
	if ok, err := checkBigInteger(x2); !ok {
		return FAULT, err
	}
	x1 := AssertStackItem(e.evaluationStack.Pop()).GetBigInteger()
	if ok, err := checkBigInteger(x1); !ok {
		return FAULT, err
	}
	result := BigIntZip(x1, x2, e.opCode)
	if ok, err := checkBigInteger(result); !ok {
		return FAULT, err
	}
	err := pushData(e, result)
	if err != nil {
		return FAULT, err
	}
	return NONE, nil
}

func checkBigInteger(value *big.Int) (bool, error) {
	if len(value.Bytes()) <= MAX_BIGINTEGER {
		return true, nil
	}
	return false, errors.ErrOverBigIntegerSize
}

func opBoolZip(e *ExecutionEngine) (VMState, error) {
	if e.evaluationStack.Count() < 2 {
		return FAULT, nil
	}
	x2 := AssertStackItem(e.evaluationStack.Pop()).GetBoolean()
	x1 := AssertStackItem(e.evaluationStack.Pop()).GetBoolean()
	err := pushData(e, BoolZip(x1, x2, e.opCode))
	if err != nil {
		return FAULT, err
	}
	return NONE, nil
}

func opBigIntComp(e *ExecutionEngine) (VMState, error) {
	if e.evaluationStack.Count() < 2 {
		return FAULT, errors.ErrUnderStackLen
	}
	x2 := AssertStackItem(e.evaluationStack.Pop()).GetBigInteger()
	x1 := AssertStackItem(e.evaluationStack.Pop()).GetBigInteger()
	err := pushData(e, BigIntMultiComp(x1, x2, e.opCode))
	if err != nil {
		return FAULT, err
	}
	return NONE, nil
}

func opWithIn(e *ExecutionEngine) (VMState, error) {
	if e.evaluationStack.Count() < 3 {
		return FAULT, errors.ErrUnderStackLen
	}
	b := AssertStackItem(e.evaluationStack.Pop()).GetBigInteger()
	if ok, err := checkBigInteger(b); !ok {
		return FAULT, err
	}
	a := AssertStackItem(e.evaluationStack.Pop()).GetBigInteger()
	if ok, err := checkBigInteger(a); !ok {
		return FAULT, err
	}
	x := AssertStackItem(e.evaluationStack.Pop()).GetBigInteger()
	if ok, err := checkBigInteger(x); !ok {
		return FAULT, err
	}
	err := pushData(e, WithInOp(x, a, b))
	if err != nil {
		return FAULT, err
	}
	return NONE, nil
}
