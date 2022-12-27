package blockchain

type ValidateFuncName string
type StoreFuncName string

var (
	ValidateFuncNames = blockValidateFuncs{
		PowCheckBlockSanity:       "checkblocksanity",
		CheckHeader:               "checkheader",
		CheckTransactionsCount:    "checktransactionscount",
		CheckBlockSize:            "checkblocksize",
		CheckCoinBaseTransaction:  "checkcoinbasetransaction",
		CheckTransactionsMerkle:   "checktransactionsmerkle",
		PowCheckBlockContext:      "checkblockcontext",
		CheckProofOfWork:          "checkproofofwork",
		CheckFinalizedTransaction: "checkfinalizedtransaction",
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
	PowCheckBlockSanity       ValidateFuncName
	CheckHeader               ValidateFuncName
	CheckTransactionsCount    ValidateFuncName
	CheckBlockSize            ValidateFuncName
	CheckCoinBaseTransaction  ValidateFuncName
	CheckTransactionsMerkle   ValidateFuncName
	PowCheckBlockContext      ValidateFuncName
	CheckProofOfWork          ValidateFuncName
	CheckFinalizedTransaction ValidateFuncName
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
