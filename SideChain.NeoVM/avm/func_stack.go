package avm

import (
	"math/big"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/errors"
)

func opDupFromAltStack(e *ExecutionEngine) (VMState, error) {
	e.evaluationStack.Push(e.altStack.Peek(0))
	return NONE, nil
}

func opToAltStack(e *ExecutionEngine) (VMState, error) {
	if e.evaluationStack.Count() < 1 {
		return FAULT, errors.ErrUnderStackLen
	}
	e.altStack.Push(e.evaluationStack.Pop())
	return NONE, nil
}

func opFromAltStack(e *ExecutionEngine) (VMState, error) {
	e.evaluationStack.Push(e.altStack.Pop())
	return NONE, nil
}

func opXDrop(e *ExecutionEngine) (VMState, error) {
	if e.evaluationStack.Count() < 1 {
		return FAULT, errors.ErrUnderStackLen
	}
	n := int(AssertStackItem(e.evaluationStack.Pop()).GetBigInteger().Int64())
	if n < 0 {
		return FAULT, errors.ErrBadValue
	}
	e.evaluationStack.Remove(n)
	return NONE, nil
}

func opXSwap(e *ExecutionEngine) (VMState, error) {
	if e.evaluationStack.Count() < 1 {
		return FAULT, errors.ErrUnderStackLen
	}
	n := int(AssertStackItem(e.evaluationStack.Pop()).GetBigInteger().Int64())
	if n <= 0 || n > e.evaluationStack.Count()-1 {
		return FAULT, errors.ErrBadValue
	}
	e.evaluationStack.Swap(0, n)
	return NONE, nil
}

func opXTuck(e *ExecutionEngine) (VMState, error) {
	if e.evaluationStack.Count() < 1 {
		return FAULT, errors.ErrUnderStackLen
	}
	n := int(AssertStackItem(e.evaluationStack.Pop()).GetBigInteger().Int64())
	if n < 0 || n > e.evaluationStack.Count()-1 {
		return FAULT, nil
	}
	e.evaluationStack.Insert(n, e.evaluationStack.Peek(0))
	return NONE, nil
}

func opDepth(e *ExecutionEngine) (VMState, error) {
	pushData(e, e.evaluationStack.Count())
	return NONE, nil
}

func opDrop(e *ExecutionEngine) (VMState, error) {
	if e.evaluationStack.Count() < 1 {
		return FAULT, errors.ErrUnderStackLen
	}
	e.evaluationStack.Pop()
	return NONE, nil
}

func opDup(e *ExecutionEngine) (VMState, error) {
	if e.evaluationStack.Count() < 1 {
		return FAULT, errors.ErrUnderStackLen
	}
	e.evaluationStack.Push(e.evaluationStack.Peek(0))
	return NONE, nil
}

func opNip(e *ExecutionEngine) (VMState, error) {
	if e.evaluationStack.Count() < 2 {
		return FAULT, errors.ErrUnderStackLen
	}
	x2 := e.evaluationStack.Pop()
	e.evaluationStack.Pop()
	e.evaluationStack.Push(x2)
	return NONE, nil
}

func opOver(e *ExecutionEngine) (VMState, error) {
	if e.evaluationStack.Count() < 2 {
		return FAULT, errors.ErrUnderStackLen
	}
	x2 := e.evaluationStack.Pop()
	x1 := e.evaluationStack.Peek(0)
	e.evaluationStack.Push(x2)
	e.evaluationStack.Push(x1)
	return NONE, nil
}

func opPick(e *ExecutionEngine) (VMState, error) {
	if e.evaluationStack.Count() < 2 {
		return FAULT, errors.ErrUnderStackLen
	}
	n := int(AssertStackItem(e.evaluationStack.Pop()).GetBigInteger().Int64())
	if n < 0 {
		return FAULT, errors.ErrBadValue
	}
	if e.evaluationStack.Count() < n+1 {
		return FAULT, errors.ErrBadValue
	}
	e.evaluationStack.Push(e.evaluationStack.Peek(n))
	return NONE, nil
}

func opRoll(e *ExecutionEngine) (VMState, error) {
	n := PopInt(e)
	if n == 0 { return NONE,  errors.ErrUnderStackLen }
	inter := e.evaluationStack.Remove(n)
	if inter == nil {
		inter, _ = NewStackItem(big.NewInt(0))
	}
	Push(e, inter)
	return NONE, nil
}

func opRot(e *ExecutionEngine) (VMState, error) {
	if e.evaluationStack.Count() < 3 {
		return FAULT, errors.ErrUnderStackLen
	}
	x3 := e.evaluationStack.Pop()
	x2 := e.evaluationStack.Pop()
	x1 := e.evaluationStack.Pop()
	e.evaluationStack.Push(x2)
	e.evaluationStack.Push(x3)
	e.evaluationStack.Push(x1)
	return NONE, nil
}

func opSwap(e *ExecutionEngine) (VMState, error) {
	if e.evaluationStack.Count() < 2 {
		return FAULT, errors.ErrUnderStackLen
	}
	x2 := e.evaluationStack.Pop()
	x1 := e.evaluationStack.Pop()
	e.evaluationStack.Push(x2)
	e.evaluationStack.Push(x1)
	return NONE, nil
}

func opTuck(e *ExecutionEngine) (VMState, error) {
	if e.evaluationStack.Count() < 2 {
		return FAULT, errors.ErrUnderStackLen
	}
	x2 := e.evaluationStack.Pop()
	x1 := e.evaluationStack.Pop()
	e.evaluationStack.Push(x2)
	e.evaluationStack.Push(x1)
	e.evaluationStack.Push(x2)
	return NONE, nil
}

func pushData(e *ExecutionEngine, data interface{}) error {
	d, err := NewStackItem(data)
	if err == nil {
		e.evaluationStack.Push(d)
		return nil
	}
	return err
}

