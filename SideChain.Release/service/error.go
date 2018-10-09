package service

import "fmt"

var ErrBreak = fmt.Errorf("break out from here")

type ErrorCode int

const (
	Error                   ErrorCode = -1
	Success                 ErrorCode = 0
	ErrInvalidInput         ErrorCode = 45003
	ErrInvalidOutput        ErrorCode = 45004
	ErrAssetPrecision       ErrorCode = 45005
	ErrTransactionBalance   ErrorCode = 45006
	ErrAttributeProgram     ErrorCode = 45007
	ErrTransactionSignature ErrorCode = 45008
	ErrTransactionPayload   ErrorCode = 45009
	ErrDoubleSpend          ErrorCode = 45010
	ErrTxHashDuplicate      ErrorCode = 45011
	ErrSidechainTxDuplicate ErrorCode = 45012
	ErrMainchainTxDuplicate ErrorCode = 45013
	ErrXmitFail             ErrorCode = 45014
	ErrTransactionSize      ErrorCode = 45015
	ErrUnknownReferedTx     ErrorCode = 45016
	ErrInvalidReferedTx     ErrorCode = 45017
	ErrIneffectiveCoinbase  ErrorCode = 45018
	ErrUTXOLocked           ErrorCode = 45019
	ErrRechargeToSideChain  ErrorCode = 45020
	ErrCrossChain           ErrorCode = 45021

	SessionExpired       ErrorCode = 41001
	IllegalDataFormat    ErrorCode = 41003
	PowServiceNotStarted ErrorCode = 41004
	InvalidMethod        ErrorCode = 42001
	InvalidParams        ErrorCode = 42002
	InvalidToken         ErrorCode = 42003
	InvalidTransaction   ErrorCode = 43001
	InvalidAsset         ErrorCode = 43002
	UnknownTransaction   ErrorCode = 44001
	UnknownAsset         ErrorCode = 44002
	UnknownBlock         ErrorCode = 44003
	InternalError        ErrorCode = 45002
)

var errorCodeStrings = map[ErrorCode]string{
	Error:                   "Unclassified error",
	Success:                 "Success",
	SessionExpired:          "Session expired",
	IllegalDataFormat:       "Illegal Dataformat",
	PowServiceNotStarted:    "pow service not started",
	InvalidMethod:           "Invalid method",
	InvalidParams:           "Invalid Params",
	InvalidToken:            "Verify token error",
	InvalidTransaction:      "Invalid transaction",
	InvalidAsset:            "Invalid asset",
	UnknownTransaction:      "Unknown Transaction",
	UnknownAsset:            "Unknown asset",
	UnknownBlock:            "Unknown Block",
	InternalError:           "Internal error",
	ErrUTXOLocked:           "Error utxo locked",
	ErrInvalidInput:         "INTERNAL ERROR, ErrInvalidInput",
	ErrInvalidOutput:        "INTERNAL ERROR, ErrInvalidOutput",
	ErrAssetPrecision:       "INTERNAL ERROR, ErrAssetPrecision",
	ErrTransactionBalance:   "INTERNAL ERROR, ErrTransactionBalance",
	ErrAttributeProgram:     "INTERNAL ERROR, ErrAttributeProgram",
	ErrTransactionSignature: "INTERNAL ERROR, ErrTransactionSignature",
	ErrTransactionPayload:   "INTERNAL ERROR, ErrTransactionPayload",
	ErrDoubleSpend:          "INTERNAL ERROR, ErrDoubleSpend",
	ErrTxHashDuplicate:      "INTERNAL ERROR, ErrTxHashDuplicate",
	ErrSidechainTxDuplicate: "INTERNAL ERROR, ErrSidechainTxDuplicate",
	ErrMainchainTxDuplicate: "INTERNAL ERROR, ErrMainchainTxDuplicate",
	ErrXmitFail:             "INTERNAL ERROR, ErrXmitFail",
	ErrTransactionSize:      "INTERNAL ERROR, ErrTransactionSize",
	ErrUnknownReferedTx:     "INTERNAL ERROR, ErrUnknownReferedTx",
	ErrInvalidReferedTx:     "INTERNAL ERROR, ErrInvalidReferedTx",
	ErrIneffectiveCoinbase:  "INTERNAL ERROR, ErrIneffectiveCoinbase",
	ErrRechargeToSideChain:  "INTERNAL ERROR, ErrRechargeToSideChain",
	ErrCrossChain:           "INTERNAL ERROR, ErrCrossChain",
}

// String returns the ErrorCode as a human-readable name.
func (e ErrorCode) String() string {
	if s := errorCodeStrings[e]; s != "" {
		return s
	}
	return fmt.Sprintf("Unknown ErrorCode (%d)", int(e))
}

// RuleError identifies a rule violation.  It is used to indicate that
// processing of a block or transaction failed due to one of the many validation
// rules.  The caller can use type assertions to determine if a failure was
// specifically due to a rule violation and access the ErrorCode field to
// ascertain the specific reason for the rule violation.
type RuleError struct {
	ErrorCode   ErrorCode // Describes the kind of error
	Description string    // Human readable description of the issue
}

// Error satisfies the error interface and prints human-readable errors.
func (e RuleError) Error() string {
	return e.Description
}

// ruleError creates an RuleError given a set of arguments.
func ruleError(c ErrorCode, desc string) RuleError {
	return RuleError{ErrorCode: c, Description: desc}
}
