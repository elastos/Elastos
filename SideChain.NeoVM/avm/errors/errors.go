package errors

import "errors"

var (
	ErrBadValue           = errors.New("bad value")
	ErrBadType            = errors.New("bad type")
	ErrOverLen            = errors.New("the count over the size")
	ErrFault              = errors.New("the exeution meet fault")
	ErrUnderStackLen      = errors.New("the count under the stack length")
	ErrNotSupportOpCode   = errors.New("does not support the operation code")
	ErrOverLimitStack     = errors.New("the stack over max size")
	ErrOverMaxItemSize    = errors.New("the item over max size")
	ErrOverMaxArraySize   = errors.New("the array over max size")
	ErrOverStackLen       = errors.New("the count over the stack length")
	ErrOverBigIntegerSize = errors.New("the big int is over 32 bytes")
	ErrOutOfGas           = errors.New("out of gas")
	ErrOverCodeLen        = errors.New("the count over the code length")
	ErrNotArray           = errors.New("not array")
	ErrorProgramCode      = errors.New("[ToProgramHash] failed, empty program code")
	ErrTableIsNil         = errors.New("table is nil")
	ErrNotFindScript      = errors.New("can not find script")
	ErrServiceIsNil       = errors.New("service is nil")
	ErrNotSupportSysCall  = errors.New("does not support the sysCall")
	ErrNumericOverFlow    = errors.New("the number is over flow")
)
