package interfaces

type IScriptTable interface {
	GetScript(hash []byte) ([]byte)
}
