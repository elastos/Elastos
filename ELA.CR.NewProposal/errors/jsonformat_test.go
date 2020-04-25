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

func TestToJsonFormatter(t *testing.T) {
	plainErr := errors.New("plain error")
	simpleChildCode := ErrTxAttributeProgram
	simpleChild := Simple(simpleChildCode, plainErr)
	simpleParentCode := ErrTxValidation
	simpleParent := Simple(simpleParentCode, simpleChild)

	formatter := ToJsonFormatter(simpleParent)
	assert.Equal(t, int(simpleParentCode), formatter.Code)
	assert.Equal(t, ErrMap[simpleParentCode], formatter.Description)
	assert.Equal(t, int(simpleChildCode), formatter.Inner.Code)
	assert.Equal(t, ErrMap[simpleChildCode], formatter.Inner.Description)
	assert.Equal(t, int(ErrFail), formatter.Inner.Inner.Code)
	assert.Equal(t, plainErr.Error(), formatter.Inner.Inner.Description)
}
