package errors

type ErrCode int

const (
	Error                     ErrCode = -1
	Success                   ErrCode = 0
	ErrInvalidInput           ErrCode = 45003
	ErrInvalidOutput          ErrCode = 45004
	ErrAssetPrecision         ErrCode = 45005
	ErrTransactionBalance     ErrCode = 45006
	ErrAttributeProgram       ErrCode = 45007
	ErrTransactionSignature   ErrCode = 45008
	ErrTransactionPayload     ErrCode = 45009
	ErrDoubleSpend            ErrCode = 45010
	ErrTransactionDuplicate   ErrCode = 45011
	ErrSidechainTxDuplicate   ErrCode = 45012
	ErrXmitFail               ErrCode = 45014
	ErrTransactionSize        ErrCode = 45015
	ErrUnknownReferredTx      ErrCode = 45016
	ErrIneffectiveCoinbase    ErrCode = 45018
	ErrUTXOLocked             ErrCode = 45019
	ErrSideChainPowConsensus  ErrCode = 45020
	ErrReturnDepositConsensus ErrCode = 45021
	ErrProducerProcessing     ErrCode = 45022
	ErrProducerNodeProcessing ErrCode = 45023
	ErrTransactionPoolSize    ErrCode = 45024

	SessionExpired       ErrCode = 41001
	IllegalDataFormat    ErrCode = 41003
	PowServiceNotStarted ErrCode = 41004
	InvalidMethod        ErrCode = 42001
	InvalidParams        ErrCode = 42002
	InvalidToken         ErrCode = 42003
	InvalidTransaction   ErrCode = 43001
	InvalidAsset         ErrCode = 43002
	UnknownTransaction   ErrCode = 44001
	UnknownAsset         ErrCode = 44002
	UnknownBlock         ErrCode = 44003
	InternalError        ErrCode = 45002
)

var ErrMap = map[ErrCode]string{
	Error:                     "Unclassified error",
	Success:                   "Success",
	SessionExpired:            "Session expired",
	IllegalDataFormat:         "Illegal Dataformat",
	PowServiceNotStarted:      "pow service not started",
	InvalidMethod:             "Invalid method",
	InvalidParams:             "Invalid Params",
	InvalidToken:              "Verify token error",
	InvalidTransaction:        "Invalid transaction",
	InvalidAsset:              "Invalid asset",
	UnknownTransaction:        "Unknown Transaction",
	UnknownAsset:              "Unknown asset",
	UnknownBlock:              "Unknown Block",
	InternalError:             "Internal error",
	ErrUTXOLocked:             "Error utxo locked",
	ErrSideChainPowConsensus:  "Error sidechain pow consensus",
	ErrReturnDepositConsensus: "Error return deposit consensus",
	ErrProducerProcessing:     "Error producer processing",
	ErrProducerNodeProcessing: "Error producer node processing",
	ErrTransactionPoolSize:    "Error transactions size of transaction pool",
	ErrInvalidInput:           "INTERNAL ERROR, ErrInvalidInput",
	ErrInvalidOutput:          "INTERNAL ERROR, ErrInvalidOutput",
	ErrAssetPrecision:         "INTERNAL ERROR, ErrAssetPrecision",
	ErrTransactionBalance:     "INTERNAL ERROR, ErrTransactionBalance",
	ErrAttributeProgram:       "INTERNAL ERROR, ErrAttributeProgram",
	ErrTransactionSignature:   "INTERNAL ERROR, ErrTransactionSignature",
	ErrTransactionPayload:     "INTERNAL ERROR, ErrTransactionPayload",
	ErrDoubleSpend:            "INTERNAL ERROR, ErrDoubleSpend",
	ErrTransactionDuplicate:   "INTERNAL ERROR, ErrTransactionDuplicate",
	ErrSidechainTxDuplicate:   "INTERNAL ERROR, ErrSidechainTxDuplicate",
	ErrXmitFail:               "INTERNAL ERROR, ErrXmitFail",
	ErrTransactionSize:        "INTERNAL ERROR, ErrTransactionSize",
	ErrUnknownReferredTx:      "INTERNAL ERROR, ErrUnknownReferredTx",
	ErrIneffectiveCoinbase:    "INTERNAL ERROR, ErrIneffectiveCoinbase",
}

func (code ErrCode) Error() string {
	return ErrMap[code]
}
