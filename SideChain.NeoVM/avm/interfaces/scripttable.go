package interfaces

import (
	"github.com/elastos/Elastos.ELA.SideChain/types"
)

type IScriptTable interface {
	GetScript(hash []byte) ([]byte)

	GetTxReference(tx *IDataContainer) (map[*types.Input]*types.Output, error)
}
