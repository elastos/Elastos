package sutil

import (
	"fmt"
	"github.com/elastos/Elastos.ELA/common"
)

type STXO struct {
	// When it used to be a UTXO
	UTXO

	// The height at which it met its demise
	SpendHeight uint32

	// The tx that consumed it
	SpendTxId common.Uint256
}

func (stxo *STXO) String() string {
	return fmt.Sprint(
		"STXO:{",
		"UTXO:{",
		"Op:{TxId:", stxo.Op.TxID.String(), ", Index:", stxo.Op.Index, "},",
		"Value:", stxo.Value.String(), ",",
		"LockTime:", stxo.LockTime, ",",
		"AtHeight:", stxo.AtHeight, "},",
		"SendHeight:", stxo.SpendHeight, ",",
		"SpendTxId:", stxo.SpendTxId.String(), "}")
}

func (stxo *STXO) IsEqual(alt *STXO) bool {
	if alt == nil {
		return stxo == nil
	}

	if !stxo.UTXO.IsEqual(&alt.UTXO) {
		return false
	}

	if stxo.SpendHeight != alt.SpendHeight {
		return false
	}

	if !stxo.SpendTxId.IsEqual(alt.SpendTxId) {
		return false
	}

	return true
}

func NewSTXO(utxo *UTXO, spendHeight uint32, spendTxId common.Uint256) *STXO {
	return &STXO{
		UTXO:        *utxo,
		SpendHeight: spendHeight,
		SpendTxId:   spendTxId,
	}
}
