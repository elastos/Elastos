// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package calculator

import (
	"bytes"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
)

type multiInputsOfOneAccount struct {
	inputSize uint64
	initSize  uint64
}

func (m *multiInputsOfOneAccount) initialSize() uint64 {
	return m.initSize
}

func (m *multiInputsOfOneAccount) increase() uint64 {
	return m.inputSize
}

func newMultiInputsOfOneAccount() (*multiInputsOfOneAccount, error) {
	inputSize, err := calculateInputSize()
	if err != nil {
		return nil, err
	}

	protoTx, err := createSingleInputOutputTx()
	if err != nil {
		return nil, err
	}

	return &multiInputsOfOneAccount{
		inputSize: inputSize,
		initSize:  uint64(protoTx.GetSize()) - inputSize,
	}, nil
}

func calculateInputSize() (uint64, error) {
	input := types.Input{
		Previous: types.OutPoint{
			TxID:  common.Uint256{},
			Index: 0,
		},
		Sequence: 0,
	}

	var buf bytes.Buffer
	if err := input.Serialize(&buf); err != nil {
		return 0, err
	}
	return uint64(buf.Len()), nil
}
