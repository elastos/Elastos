package smartcontract

import (
	"math/big"
	"bytes"
	"strconv"

	"github.com/elastos/Elastos.ELA/common"

	"github.com/elastos/Elastos.ELA.SideChain/database"
	st "github.com/elastos/Elastos.ELA.SideChain/types"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/interfaces"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/datatype"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/service"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/types"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm"
	ns "github.com/elastos/Elastos.ELA.SideChain.NeoVM/smartcontract/service"
)

type SmartContract struct {
	Engine         Engine
	Code           []byte
	Input          []byte
	ParameterTypes []types.ContractParameterType
	Caller         common.Uint168
	CodeHash       common.Uint168
	ReturnType     types.ContractParameterType
}

type Context struct {
	Caller         common.Uint168
	Code           []byte
	Input          []byte
	CodeHash       common.Uint168
	CacheCodeTable interfaces.IScriptTable
	Time           *big.Int
	BlockNumber    *big.Int
	SignableData   SignableData
	StateMachine   ns.StateMachine
	DBCache        database.Batch
	Gas            common.Fixed64
	ReturnType     types.ContractParameterType
	ParameterTypes []types.ContractParameterType
	Trigger        avm.TriggerType
}

type Engine interface {
	Create(caller common.Uint168, code []byte) ([]byte, error)
	Call(caller common.Uint168, codeHash common.Uint168, input []byte) ([]byte, error)
}

func NewSmartContract(context *Context) (*SmartContract, error) {
	e := avm.NewExecutionEngine(context.SignableData,
		new(avm.CryptoECDsa),
		avm.MAXSTEPS,
		context.CacheCodeTable,
		context.StateMachine,
		context.Gas,
		context.Trigger,
		false,
	)

	return &SmartContract{
		Engine:         e,
		Code:           context.Code,
		CodeHash:       context.CodeHash,
		Input:          context.Input,
		Caller:         context.Caller,
		ReturnType:     context.ReturnType,
		ParameterTypes: context.ParameterTypes,
	}, nil
}

func (sc *SmartContract) DeployContract(payload *types.PayloadDeploy) ([]byte, error) {
	buffer := new(bytes.Buffer)
	paramBuilder := avm.NewParamsBuider(buffer)
	var parameterTypes []byte
	parameterTypes = types.ContractParameterTypeToByte(payload.Code.ParameterTypes)
	returnType := byte(payload.Code.ReturnType)
	paramBuilder.EmitSysCall("Neo.Contract.Create", payload.Code.Code, parameterTypes, returnType, payload.Name,
		payload.CodeVersion, payload.Author, payload.Email, payload.Description)
	_ , err := sc.Engine.Call(sc.Caller, sc.CodeHash, paramBuilder.Bytes())
	if err != nil {
		return nil, err
	}
	return sc.Code, nil
}

func (sc *SmartContract) InvokeContract() (interface{}, error) {
	_, err := sc.Engine.Call(sc.Caller, sc.CodeHash, sc.Input)
	if err != nil {
		return nil, err
	}
	return sc.InvokeResult()
}

func (sc *SmartContract) InvokeResult() (interface{}, error) {
	engine := sc.Engine.(*avm.ExecutionEngine)
	if engine.GetEvaluationStack().Count() > 0 && avm.Peek(engine) != nil {
		switch sc.ReturnType {
		case types.Boolean:
			return avm.PopBoolean(engine), nil
		case types.Integer:
			return avm.PopBigInt(engine), nil
		case types.ByteArray:
			bs := avm.PopByteArray(engine)
			return bs, nil
		case types.String:
			return string(avm.PopByteArray(engine)), nil
		case types.Hash160, types.Hash256:
			data := avm.PopByteArray(engine)
			return common.BytesToHexString(common.BytesReverse(data)), nil
		case types.PublicKey:
			return common.BytesToHexString(avm.PopByteArray(engine)), nil
		case types.Object:
			data := avm.PeekStackItem(engine)
			switch data.(type) {
			case *datatype.Boolean:
				return data.GetBoolean(), nil
			case *datatype.Integer:
				return data.GetBigInteger(), nil
			case *datatype.ByteArray:
				return common.BytesToHexString(data.GetByteArray()), nil
			case *datatype.GeneralInterface:
				interop := data.GetInterface()
				switch interop.(type) {
				case *types.Header:
					return service.GetHeaderInfo(interop.(*types.Header)), nil
				case *st.Block:
					return service.GetBlockInfo(interop.(*st.Block)), nil
				case *st.Transaction:
					return service.GetTXInfo(interop.(*st.Transaction)), nil
				case *st.Asset:
					return service.GetAssetInfo(interop.(*st.Asset)), nil
				}
			}

		}
	}
	return nil, nil
}

func (sc *SmartContract) InvokeParamsTransform() ([]byte, error) {
	builder := avm.NewParamsBuider(new(bytes.Buffer))
	b := bytes.NewBuffer(sc.Input)
	for _, k := range sc.ParameterTypes {
		switch k {
		case types.Boolean:
			p, err := common.ReadUint8(b)
			if err != nil {
				return nil, err
			}
			if p >= 1 {
				builder.EmitPushBool(true)
			} else {
				builder.EmitPushBool(false)
			}
		case types.Integer:
			p, err := common.ReadVarBytes(b, avm.MAX_BIGINTEGER, "SmartContract InvokeParamsTransform Integer")
			if err != nil {
				return nil, err
			}
			i, err := strconv.ParseInt(string(p), 10, 64)
			if err != nil {
				return nil, err
			}
			builder.EmitPushInteger(int64(i))
		case types.Hash160:
			p, err := common.ReadVarBytes(b, 20, "SmartContract InvokeParamsTransform Hash160")
			if err != nil {
				return nil, err
			}
			builder.EmitPushByteArray(common.BytesReverse(p))
		case types.Hash256:
			p, err := common.ReadVarBytes(b, 32, "SmartContract InvokeParamsTransform Hash256")
			if err != nil {
				return nil, err
			}
			builder.EmitPushByteArray(common.BytesReverse(p))
		case types.Hash168:
			p, err := common.ReadVarBytes(b, 21, "SmartContract InvokeParamsTransform Hash168")
			if err != nil {
				return nil, err
			}
			builder.EmitPushByteArray(common.BytesReverse(p))
		case types.ByteArray, types.String:
			p, err := common.ReadVarBytes(b, common.MaxVarStringLength, "SmartContract InvokeParamsTransform ByteArray")
			if err != nil {
				return nil, err
			}
			builder.EmitPushByteArray(p)

		}
		builder.EmitPushCall(sc.CodeHash.Bytes())
		return builder.Bytes(), nil
	}
	return nil, nil
}
