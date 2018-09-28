package blockchain

type ValidateFuncName string
type StoreFuncName string

var (
	ValidateFuncNames = blockValidateFuncs{
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

	StoreFuncNames = storeFuncs{
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
)

type blockValidateFuncs struct {
	PowCheckBlockSanity        ValidateFuncName
	PowCheckHeader             ValidateFuncName
	PowCheckTransactionsCount  ValidateFuncName
	PowCheckBlockSize          ValidateFuncName
	PowCheckTransactionsFee    ValidateFuncName
	PowCheckTransactionsMerkle ValidateFuncName
	PowCheckBlockContext       ValidateFuncName
	CheckProofOfWork           ValidateFuncName
	CheckFinalizedTransaction  ValidateFuncName
}

type storeFuncs struct {
	PersistTrimmedBlock  StoreFuncName
	PersistBlockHash     StoreFuncName
	PersistCurrentBlock  StoreFuncName
	PersistUnspendUTXOs  StoreFuncName
	PersistTransactions  StoreFuncName
	PersistUnspend       StoreFuncName
	RollbackTrimmedBlock StoreFuncName
	RollbackBlockHash    StoreFuncName
	RollbackCurrentBlock StoreFuncName
	RollbackUnspendUTXOs StoreFuncName
	RollbackTransactions StoreFuncName
	RollbackUnspend      StoreFuncName
}
