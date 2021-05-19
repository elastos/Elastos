package types

import (
	"github.com/elastos/Elastos.ELA.SideChain/types"
)

func init() {
	getPayloadByTxType := types.GetPayloadByTxType
	types.GetPayloadByTxType = func(txType types.TxType) (types.Payload, error) {
		if txType == types.Deploy {
			return &PayloadDeploy{}, nil
		} else if txType == types.Invoke {
			return &PayloadInvoke{}, nil
		}
		return getPayloadByTxType(txType)
	}
}
