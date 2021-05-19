// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package errors

import (
	"encoding/json"
)

type JsonFormatter struct {
	Code        int            `json:"Code"`
	Description string         `json:"Description"`
	Inner       *JsonFormatter `json:"Inner"`
}

func (f *JsonFormatter) Format() (string, error) {
	data, err := json.MarshalIndent(f, "", "\t")
	if err != nil {
		return "", err
	}
	return string(data), nil
}

func ToJsonFormatter(err ELAError) *JsonFormatter {
	transform := func(err ELAError) *JsonFormatter {
		return &JsonFormatter{
			Code:        int(err.Code()),
			Description: err.Error(),
			Inner:       nil,
		}
	}

	result := transform(err)

	current := result
	var next *JsonFormatter
	childErr := err.InnerError()

	var elaErr ELAError
	for childErr != nil {
		switch this := childErr.(type) {
		case ELAError:
			elaErr = this
			childErr = elaErr.InnerError()
		default:
			elaErr = SimpleWithMessage(ErrFail, nil, childErr.Error())
			childErr = nil
		}
		next = transform(elaErr)
		current.Inner = next
		current = next
	}
	return result
}
