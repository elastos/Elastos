package errors

import (
	"testing"
)

func TestErrCode_Message(t *testing.T) {
	errorCodeArray := []ErrCode{
		Error,
		Success,
		ErrInvalidInput,
		ErrInvalidOutput,
		ErrAssetPrecision,
		ErrTransactionBalance,
		ErrAttributeProgram,
		ErrTransactionSignature,
		ErrTransactionPayload,
		ErrDoubleSpend,
		ErrTransactionDuplicate,
		ErrSidechainTxDuplicate,
		ErrXmitFail,
		ErrTransactionSize,
		ErrUnknownReferredTx,
		ErrIneffectiveCoinbase,
		ErrUTXOLocked,
		ErrSideChainPowConsensus,
		SessionExpired,
		IllegalDataFormat,
		PowServiceNotStarted,
		InvalidMethod,
		InvalidParams,
		InvalidToken,
		InvalidTransaction,
		InvalidAsset,
		UnknownTransaction,
		UnknownAsset,
		UnknownBlock,
		InternalError,
	}
	for _, errorCode := range errorCodeArray {
		message := errorCode.Error()
		if message == "" {
			t.Error(errorCode, "Should have message")
		}
	}
}
