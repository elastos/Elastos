package core

import (
	"github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA.SideChain/vm/interfaces"
	"github.com/elastos/Elastos.ELA.Utility/common"
)

func IsRegisterIdentificationTx(tx *types.Transaction) bool {
	return tx.TxType == RegisterIdentification
}

func InitTransactionHelper() {
	types.Name = name
	types.GetDataContainer = getDataContainer
}

func name(txType types.TransactionType) string {
	switch txType {
	case types.CoinBase:
		return "CoinBase"
	case types.RegisterAsset:
		return "RegisterAsset"
	case types.TransferAsset:
		return "TransferAsset"
	case types.Record:
		return "Record"
	case types.Deploy:
		return "Deploy"
	case types.SideChainPow:
		return "SideChainPow"
	case types.RechargeToSideChain:
		return "RechargeToSideChain"
	case types.WithdrawFromSideChain:
		return "WithdrawFromSideChain"
	case types.TransferCrossChainAsset:
		return "TransferCrossChainAsset"
	case RegisterIdentification:
		return "RegisterIdentification"
	default:
		return "Unknown"
	}
}

func getDataContainer(programHash *common.Uint168, tx *types.Transaction) interfaces.IDataContainer {
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
