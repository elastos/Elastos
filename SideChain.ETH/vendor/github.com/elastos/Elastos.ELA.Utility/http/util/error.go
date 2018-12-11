package util

// Error is the error data for the JSON-RPC request.
type Error struct {
	Code    int    `json:"code"`
	Message string `json:"message"`
}

func (e Error) Error() string {
	return e.Message
}

// NewError returns a new Error instance.
func NewError(code int, message string) *Error {
	return &Error{Code: code, Message: message}
}
