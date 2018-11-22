package avm

import (
	"fmt"

	"github.com/elastos/Elastos.ELA.Utility/common"

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
		return FAULT, nil
	}
	script_hash := e.context.OpReader.ReadBytes(20)
	script := e.table.GetScript(script_hash)
	if script == nil {
		return FAULT, nil
	}
	if e.opCode == TAILCALL {
		e.invocationStack.Pop()
	}
	e.LoadScript(script, false)
	return NONE, nil
}

func opSysCall(e *ExecutionEngine) (VMState, error) {
	if e.service == nil {
		return FAULT, nil
	}
	success := e.service.Invoke(e.context.OpReader.ReadVarString(), e)
	if success {
		return NONE, nil
	} else {
		return FAULT, nil
	}
}

func opCallI(e *ExecutionEngine) (VMState, error) {
	_, err := e.context.OpReader.ReadByte()
	if err != nil {
		return FAULT, err
	}
	pcount, err := e.context.OpReader.ReadByte();
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
	rvcount, err := e.context.OpReader.ReadByte();
	if err != nil {
		return FAULT ,err
	}
	pcount, err := e.context.OpReader.ReadByte();
	if err != nil {
		return FAULT, err
	}
	if (e.evaluationStack.Count() < int(pcount)) {
		return FAULT ,err
	}
	var script_hash []byte
	if (e.opCode == CALL_ED || e.opCode == CALL_EDT) {
		script_hash = PopByteArray(e)
	} else {
		script_hash = e.context.OpReader.ReadBytes(20)
		script_hash = common.BytesReverse(script_hash)
	}

	script := e.table.GetScript(script_hash)
	if script == nil {
		return FAULT ,err
	}
	if (e.opCode == CALL_ET || e.opCode == CALL_EDT) {
		e.invocationStack.Pop()
	}

	e.LoadScript(script, false)
	fmt.Println("opCallE rvcount:", rvcount)

	return NONE, nil
}