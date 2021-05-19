package avm

import (
	"github.com/elastos/Elastos.ELA/common"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/errors"
)

func opNop(e *ExecutionEngine) (VMState, error) {
	//time.Sleep(1 * time.Millisecond)
	return NONE, nil
}

func opJmp(e *ExecutionEngine) (VMState, error) {
	offset := int(e.context.OpReader.ReadInt16())
	offset = e.context.GetInstructionPointer() + offset - 3

	if offset < 0 || offset > len(e.context.Script) {
		return FAULT, errors.ErrFault
	}
	fValue := true
	if e.opCode > JMP {
		s := AssertStackItem(e.evaluationStack.Pop())
		fValue = s.GetBoolean()
		if e.opCode == JMPIFNOT {
			fValue = !fValue
		}
	}
	if fValue {
		e.context.SetInstructionPointer(offset)
	}

	return NONE, nil
}

func opCall(e *ExecutionEngine) (VMState, error) {
	e.invocationStack.Push(e.context.Clone())
	e.context.SetInstructionPointer(e.context.GetInstructionPointer() + 2)
	e.opCode = JMP
	e.context = e.invocationStack.Peek(0).(*ExecutionContext)
	opJmp(e)
	return NONE, nil
}

func opRet(e *ExecutionEngine) (VMState, error) {
	e.invocationStack.Pop()
	return NONE, nil
}

func opAppCall(e *ExecutionEngine) (VMState, error) {
	if e.table == nil {
		return FAULT, errors.ErrTableIsNil
	}
	script_hash := e.context.OpReader.ReadBytes(20)
	script_hash = common.BytesReverse(script_hash)
	script := e.table.GetScript(script_hash)
	if script == nil {
		return FAULT, errors.ErrNotFindScript
	}
	if e.opCode == TAILCALL {
		e.invocationStack.Pop()
	}
	e.LoadScript(script, false)
	return NONE, nil
}

func opSysCall(e *ExecutionEngine) (VMState, error) {
	if e.service == nil {
		return FAULT, errors.ErrServiceIsNil
	}
	success, err := e.service.Invoke(e.context.OpReader.ReadVarString(), e)
	if success && err == nil {
		return NONE, nil
	} else if err ==  errors.ErrNotSupportSysCall {
		return FAULT, err
	} else if err != nil {
		return FAULT, err
	}
	return FAULT, nil
}

func opCallI(e *ExecutionEngine) (VMState, error) {
	_, err := e.context.OpReader.ReadByte()
	if err != nil {
		return FAULT, err
	}
	pcount, err := e.context.OpReader.ReadByte()
	if err != nil {
		return FAULT, err
	}
	if e.evaluationStack.Count() < int(pcount) {
		return FAULT, nil
	}
	e.invocationStack.Push(e.context.Clone())
	e.evaluationStack.CopyTo(e.evaluationStack, int(pcount))
	e.context.SetInstructionPointer(e.context.GetInstructionPointer() + 2)
	e.context = AssertExecutionContext(e.invocationStack.Peek(0))
	e.opCode = JMP
	opJmp(e)
	return NONE, nil
}

func opCallE(e *ExecutionEngine) (VMState, error) {
	if e.table == nil {
		return FAULT, nil
	}
	_, err := e.context.OpReader.ReadByte()
	if err != nil {
		return FAULT, err
	}
	pcount, err := e.context.OpReader.ReadByte()
	if err != nil {
		return FAULT, err
	}
	if (e.evaluationStack.Count() < int(pcount)) {
		return FAULT, err
	}
	var script_hash []byte
	if (e.opCode == CALL_ED || e.opCode == CALL_EDT) {
		script_hash = PopByteArray(e)
		if len(script_hash) == 21 {
			hash := make([]byte, 20)
			copy(hash, script_hash[1 :])
			script_hash = hash
		}
	} else {
		script_hash = e.context.OpReader.ReadBytes(20)
		script_hash = common.BytesReverse(script_hash)
	}

	script := e.table.GetScript(script_hash)
	if script == nil {
		return FAULT, err
	}
	if (e.opCode == CALL_ET || e.opCode == CALL_EDT) {
		e.invocationStack.Pop()
	}

	e.LoadScript(script, false)

	return NONE, nil
}

func OpThrow(e *ExecutionEngine) (VMState, error) {
	e.state |= FAULT
	log.Error("contract throw an exception!")
	return NONE, nil
}

func OpThowIfNot(e *ExecutionEngine) (VMState, error) {
	data := PopBoolean(e)
	if data == false {
		log.Error("contract throw an exception!")
		e.state |= FAULT
	}
	return NONE, nil
}
