package avm

import (
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/errors"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/common"
)

type IGeneralService interface {
	Register(method string, handler func(*ExecutionEngine) bool) bool
	GetServiceMap() map[string]func(*ExecutionEngine) bool
}

type GeneralService struct {
	dictionary map[string]func(*ExecutionEngine) bool
	methods    map[uint32]func(*ExecutionEngine) bool
}

func NewGeneralService() *GeneralService {
	var is GeneralService
	is.dictionary = make(map[string]func(*ExecutionEngine) bool, 0)
	is.methods = make(map[uint32]func(*ExecutionEngine) bool, 0)

	is.Register("System.ExecutionEngine.GetScriptContainer", is.GetScriptContainer)
	is.Register("System.ExecutionEngine.GetExecutingScriptHash", is.GetExecutingScriptHash)
	is.Register("System.ExecutionEngine.GetCallingScriptHash", is.GetCallingScriptHash)
	is.Register("System.ExecutionEngine.GetEntryScriptHash", is.GetEntryScriptHash)
	return &is
}

func (is *GeneralService) Register(method string, handler func(*ExecutionEngine) bool) bool {

	if _, ok := is.dictionary[method]; ok {
		return false
	}
	is.dictionary[method] = handler

	hash := common.StringToInvokeHash([]byte(method))
	is.methods[hash] = handler
	return true
}

func (is *GeneralService) MergeMap(dictionary map[string]func(engine *ExecutionEngine) bool) {
	for k, v := range dictionary {
		if _, ok := is.dictionary[k]; !ok {
			is.dictionary[k] = v

			hash := common.StringToInvokeHash([]byte(k))
			is.methods[hash] = v
		}
	}
}

func (i *GeneralService) GetServiceMap() map[string]func(*ExecutionEngine) bool {
	return i.dictionary
}

func (is *GeneralService) Invoke(method string, engine *ExecutionEngine) (bool, error) {
	if v, ok := is.dictionary[method]; ok {
		if engine.context.GetPriceOnly {
			return true, nil
		}
		return v(engine), nil
	}

	methodBytes := []byte(method)
	var hash uint32
	if len(methodBytes) == 4 {
		hash = common.BytesToUInt(methodBytes)
	} else {
		hash = common.StringToInvokeHash([]byte(method))
	}
	if v, ok := is.methods[hash]; ok {
		if engine.context.GetPriceOnly {
			return true, nil
		}
		return v(engine), nil
	}

	return false, errors.ErrNotSupportSysCall
}

func (is *GeneralService) GetScriptContainer(engine *ExecutionEngine) bool {
	PushData(engine, engine.dataContainer)
	return true
}

func (is *GeneralService) GetExecutingScriptHash(engine *ExecutionEngine) bool {
	PushData(engine, engine.crypto.Hash168(engine.ExecutingScript()))
	return true
}

func (is *GeneralService) GetCallingScriptHash(engine *ExecutionEngine) bool {
	pushData(engine, engine.crypto.Hash168(engine.CallingScript()))
	return true
}

func (is *GeneralService) GetEntryScriptHash(engine *ExecutionEngine) bool {
	pushData(engine, engine.crypto.Hash168(engine.EntryScript()))
	return true
}
