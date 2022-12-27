package avm

import (
	"io"
	_ "sort"
	"math"

	"github.com/elastos/Elastos.ELA/common"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/interfaces"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/utils"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/errors"
)

const (
	MAXSTEPS                      = -1
	ratio                         = 100000
	gasFree                       = 10 * 100000000
	StackLimit             uint32 = 2 * 1024
	MaxItemSize            uint32 = 1024 * 1024
	MaxArraySize           uint32 = 1024
	MAXInvocationStackSize        = 1024
	MaxParameterSize              = 252
	MAXContractDescript           = 65536
	MAX_SHL_SHR                   = math.MaxUint16
	Min_SHL_SHR                   = -MAX_SHL_SHR
	MAX_BIGINTEGER                = 32
)

func NewExecutionEngine(container interfaces.IDataContainer, crypto interfaces.ICrypto, maxSteps int,
	table interfaces.IScriptTable, service IGeneralService, gas common.Fixed64, trigger TriggerType,
	testMode bool) *ExecutionEngine {
	var engine ExecutionEngine
	engine.crypto = crypto
	engine.dataContainer = container

	engine.table = table
	engine.invocationStack = utils.NewRandAccessStack()
	engine.evaluationStack = utils.NewRandAccessStack()
	engine.altStack = utils.NewRandAccessStack()

	engine.state = BREAK
	engine.context = nil
	engine.opCode = 0
	engine.opCount = 0
	engine.maxSteps = maxSteps

	engine.service = NewGeneralService()
	if service != nil {
		engine.service.MergeMap(service.GetServiceMap())
	}

	engine.trigger = trigger
	engine.gas = gas.IntValue() + gasFree
	engine.gasConsumed = 0
	engine.testMode = testMode

	return &engine
}

type ExecutionEngine struct {
	crypto  interfaces.ICrypto
	table   interfaces.IScriptTable
	service *GeneralService

	dataContainer   interfaces.IDataContainer
	invocationStack *utils.RandomAccessStack
	opCount         int

	maxSteps int

	evaluationStack *utils.RandomAccessStack
	altStack        *utils.RandomAccessStack
	state           VMState

	context *ExecutionContext

	//current opcode
	opCode      OpCode
	gas         int64
	gasConsumed int64
	trigger     TriggerType
	testMode    bool
}

func (e *ExecutionEngine) IsTestMode() bool {
	return e.testMode
}

func (e *ExecutionEngine) GetGasConsumed() int64 {
	return e.gasConsumed
}

func (e *ExecutionEngine) GetTrigger() TriggerType {
	return e.trigger
}

func (e *ExecutionEngine) GetDataContainer() interfaces.IDataContainer {
	return e.dataContainer
}

func (e *ExecutionEngine) GetState() VMState {
	return e.state
}

func (e *ExecutionEngine) GetEvaluationStack() *utils.RandomAccessStack {
	return e.evaluationStack
}

func (e *ExecutionEngine) GetExecuteResult() bool {
	return AssertStackItem(e.evaluationStack.Pop()).GetBoolean()
}

func (e *ExecutionEngine) ExecutingScript() []byte {
	context := AssertExecutionContext(e.invocationStack.Peek(0))
	if context != nil {
		return context.Script
	}
	return nil
}

func (e *ExecutionEngine) CallingScript() []byte {
	if e.invocationStack.Count() > 1 {
		context := AssertExecutionContext(e.invocationStack.Peek(1))
		if context != nil {
			return context.Script
		}
		return nil
	}
	return nil
}

func (e *ExecutionEngine) CurrentContext() *ExecutionContext {
	context := AssertExecutionContext(e.invocationStack.Peek(0))
	if context != nil {
		return context
	}
	return nil
}

func (e *ExecutionEngine) Create(caller common.Uint168, code []byte) ([]byte, error) {
	return code, nil
}

func (e *ExecutionEngine) Call(caller common.Uint168, codeHash common.Uint168, input []byte) ([]byte, error) {
	e.LoadScript(input, false)
	err := e.Execute()
	if err != nil {
		return nil, err
	}
	return nil, nil
}

func (e *ExecutionEngine) Hash168(script []byte) []byte {
	return e.crypto.Hash168(script)
}

func (e *ExecutionEngine) EntryScript() []byte {
	context := AssertExecutionContext(e.invocationStack.Peek(e.invocationStack.Count() - 1))
	if context != nil {
		return context.Script
	}
	return nil
}

func (e *ExecutionEngine) LoadScript(script []byte, pushOnly bool) *ExecutionContext {
	content := NewExecutionContext(script, pushOnly, nil)
	e.invocationStack.Push(content)
	return content
}

func (e *ExecutionEngine) LoadPriceOnlyScript(script []byte) *ExecutionContext {
	content := NewExecutionContext(script, false, nil)
	content.GetPriceOnly = true
	e.invocationStack.Push(content)
	return content
}

func (e *ExecutionEngine) Execute() error {
	e.state = e.state & (^BREAK)
	for {
		if e.state == FAULT || e.state == HALT || e.state == BREAK {
			break
		}
		err := e.StepInto()
		if err != nil {
			log.Error("ExecutionEngine on avm:", err.Error())
			return err
		}
	}
	return nil
}

func (e *ExecutionEngine) StepInto() error {
	if e.invocationStack.Count() == 0 {
		e.state = VMState(e.state | HALT)
	}
	if e.state&HALT == HALT || e.state&FAULT == FAULT {
		return nil
	}
	context := AssertExecutionContext(e.invocationStack.Peek(0))
	var opCode OpCode
	if context.GetInstructionPointer() >= len(context.Script) {
		opCode = RET
	} else {
		op, err := context.OpReader.ReadByte()
		if err == io.EOF && opCode == 0 {
			e.state = FAULT
			return err
		}
		opCode = OpCode(op)
	}

	e.opCount++
	state, err := e.ExecuteOp(OpCode(opCode), context)
	switch state {
	case VMState(HALT):
		e.state = VMState(e.state | HALT)
		return err
	case VMState(FAULT):
		e.state = VMState(e.state | FAULT)
		return err
	}
	return nil
}

func (e *ExecutionEngine) ExecuteOp(opCode OpCode, context *ExecutionContext) (VMState, error) {
	if opCode > PUSH16 && opCode != RET && context.PushOnly {
		return FAULT, errors.ErrBadValue
	}
	if opCode > PUSH16 && e.opCount > e.maxSteps && e.maxSteps > 0 {
		return FAULT, nil
	}

	if opCode >= PUSHBYTES1 && opCode <= PUSHBYTES75 {

		err := pushData(e, context.OpReader.ReadBytes(int(opCode)))
		if err != nil {
			return FAULT, err
		}
		return NONE, nil
	}
	e.opCode = opCode
	e.context = context

	if !e.checkStackSize() {
		return FAULT, errors.ErrOverLimitStack
	}

	var elaRatio int64 = 10 //ten times smaller than neogas
	price := e.getPrice() * ratio / elaRatio
	if price < 0 {
		return FAULT, errors.ErrNumericOverFlow
	}
	e.gasConsumed += price
	if e.gas < e.gasConsumed && !e.IsTestMode() {
		return FAULT, errors.ErrOutOfGas
	}

	opExec := OpExecList[opCode]
	if opExec.Exec == nil {
		return FAULT, errors.ErrNotSupportOpCode
	}

	if opExec.Validator != nil {
		if err := opExec.Validator(e); err != nil {
			return FAULT, err
		}
	}

	state, err := opExec.Exec(e)
	if err != nil || HALT == state || FAULT == state {
		return state, err
	}
	return NONE, nil
}

func (e *ExecutionEngine) StepOut() {
	e.state = e.state & (^BREAK)
	c := e.invocationStack.Count()
	for {
		if e.state == FAULT || e.state == HALT || e.state == BREAK || e.invocationStack.Count() >= c {
			break
		}
		e.StepInto()
	}
}

func (e *ExecutionEngine) StepOver() {
	if e.state == FAULT || e.state == HALT {
		return
	}
	e.state = e.state & (^BREAK)
	c := e.invocationStack.Count()
	for {
		if e.state == FAULT || e.state == HALT || e.state == BREAK || e.invocationStack.Count() > c {
			break
		}
		e.StepInto()
	}
}

func (e *ExecutionEngine) AddBreakPoint(position uint) {
	//b := e.context.BreakPoints
	//b = append(b, position)
}

func (e *ExecutionEngine) RemoveBreakPoint(position uint) bool {
	//if e.invocationStack.Count() == 0 { return false }
	//b := e.context.BreakPoints
	return true
}

func (e *ExecutionEngine) checkStackSize() bool {
	size := 0
	if e.opCode < PUSH16 {
		size = 1
	} else {
		switch e.opCode {
		case DEPTH, DUP, OVER, TUCK:
			size = 1
		case UNPACK:
			//item := Peek(e)
			item := PeekStackItem(e)
			if item == nil {
				return false
			}
			size = len(item.GetArray())
		}
	}
	size += e.evaluationStack.Count() + e.altStack.Count()
	if uint32(size) > StackLimit {
		return false
	}
	return true
}

func (e *ExecutionEngine) getPrice() int64 {
	if e.opCode <= PUSH16 {
		return 0
	}
	switch e.opCode {
	case NOP:
		return 0
	case APPCALL, TAILCALL:
		return 10
	case SYSCALL:
		return e.getPriceForSysCall()
	case SHA1, SHA256:
		return 10
	case HASH160, HASH256:
		return 20
	case CHECKSIG:
		return 100
	case CHECKMULTISIG:
		if e.evaluationStack.Count() == 0 {
			return 1
		}
		n := PeekBigInteger(e).Int64()
		if n < 1 {
			return 1
		}
		return int64(100 * n)
	default:
		return 1
	}
}

func (e *ExecutionEngine) getPriceForSysCall() int64 {
	context := e.context
	i := context.GetInstructionPointer() - 1
	c := len(context.Script)
	if i >= c-3 {
		return 1
	}
	l := int(context.Script[i+1])
	if i > c-l-2 {
		return 1
	}
	name := string(context.Script[i+2 : i+2+l])
	switch name {
	case "Neo.Runtime.CheckWitness":
		return 200
	case "Neo.Blockchain.GetHeader":
		return 100
	case "Neo.Blockchain.GetBlock":
		return 200
	case "Neo.Blockchain.GetTransaction":
		return 100
	case "Neo.Blockchain.GetTransactionHeight":
		return 100
	case "Neo.Blockchain.GetAccount":
		return 100
	case "Neo.Blockchain.RegisterValidator":
		return 1000 * 100000000 / ratio
	case "Neo.Blockchain.GetValidators":
		return 200
	case "Neo.Blockchain.CreateAsset":
		return 5000 * 100000000 / ratio
	case "Neo.Blockchain.GetAsset":
		return 100
	case "Neo.Contract.Create":
		return 500 * 100000000 / ratio
	case "Neo.Blockchain.GetContract":
		return 100
	case "Neo.Transaction.GetReferences":
		return 200
	case "Neo.Asset.Create":
		return 5000 * 100000000 / ratio
	case "Neo.Asset.Renew":
		return PeekBigInteger(e).Int64() * 5000 * 100000000 / ratio
	case "Neo.Storage.Get":
		return 100
	case "Neo.Storage.Put":
		price := ((len(PeekNByteArray(1, e))+len(PeekNByteArray(2, e))-1)/1024 + 1) * 1000
		return int64(price)
	case "Neo.Storage.Delete":
		return 100
	default:
		return 1
	}
}
