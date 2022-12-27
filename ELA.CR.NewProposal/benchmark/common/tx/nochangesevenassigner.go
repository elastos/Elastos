// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package tx

import (
	"github.com/elastos/Elastos.ELA/account"
	"github.com/elastos/Elastos.ELA/benchmark/common/utils"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
)

const (
	defaultFee = 100
)

type noChangesEvenAssigner struct {
	account *account.Account
	utxo    *types.UTXO
}

func (a *noChangesEvenAssigner) SignAndChange(tx *types.Transaction) error {
	tx.Inputs = []*types.Input{
		{
			Previous: types.OutPoint{
				TxID:  a.utxo.TxID,
				Index: a.utxo.Index,
			},
			Sequence: 0,
		},
	}

	sum := common.Fixed64(0)
	average := (a.utxo.Value - defaultFee) / common.Fixed64(len(tx.Outputs))
	for _, o := range tx.Outputs {
		o.Value = average
		sum += average
	}

	if len(tx.Outputs) > 0 {
		tx.Outputs[0].Value += a.utxo.Value - defaultFee - sum
	}

	return utils.SignStandardTx(tx, a.account)
}
