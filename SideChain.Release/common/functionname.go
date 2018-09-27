package common

type BlockValidateFunctionName string
type ChainStoreFunctionName string
type TxValidateFunctionName string

var (
	BlockValidateFunctionNames = blockValidateFunctions{
		PowCheckBlockSanity:        "powcheckblocksanity",
		PowCheckHeader:             "powcheckheader",
		PowCheckTransactionsCount:  "powchecktransactionscount",
		PowCheckBlockSize:          "powcheckblocksize",
		PowCheckTransactionsFee:    "powchecktransactionsfee",
		PowCheckTransactionsMerkle: "powchecktransactionsmerkle",
		PowCheckBlockContext:       "powcheckblockcontext",
		CheckProofOfWork:           "checkproofofwork",
		CheckFinalizedTransaction:  "checkfinalizedtransaction",
	}

	ChainStoreFunctionNames = chainStoreFunctions{
		PersistTrimmedBlock:  "persisttrimmedblock",
		PersistBlockHash:     "persistblockhash",
		PersistCurrentBlock:  "persistcurrentblock",
		PersistUnspendUTXOs:  "persistunspendutxos",
		PersistTransactions:  "persisttransactions",
		PersistUnspend:       "persistunspend",
		RollbackTrimmedBlock: "rollbacktrimmedblock",
		RollbackBlockHash:    "rollbackblockhash",
		RollbackCurrentBlock: "rollbackcurrentblock",
		RollbackUnspendUTXOs: "rollbackunspendutxos",
		RollbackTransactions: "rollbacktransactions",
		RollbackUnspend:      "rollbackunspend",
	}

	TxValidateFunctionNames = txValidateFunctions{
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

type blockValidateFunctions struct {
	PowCheckBlockSanity        BlockValidateFunctionName
	PowCheckHeader             BlockValidateFunctionName
	PowCheckTransactionsCount  BlockValidateFunctionName
	PowCheckBlockSize          BlockValidateFunctionName
	PowCheckTransactionsFee    BlockValidateFunctionName
	PowCheckTransactionsMerkle BlockValidateFunctionName
	PowCheckBlockContext       BlockValidateFunctionName
	CheckProofOfWork           BlockValidateFunctionName
	CheckFinalizedTransaction  BlockValidateFunctionName
}

type chainStoreFunctions struct {
	PersistTrimmedBlock  ChainStoreFunctionName
	PersistBlockHash     ChainStoreFunctionName
	PersistCurrentBlock  ChainStoreFunctionName
	PersistUnspendUTXOs  ChainStoreFunctionName
	PersistTransactions  ChainStoreFunctionName
	PersistUnspend       ChainStoreFunctionName
	RollbackTrimmedBlock ChainStoreFunctionName
	RollbackBlockHash    ChainStoreFunctionName
	RollbackCurrentBlock ChainStoreFunctionName
	RollbackUnspendUTXOs ChainStoreFunctionName
	RollbackTransactions ChainStoreFunctionName
	RollbackUnspend      ChainStoreFunctionName
}

type txValidateFunctions struct {
	CheckTransactionSize                    TxValidateFunctionName
	CheckTransactionInput                   TxValidateFunctionName
	CheckTransactionOutput                  TxValidateFunctionName
	CheckAssetPrecision                     TxValidateFunctionName
	CheckAttributeProgram                   TxValidateFunctionName
	CheckTransactionPayload                 TxValidateFunctionName
	CheckTransactionDuplicate               TxValidateFunctionName
	CheckTransactionCoinBase                TxValidateFunctionName
	CheckTransactionDoubleSpend             TxValidateFunctionName
	CheckTransactionSignature               TxValidateFunctionName
	CheckRechargeToSideChainTransaction     TxValidateFunctionName
	CheckTransferCrossChainAssetTransaction TxValidateFunctionName
	CheckTransactionUTXOLock                TxValidateFunctionName
	CheckTransactionBalance                 TxValidateFunctionName
	CheckReferencedOutput                   TxValidateFunctionName
}
