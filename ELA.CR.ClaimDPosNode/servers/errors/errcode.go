// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package errors

type ServerErrCode int

const (
	Error                       ServerErrCode = -1
	Success                     ServerErrCode = 0
	ErrInvalidInput             ServerErrCode = 45003
	ErrInvalidOutput            ServerErrCode = 45004
	ErrAssetPrecision           ServerErrCode = 45005
	ErrTransactionBalance       ServerErrCode = 45006
	ErrAttributeProgram         ServerErrCode = 45007
	ErrTransactionSignature     ServerErrCode = 45008
	ErrTransactionPayload       ServerErrCode = 45009
	ErrDoubleSpend              ServerErrCode = 45010
	ErrTransactionDuplicate     ServerErrCode = 45011
	ErrSidechainTxDuplicate     ServerErrCode = 45012
	ErrXmitFail                 ServerErrCode = 45014
	ErrTransactionSize          ServerErrCode = 45015
	ErrUnknownReferredTx        ServerErrCode = 45016
	ErrIneffectiveCoinbase      ServerErrCode = 45018
	ErrUTXOLocked               ServerErrCode = 45019
	ErrSideChainPowConsensus    ServerErrCode = 45020
	ErrReturnDepositConsensus   ServerErrCode = 45021
	ErrProducerProcessing       ServerErrCode = 45022
	ErrProducerNodeProcessing   ServerErrCode = 45023
	ErrTransactionPoolSize      ServerErrCode = 45024
	ErrCRProcessing             ServerErrCode = 45025
	ErrTransactionHeightVersion ServerErrCode = 45026

	SessionExpired       ServerErrCode = 41001
	IllegalDataFormat    ServerErrCode = 41003
	PowServiceNotStarted ServerErrCode = 41004
	InvalidMethod        ServerErrCode = 42001
	InvalidParams        ServerErrCode = 42002
	InvalidToken         ServerErrCode = 42003
	InvalidTransaction   ServerErrCode = 43001
	InvalidAsset         ServerErrCode = 43002
	UnknownTransaction   ServerErrCode = 44001
	UnknownAsset         ServerErrCode = 44002
	UnknownBlock         ServerErrCode = 44003
	InternalError        ServerErrCode = 45002
)

var ErrMap = map[ServerErrCode]string{
	Error:                       "Unclassified error",
	Success:                     "Success",
	SessionExpired:              "Session expired",
	IllegalDataFormat:           "Illegal Dataformat",
	PowServiceNotStarted:        "pow service not started",
	InvalidMethod:               "Invalid method",
	InvalidParams:               "Invalid Params",
	InvalidToken:                "Verify token error",
	InvalidTransaction:          "Invalid transaction",
	InvalidAsset:                "Invalid asset",
	UnknownTransaction:          "Unknown Transaction",
	UnknownAsset:                "Unknown asset",
	UnknownBlock:                "Unknown Block",
	InternalError:               "Internal error",
	ErrUTXOLocked:               "Error utxo locked",
	ErrSideChainPowConsensus:    "Error sidechain pow consensus",
	ErrReturnDepositConsensus:   "Error return deposit consensus",
	ErrProducerProcessing:       "Error producer processing",
	ErrProducerNodeProcessing:   "Error producer node processing",
	ErrTransactionPoolSize:      "Error transactions size of transaction pool",
	ErrCRProcessing:             "Error CR processing",
	ErrTransactionHeightVersion: "Error height version of transaction",
	ErrInvalidInput:             "INTERNAL ERROR, ErrInvalidInput",
	ErrInvalidOutput:            "INTERNAL ERROR, ErrInvalidOutput",
	ErrAssetPrecision:           "INTERNAL ERROR, ErrAssetPrecision",
	ErrTransactionBalance:       "INTERNAL ERROR, ErrTransactionBalance",
	ErrAttributeProgram:         "INTERNAL ERROR, ErrAttributeProgram",
	ErrTransactionSignature:     "INTERNAL ERROR, ErrTransactionSignature",
	ErrTransactionPayload:       "INTERNAL ERROR, ErrTransactionPayload",
	ErrDoubleSpend:              "INTERNAL ERROR, ErrDoubleSpend",
	ErrTransactionDuplicate:     "INTERNAL ERROR, ErrTransactionDuplicate",
	ErrSidechainTxDuplicate:     "INTERNAL ERROR, ErrSidechainTxDuplicate",
	ErrXmitFail:                 "INTERNAL ERROR, ErrXmitFail",
	ErrTransactionSize:          "INTERNAL ERROR, ErrTransactionSize",
	ErrUnknownReferredTx:        "INTERNAL ERROR, ErrUnknownReferredTx",
	ErrIneffectiveCoinbase:      "INTERNAL ERROR, ErrIneffectiveCoinbase",
}

func (code ServerErrCode) Error() string {
	return ErrMap[code]
}
