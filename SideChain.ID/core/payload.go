package core

import (
	"errors"

	ucore "github.com/elastos/Elastos.ELA.SideChain/core"
)

func InitPayloadHelper() {
	ucore.PayloadHelper = &ucore.PayloadBase{}
	ucore.PayloadHelper.GetPayload = getPayload
}

func getPayload(txType ucore.TransactionType) (ucore.Payload, error) {
	var p ucore.Payload
	switch txType {
	case ucore.CoinBase:
		p = new(ucore.PayloadCoinBase)
	case ucore.RegisterAsset:
		p = new(ucore.PayloadRegisterAsset)
	case ucore.TransferAsset:
		p = new(ucore.PayloadTransferAsset)
	case ucore.Record:
		p = new(ucore.PayloadRecord)
	case ucore.RechargeToSideChain:
		p = new(ucore.PayloadRechargeToSideChain)
	case ucore.TransferCrossChainAsset:
		p = new(ucore.PayloadTransferCrossChainAsset)
	case RegisterIdentification:
		p = new(PayloadRegisterIdentification)
	default:
		return nil, errors.New("[Transaction], invalid transaction type.")
	}
	return p, nil
}
