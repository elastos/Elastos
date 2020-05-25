// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package errors

import (
	"fmt"
	"testing"

	"github.com/stretchr/testify/assert"
)

var errCodeRanges = [][]int{
	{int(Success), int(ErrFail)},
	{int(ErrBlockFailure), int(ErrBlockFailure)},
	{int(ErrBlockSerializeDeserialize), int(ErrBlockSerializeDeserialize)},
	{int(ErrBlockValidation), int(ErrBlockIneffectiveCoinbase)},
	{int(ErrTxFailure), int(ErrTxFailure)},
	{int(ErrTxSerializeDeserialize), int(ErrTxSerializeDeserialize)},
	{int(ErrTxValidation), int(ErrTxReturnDeposit)},
	{int(ErrTxSidechainValidation), int(ErrTxSidechainPowConsensus)},
	{int(ErrDPoSFailure), int(ErrDPoSFailure)},
	{int(ErrCRFailure), int(ErrCRFailure)},
	{int(ErrDbFailure), int(ErrDbFailure)},
	{int(ErrP2pFailure), int(ErrP2pFailure)},
	{int(ErrP2pReject), int(ErrP2pRejectCheckpoint)},
	{int(ErrPoolFailure), int(ErrPoolFailure)},
	{int(ErrTxPoolFailure), int(ErrTxPoolDoubleSpend)},
}

func TestErrMap(t *testing.T) {
	for _, item := range errCodeRanges {
		for i := item[1]; i <= item[0]; i++ {
			_, exist := ErrMap[ErrCode(i)]
			assert.True(t, exist, fmt.Sprintf("%d", i))
		}
	}
}
