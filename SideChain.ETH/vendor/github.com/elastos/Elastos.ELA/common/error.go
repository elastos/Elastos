package common

import (
	"fmt"
)

// funcError describes an issue with a function.
//
// This provides a mechanism for the caller to type assert the error to
// differentiate between general io errors such as io.EOF and issues that
// resulted from malformed messages.
type funcError struct {
	Func        string // Function name
	Description string // Human readable description of the issue
}

// Error satisfies the error interface and prints human-readable errors.
func (e *funcError) Error() string {
	if e.Func != "" {
		return fmt.Sprintf("%v: %v", e.Func, e.Description)
	}
	return e.Description
}

// FuncError creates an error for the given function and description.
func FuncError(f string, desc string) error {
	return &funcError{Func: f, Description: desc}
}
