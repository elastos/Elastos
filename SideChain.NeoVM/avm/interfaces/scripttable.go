package interfaces

type IScriptTable interface {
	GetScript(hash []byte) ([]byte)

	GetTxReference(tx *IDataContainer) (map[IIntPut]IOutput, error)
}
