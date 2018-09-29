package core

import (
	ucore "github.com/elastos/Elastos.ELA.SideChain/core"
	"github.com/elastos/Elastos.ELA.SideChain/vm/interfaces"
	"github.com/elastos/Elastos.ELA.Utility/common"
)

func IsRegisterIdentificationTx(tx *ucore.Transaction) bool {
	return tx.TxType == RegisterIdentification
}

func InitTransactionHelper() {
	ucore.TransactionHelper = &ucore.TransactionHelperBase{}
	ucore.TransactionHelper.Init()
	ucore.TransactionHelper.Name = name
	ucore.TransactionHelper.GetDataContainer = getDataContainer
}

func name(txType ucore.TransactionType) string {
	switch txType {
	case ucore.CoinBase:
		return "CoinBase"
	case ucore.RegisterAsset:
		return "RegisterAsset"
	case ucore.TransferAsset:
		return "TransferAsset"
	case ucore.Record:
		return "Record"
	case ucore.Deploy:
		return "Deploy"
	case ucore.SideChainPow:
		return "SideChainPow"
	case ucore.RechargeToSideChain:
		return "RechargeToSideChain"
	case ucore.WithdrawFromSideChain:
		return "WithdrawFromSideChain"
	case ucore.TransferCrossChainAsset:
		return "TransferCrossChainAsset"
	case RegisterIdentification:
		return "RegisterIdentification"
	default:
		return "Unknown"
	}
}

func getDataContainer(programHash *common.Uint168, tx *ucore.Transaction) interfaces.IDataContainer {
	switch tx.TxType {
	case RegisterIdentification:
		for _, output := range tx.Outputs {
			if programHash.IsEqual(output.ProgramHash) {
				return tx.Payload.(*PayloadRegisterIdentification)
			}
		}
	}
	return tx
}
