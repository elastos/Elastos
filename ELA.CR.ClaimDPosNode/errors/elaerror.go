package errors

type ELAError interface {
	error

	Code() ErrCode
	InnerError() error
}
