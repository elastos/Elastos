package mempool

type FuncName string

var (
	FuncNames = funcNames{
		CheckTransactionSize:                    "checktransactionsize",
		CheckTransactionInput:                   "checktransactioninput",
		CheckTransactionOutput:                  "checktransactionoutput",
		CheckAssetPrecision:                     "checkassetprecision",
		CheckAttributeProgram:                   "checkattributeprogram",
		CheckTransactionPayload:                 "checktransactionpayload",
		CheckTransactionDuplicate:               "checktransactionduplicate",
		CheckTransactionCoinBase:                "checktransactioncoinbase",
		CheckTransactionDoubleSpend:             "checktransactiondoublespend",
		CheckTransactionSignature:               "checktransactionsignature",
		CheckRechargeToSideChainTransaction:     "checkrechargetosidechaintransaction",
		CheckTransferCrossChainAssetTransaction: "checktransfercrosschainassettransaction",
		CheckTransactionUTXOLock:                "checktransactionutxolock",
		CheckTransactionBalance:                 "checktransactionbalance",
		CheckReferencedOutput:                   "checkreferencedoutput",
	}
)

type funcNames struct {
	CheckTransactionSize                    FuncName
	CheckTransactionInput                   FuncName
	CheckTransactionOutput                  FuncName
	CheckAssetPrecision                     FuncName
	CheckAttributeProgram                   FuncName
	CheckTransactionPayload                 FuncName
	CheckTransactionDuplicate               FuncName
	CheckTransactionCoinBase                FuncName
	CheckTransactionDoubleSpend             FuncName
	CheckTransactionSignature               FuncName
	CheckRechargeToSideChainTransaction     FuncName
	CheckTransferCrossChainAssetTransaction FuncName
	CheckTransactionUTXOLock                FuncName
	CheckTransactionBalance                 FuncName
	CheckReferencedOutput                   FuncName
}
