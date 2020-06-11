// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package errors

import "fmt"

type ErrCode int

const (
	Success ErrCode = 0
	ErrFail ErrCode = -1

	// 1-block
	ErrBlockFailure ErrCode = -10000
	// 11 serialize
	ErrBlockSerializeDeserialize ErrCode = -11000
	// 12 check
	ErrBlockValidation          ErrCode = -12000
	ErrBlockIneffectiveCoinbase ErrCode = -12001

	// 2-transaction
	ErrTxFailure ErrCode = -20000
	// 21 serialize
	ErrTxSerializeDeserialize ErrCode = -21000
	// 22 check
	ErrTxValidation        ErrCode = -22000
	ErrTxInvalidInput      ErrCode = -22001
	ErrTxInvalidOutput     ErrCode = -22002
	ErrTxBalance           ErrCode = -22003
	ErrTxAttributeProgram  ErrCode = -22004
	ErrTxSignature         ErrCode = -22005
	ErrTxPayload           ErrCode = -22006
	ErrTxDoubleSpend       ErrCode = -22007
	ErrTxSize              ErrCode = -22008
	ErrTxUnknownReferredTx ErrCode = -22009
	ErrTxUTXOLocked        ErrCode = -22010
	ErrTxDuplicate         ErrCode = -22011
	ErrTxHeightVersion     ErrCode = -22012
	ErrTxAssetPrecision    ErrCode = -22013
	ErrTxReturnDeposit     ErrCode = -22014
	ErrTxAppropriation     ErrCode = -22015
	ErrTxAssetsRectify     ErrCode = -22016
	ErrTxRealWithdraw      ErrCode = -22017
	ErrTxCRDPOSManagement  ErrCode = -22018
	// 23 sidechain
	ErrTxSidechainValidation   ErrCode = -23000
	ErrTxSidechainDuplicate    ErrCode = -23001
	ErrTxSidechainPowConsensus ErrCode = -23002

	// 3-DPoS
	ErrDPoSFailure ErrCode = -30000

	// 4-CR
	ErrCRFailure ErrCode = -40000

	// 5-Database
	ErrDbFailure ErrCode = -50000

	// 6-P2P
	ErrP2pFailure ErrCode = -60000
	// 61 reject
	ErrP2pReject                ErrCode = -61000
	ErrP2pRejectMalformed       ErrCode = -61001
	ErrP2pRejectInvalid         ErrCode = -61002
	ErrP2pRejectObsolete        ErrCode = -61003
	ErrP2pRejectDuplicate       ErrCode = -61004
	ErrP2pRejectNonstandard     ErrCode = -61005
	ErrP2pRejectDust            ErrCode = -61006
	ErrP2pRejectInsufficientFee ErrCode = -61007
	ErrP2pRejectCheckpoint      ErrCode = -61008

	// 7-pool
	ErrPoolFailure ErrCode = -70000
	// 71 tx pool
	ErrTxPoolFailure              ErrCode = -71000
	ErrTxPoolOverCapacity         ErrCode = -71001
	ErrTxPoolSidechainTxDuplicate ErrCode = -71002
	ErrTxPoolDPoSTxDuplicate      ErrCode = -71003
	ErrTxPoolCRTxDuplicate        ErrCode = -71004
	ErrTxPoolDoubleSpend          ErrCode = -71005
	ErrTxPoolTypeCastFailure      ErrCode = -71006
	ErrTxPoolTxDuplicate          ErrCode = -71007
)

type SimpleErr struct {
	code    ErrCode
	inner   error
	message string
}

func (e *SimpleErr) Error() string {
	if len(e.message) == 0 {
		return fmt.Sprintf("%s", ErrMap[e.code])
	} else {
		return e.message
	}
}

func (e *SimpleErr) Code() ErrCode {
	return e.code
}

func (e *SimpleErr) InnerError() error {
	return e.inner
}

func Simple(code ErrCode, inner error) *SimpleErr {
	return &SimpleErr{
		code:    code,
		inner:   inner,
		message: "",
	}
}

func SimpleWithMessage(code ErrCode, inner error, message string) *SimpleErr {
	return &SimpleErr{
		code:    code,
		inner:   inner,
		message: message,
	}
}
