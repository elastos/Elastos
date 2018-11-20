package errors

import "errors"

var (
	ErrBadValue = errors.New("bad value")
	ErrBadType  = errors.New("bad type")
	ErrOverLen  = errors.New("the count over the size")
	ErrFault    = errors.New("The exeution meet fault")

	ErrNotSupportOpCode = errors.New("does not support the operation code")
	ErrOverLimitStack   = errors.New("the stack over max size")
	ErrOutOfGas         = errors.New("out of gas")

	ErrorProgramCode = errors.New("[ToProgramHash] failed, empty program code")
)
