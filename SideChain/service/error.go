package service

import "fmt"

type ErrorCode int

const (
	InvalidParams        ErrorCode = 42002
	InvalidTransaction   ErrorCode = 43001
	InvalidAsset         ErrorCode = 43002
	UnknownTransaction   ErrorCode = 44001
	UnknownAsset         ErrorCode = 44002
	UnknownBlock         ErrorCode = 44003
	InternalError        ErrorCode = 45002
)

var errorCodeStrings = map[ErrorCode]string{
	InvalidParams:           "Invalid Params",
	InvalidTransaction:      "Invalid transaction",
	InvalidAsset:            "Invalid asset",
	UnknownTransaction:      "Unknown Transaction",
	UnknownAsset:            "Unknown asset",
	UnknownBlock:            "Unknown Block",
	InternalError:           "Internal error",
}

// String returns the ErrorCode as a human-readable name.
func (e ErrorCode) String() string {
	if s := errorCodeStrings[e]; s != "" {
		return s
	}
	return fmt.Sprintf("Unknown ErrorCode (%d)", int(e))
}