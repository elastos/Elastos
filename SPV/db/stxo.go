package db

import "SPVWallet/core"

type STXO struct {
	// When it used to be a UTXO
	UTXO

	// The height at which it met its demise
	SpendHeight uint32

	// The tx that consumed it
	SpendTxid core.Uint256
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

	if !stxo.SpendTxid.IsEqual(&alt.SpendTxid) {
		return false
	}

	return true
}