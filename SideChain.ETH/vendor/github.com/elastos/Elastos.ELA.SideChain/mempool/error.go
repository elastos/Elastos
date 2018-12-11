package mempool

import "fmt"

var ErrBreak = fmt.Errorf("break out from here")

type ErrorCode int

const (
	ErrInvalidInput         ErrorCode = 45003
	ErrInvalidOutput        ErrorCode = 45004
	ErrAssetPrecision       ErrorCode = 45005
	ErrTransactionBalance   ErrorCode = 45006
	ErrAttributeProgram     ErrorCode = 45007
	ErrTransactionSignature ErrorCode = 45008
	ErrTransactionPayload   ErrorCode = 45009
	ErrDoubleSpend          ErrorCode = 45010
	ErrTxHashDuplicate      ErrorCode = 45011
	ErrMainchainTxDuplicate ErrorCode = 45013
	ErrTransactionSize      ErrorCode = 45015
	ErrUnknownReferedTx     ErrorCode = 45016
	ErrInvalidReferedTx     ErrorCode = 45017
	ErrIneffectiveCoinbase  ErrorCode = 45018
	ErrUTXOLocked           ErrorCode = 45019
	ErrRechargeToSideChain  ErrorCode = 45020
	ErrCrossChain           ErrorCode = 45021
)

var errorCodeStrings = map[ErrorCode]string{
	ErrUTXOLocked:           "ErrUTXOLocked",
	ErrInvalidInput:         "ErrInvalidInput",
	ErrInvalidOutput:        "ErrInvalidOutput",
	ErrAssetPrecision:       "ErrAssetPrecision",
	ErrTransactionBalance:   "ErrTransactionBalance",
	ErrAttributeProgram:     "ErrAttributeProgram",
	ErrTransactionSignature: "ErrTransactionSignature",
	ErrTransactionPayload:   "ErrTransactionPayload",
	ErrDoubleSpend:          "ErrDoubleSpend",
	ErrTxHashDuplicate:      "ErrTxHashDuplicate",
	ErrMainchainTxDuplicate: "ErrMainchainTxDuplicate",
	ErrUnknownReferedTx:     "ErrUnknownReferedTx",
	ErrInvalidReferedTx:     "ErrInvalidReferedTx",
	ErrIneffectiveCoinbase:  "ErrIneffectiveCoinbase",
	ErrRechargeToSideChain:  "ErrRechargeToSideChain",
	ErrCrossChain:           "ErrCrossChain",
	ErrTransactionSize:      "ErrTransactionSize",
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
