// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package tx

import (
	"github.com/elastos/Elastos.ELA/account"
	"github.com/elastos/Elastos.ELA/benchmark/common/utils"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
)

const (
	defaultAmount = 10000000 // 0.1 ELA
)

type fixAmountAssigner struct {
	account *account.Account
	utxo    *types.UTXO
}

func (a *fixAmountAssigner) SignAndChange(tx *types.Transaction) error {
	tx.Inputs = []*types.Input{
		{
			Previous: types.OutPoint{
				TxID:  a.utxo.TxID,
				Index: a.utxo.Index,
			},
			Sequence: 0,
		},
	}

	for _, o := range tx.Outputs {
		o.Value = defaultAmount
	}
	tx.Outputs = append(tx.Outputs, &types.Output{
		AssetID: config.ELAAssetID,
		Value: a.utxo.Value -
			defaultAmount*common.Fixed64(len(tx.Outputs)) - defaultFee,
		OutputLock:  0,
		ProgramHash: a.account.ProgramHash,
		Type:        types.OTNone,
		Payload:     &outputpayload.DefaultOutput{},
	})

	return utils.SignStandardTx(tx, a.account)
}
