package types

import (
	"github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA.SideChain/vm/interfaces"
	"github.com/elastos/Elastos.ELA.Utility/common"
)

func IsRegisterIdentificationTx(tx *types.Transaction) bool {
	return tx.TxType == RegisterIdentification
}

func init() {
	types.TxTypeStr = func(txType types.TxType) string {
		if txType == RegisterIdentification {
			return "RegisterIdentification"
		}
		return types.TxTypeStr(txType)
	}

	types.GetDataContainer = func(programHash *common.Uint168, tx *types.Transaction) interfaces.IDataContainer {
		if tx.TxType == RegisterIdentification {
			for _, output := range tx.Outputs {
				if programHash.IsEqual(output.ProgramHash) {
					return tx.Payload.(*PayloadRegisterIdentification)
				}
			}
		}
		return types.GetDataContainer(programHash, tx)
	}

	types.GetPayloadByTxType = func(txType types.TxType) (types.Payload, error) {
		if txType == RegisterIdentification {
			return &PayloadRegisterIdentification{}, nil
		}
		return types.GetPayloadByTxType(txType)
	}
}
