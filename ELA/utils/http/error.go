// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package http

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
