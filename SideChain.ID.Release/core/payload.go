package core

import (
	"errors"

	"github.com/elastos/Elastos.ELA.SideChain/types"
)

func getPayloadByTxType(txType types.TransactionType) (types.Payload, error) {
	var p types.Payload
	switch txType {
	case types.CoinBase:
		p = new(types.PayloadCoinBase)
	case types.RegisterAsset:
		p = new(types.PayloadRegisterAsset)
	case types.TransferAsset:
		p = new(types.PayloadTransferAsset)
	case types.Record:
		p = new(types.PayloadRecord)
	case types.RechargeToSideChain:
		p = new(types.PayloadRechargeToSideChain)
	case types.TransferCrossChainAsset:
		p = new(types.PayloadTransferCrossChainAsset)
	case RegisterIdentification:
		p = new(PayloadRegisterIdentification)
	default:
		return nil, errors.New("[Transaction], invalid transaction type.")
	}
	return p, nil
}
