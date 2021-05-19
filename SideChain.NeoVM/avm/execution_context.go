package avm

import (
	"io"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/utils"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/common"
)

type ExecutionContext struct {
	Script             []byte
	OpReader           *utils.VmReader
	PushOnly           bool
	BreakPoints        []uint
	InstructionPointer int
	CodeHash           []byte
	GetPriceOnly       bool
}

func NewExecutionContext(script []byte, pushOnly bool, breakPoints []uint) *ExecutionContext {
	var executionContext ExecutionContext
	executionContext.Script = script
	executionContext.OpReader = utils.NewVmReader(script)
	executionContext.PushOnly = pushOnly
	executionContext.BreakPoints = breakPoints
	executionContext.InstructionPointer = executionContext.OpReader.Position()
	executionContext.GetPriceOnly = false
	return &executionContext
}

func (ec *ExecutionContext) GetInstructionPointer() int {
	return ec.OpReader.Position()
}

func (ec *ExecutionContext) SetInstructionPointer(offset int) {
	ec.InstructionPointer = offset
	ec.OpReader.Seek(int64(offset), io.SeekStart)
}

func (ec *ExecutionContext) GetCodeHash() []byte {
	if ec.CodeHash == nil {
		hash, err := common.ToProgramHash(ec.Script)
		if err != nil {
			return nil
		}
		ec.CodeHash = hash.Bytes()
	}
	return ec.CodeHash
}

func (ec *ExecutionContext) NextInstruction() OpCode {
	return OpCode(ec.Script[ec.OpReader.Position()])
}

func (ec *ExecutionContext) Clone() *ExecutionContext {
	executionContext := NewExecutionContext(ec.Script, ec.PushOnly, ec.BreakPoints)
	executionContext.InstructionPointer = ec.InstructionPointer
	executionContext.SetInstructionPointer(ec.GetInstructionPointer())
	return executionContext
}
