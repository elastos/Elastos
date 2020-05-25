// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package errors

import "fmt"

const (
	prefixTx       = "transaction"
	prefixBlock    = "block"
	prefixP2p      = "p2p"
	prefixDPoS     = "DPoS"
	prefixCR       = "CR"
	prefixDatabase = "Database"
	prefixPool     = "pool"

	prefixSerialize = "serialize/deserialize"
	prefixValidate  = "validate"
	prefixReject    = "reject"
	prefixSidechain = "about sidechain validate"
	prefixTxPool    = "about transaction"
)

func FormatErrString(mainClass, subClass, message string) string {
	return fmt.Sprintf("%s %s error: %s", mainClass, subClass, message)
}

var ErrMap = map[ErrCode]string{
	Success: "succeed",
	ErrFail: "failed",

	// 1-block
	ErrBlockFailure: FormatErrString(prefixBlock, "",
		"unknown"),

	// 11 serialize
	ErrBlockSerializeDeserialize: FormatErrString(prefixBlock, prefixSerialize,
		"serialize/deserialize failed"),

	// 12 check
	ErrBlockValidation: FormatErrString(prefixBlock, prefixValidate,
		"validation failed"),
	ErrBlockIneffectiveCoinbase: FormatErrString(prefixBlock, prefixValidate,
		"coinbase transaction is invalid"),

	// 2-transaction
	ErrTxFailure: FormatErrString(prefixTx, "", "unknown"),

	// 21 serialize
	ErrTxSerializeDeserialize: FormatErrString(prefixTx, prefixSerialize,
		"serialize/deserialize failed"),

	// 22 check
	ErrTxValidation: FormatErrString(prefixTx, prefixValidate,
		"validation failed"),
	ErrTxInvalidInput: FormatErrString(prefixTx, prefixValidate,
		"input invalid"),
	ErrTxInvalidOutput: FormatErrString(prefixTx, prefixValidate,
		"output invalid"),
	ErrTxBalance: FormatErrString(prefixTx, prefixValidate,
		"balance not matched"),
	ErrTxAttributeProgram: FormatErrString(prefixTx, prefixValidate,
		"attribute or program content invalid"),
	ErrTxSignature: FormatErrString(prefixTx, prefixValidate,
		"signature invalid"),
	ErrTxPayload: FormatErrString(prefixTx, prefixValidate,
		"payload content invalid"),
	ErrTxDoubleSpend: FormatErrString(prefixTx, prefixValidate,
		"double spend"),
	ErrTxSize: FormatErrString(prefixTx, prefixValidate,
		"size over the limit"),
	ErrTxUnknownReferredTx: FormatErrString(prefixTx, prefixValidate,
		"unknown referred transaction"),
	ErrTxUTXOLocked: FormatErrString(prefixTx, prefixValidate,
		"using locked UTXO"),
	ErrTxDuplicate: FormatErrString(prefixTx, prefixValidate,
		"transaction already exist"),
	ErrTxHeightVersion: FormatErrString(prefixTx, prefixValidate,
		"height version invalid"),
	ErrTxAssetPrecision: FormatErrString(prefixTx, prefixValidate,
		"asset precision invalid"),
	ErrTxReturnDeposit: FormatErrString(prefixTx, prefixValidate,
		"return deposit content invalid"),
	// 23 sidechain
	ErrTxSidechainValidation: FormatErrString(prefixTx, prefixSidechain,
		"unknown"),
	ErrTxSidechainDuplicate: FormatErrString(prefixTx, prefixSidechain,
		"already exist"),
	ErrTxSidechainPowConsensus: FormatErrString(prefixTx, prefixSidechain,
		"pow consensus invalid"),

	// 3-DPoS
	ErrDPoSFailure: FormatErrString(prefixDPoS, "", "unknown"),

	// 4-CR
	ErrCRFailure: FormatErrString(prefixCR, "", "unknown"),

	// 5-Database
	ErrDbFailure: FormatErrString(prefixDatabase, "",
		"unknown"),

	// 6-P2P
	ErrP2pFailure: FormatErrString(prefixP2p, "", "unknown"),
	// 61 reject
	ErrP2pReject: FormatErrString(prefixP2p, prefixReject, "unknown"),
	ErrP2pRejectMalformed: FormatErrString(prefixP2p, prefixReject,
		"malformed message"),
	ErrP2pRejectInvalid: FormatErrString(prefixP2p, prefixReject,
		"invalid message"),
	ErrP2pRejectObsolete: FormatErrString(prefixP2p, prefixReject,
		"obsoleted protocol"),
	ErrP2pRejectDuplicate: FormatErrString(prefixP2p, prefixReject,
		"duplicated message"),
	ErrP2pRejectNonstandard: FormatErrString(prefixP2p, prefixReject,
		"message is not standard"),
	ErrP2pRejectDust: FormatErrString(prefixP2p, prefixReject,
		"dust message"),
	ErrP2pRejectInsufficientFee: FormatErrString(prefixP2p, prefixReject,
		"insufficient fee"),
	ErrP2pRejectCheckpoint: FormatErrString(prefixP2p, prefixReject,
		"invalid checkpoint"),

	// 7-pool
	ErrPoolFailure: FormatErrString(prefixPool, "", "unknown"),
	// 71 tx pool
	ErrTxPoolFailure: FormatErrString(prefixPool, prefixTxPool,
		"unknown"),
	ErrTxPoolOverCapacity: FormatErrString(prefixPool, prefixTxPool,
		"over capacity"),
	ErrTxPoolSidechainTxDuplicate: FormatErrString(prefixPool, prefixTxPool,
		"sidechain transaction already exist"),
	ErrTxPoolDPoSTxDuplicate: FormatErrString(prefixPool, prefixTxPool,
		"DPoS transaction conflict"),
	ErrTxPoolCRTxDuplicate: FormatErrString(prefixPool, prefixTxPool,
		"CR transaction conflict"),
	ErrTxPoolDoubleSpend: FormatErrString(prefixPool, prefixTxPool,
		"double spend with transaction in transaction pool"),
}
