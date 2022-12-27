package service

import "fmt"

type ErrorCode int

const (
	InvalidTransaction    ErrorCode = -32000
	ResolverInternalError ErrorCode = -32001
	UnknownTransaction    ErrorCode = -32002
	UnknownAsset          ErrorCode = -32003
	UnknownBlock          ErrorCode = -32004
	InvalidAsset          ErrorCode = -32005
	InvalidRequest        ErrorCode = -32600
	MethodNotFound        ErrorCode = -32601
	InvalidParams         ErrorCode = -32602
	InternalError         ErrorCode = -32603
	ParseError            ErrorCode = -32700
)

var errorCodeStrings = map[ErrorCode]string{
	InvalidParams:         "Invalid Params",
	InvalidTransaction:    "Invalid transaction",
	InvalidAsset:          "Invalid asset",
	UnknownTransaction:    "Unknown Transaction",
	UnknownAsset:          "Unknown asset",
	UnknownBlock:          "Unknown Block",
	InternalError:         "Internal error",
	InvalidRequest:        "Invalid Request",
	ResolverInternalError: "Resolver Internal Error",
	MethodNotFound:        "Method Not Found",
	ParseError:            "Parse Error",
}

// String returns the ErrorCode as a human-readable name.
func (e ErrorCode) String() string {
	if s := errorCodeStrings[e]; s != "" {
		return s
	}
	return fmt.Sprintf("Unknown ErrorCode (%d)", int(e))
}
