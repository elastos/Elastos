// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package errors

import (
	"errors"
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestSimple(t *testing.T) {
	err := Simple(ErrFail, nil)
	assert.Equal(t, ErrFail, err.Code())
	assert.Equal(t, ErrMap[ErrFail], err.Error())
	assert.Equal(t, nil, err.InnerError())

	innerErr := errors.New("inner error")
	err = Simple(ErrFail, innerErr)
	assert.Equal(t, ErrFail, err.Code())
	assert.Equal(t, ErrMap[ErrFail], err.Error())
	assert.Equal(t, innerErr, err.InnerError())
}

func TestSimpleWithMessage(t *testing.T) {
	errMsg := "my message"
	err := SimpleWithMessage(ErrFail, nil, errMsg)
	assert.Equal(t, ErrFail, err.Code())
	assert.Equal(t, errMsg, err.Error())
	assert.Equal(t, nil, err.InnerError())

	innerErr := errors.New("inner error")
	err = SimpleWithMessage(ErrFail, innerErr, errMsg)
	assert.Equal(t, ErrFail, err.Code())
	assert.Equal(t, errMsg, err.Error())
	assert.Equal(t, innerErr, err.InnerError())
}
