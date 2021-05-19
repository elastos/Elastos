package vm

type GeneralService struct {
	dictionary map[string]func(*ExecutionEngine) bool
}

func NewGeneralService() *GeneralService {
	var is GeneralService
	is.dictionary = make(map[string]func(*ExecutionEngine) bool, 0)
	is.Register("System.ScriptEngine.GetScriptContainer", is.GetScriptContainer)
	is.Register("System.ScriptEngine.GetExecutingScriptHash", is.GetExecutingScriptHash)
	is.Register("System.ScriptEngine.GetCallingScriptHash", is.GetCallingScriptHash)
	is.Register("System.ScriptEngine.GetEntryScriptHash", is.GetEntryScriptHash)
	return &is
}

func (is *GeneralService) Register(method string, handler func(*ExecutionEngine) bool) bool {
	if _, ok := is.dictionary[method]; ok {
		return false
	}
	is.dictionary[method] = handler
	return true
}

func (is *GeneralService) Invoke(method string, engine *ExecutionEngine) bool {
	if v, ok := is.dictionary[method]; ok {
		return v(engine)
	}
	return false
}

func (is *GeneralService) GetScriptContainer(engine *ExecutionEngine) bool {
	engine.evaluationStack.Push(engine.dataContainer)
	return true
}

func (is *GeneralService) GetExecutingScriptHash(engine *ExecutionEngine) bool {
	engine.evaluationStack.Push(engine.crypto.Hash168(engine.ExecutingScript()))
	return true
}

func (is *GeneralService) GetCallingScriptHash(engine *ExecutionEngine) bool {
	engine.evaluationStack.Push(engine.crypto.Hash168(engine.CallingScript()))
	return true
}

func (is *GeneralService) GetEntryScriptHash(engine *ExecutionEngine) bool {
	engine.evaluationStack.Push(engine.crypto.Hash168(engine.EntryScript()))
	return true
}
