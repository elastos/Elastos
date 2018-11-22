package types

import (
	"github.com/elastos/Elastos.ELA.SideChain/types"
)

func init() {

	txTypeStr := types.TxTypeStr
	types.TxTypeStr = func(txType types.TxType) string {
		if txType == Invoke {
			return "Invoke"
		}
		return txTypeStr(txType)
	}

	getPayloadByTxType := types.GetPayloadByTxType
	types.GetPayloadByTxType = func(txType types.TxType) (types.Payload, error) {
		if txType == types.Deploy {
			return &PayloadDeploy{}, nil
		} else if txType == Invoke {
			return &PayloadInvoke{}, nil
		}
		return getPayloadByTxType(txType)
	}
}
