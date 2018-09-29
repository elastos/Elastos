package types

import (
	"github.com/elastos/Elastos.ELA.SideChain/vm/interfaces"
	"github.com/elastos/Elastos.ELA.Utility/common"
)

var TransactionHelper *TransactionHelperBase

type TransactionHelperBase struct {
	Name             func(txType TransactionType) string
	GetDataContainer func(programHash *common.Uint168, tx *Transaction) interfaces.IDataContainer
}

func InitTransactionHelper() {
	TransactionHelper = &TransactionHelperBase{}
	TransactionHelper.Init()
}

func (t *TransactionHelperBase) Init() {
	t.Name = t.name
	t.GetDataContainer = t.getDataContainer
}

func (t *TransactionHelperBase) name(txType TransactionType) string {
	switch txType {
	case CoinBase:
		return "CoinBase"
	case RegisterAsset:
		return "RegisterAsset"
	case TransferAsset:
		return "TransferAsset"
	case Record:
		return "Record"
	case Deploy:
		return "Deploy"
	case SideChainPow:
		return "SideChainPow"
	case RechargeToSideChain:
		return "RechargeToSideChain"
	case WithdrawFromSideChain:
		return "WithdrawFromSideChain"
	case TransferCrossChainAsset:
		return "TransferCrossChainAsset"
	default:
		return "Unknown"
	}
}

func (t *TransactionHelperBase) getDataContainer(programHash *common.Uint168, tx *Transaction) interfaces.IDataContainer {
	return tx
}
