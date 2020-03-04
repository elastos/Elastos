// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package tx

import (
	"github.com/elastos/Elastos.ELA/account"
	"github.com/elastos/Elastos.ELA/core/types"
)

type AssignerType byte

const (
	NoChanges AssignerType = 0x00
	FixAmount AssignerType = 0x01
)

type Generator interface {
	Generate() *types.Transaction
}

type Assigner interface {
	SignAndChange(tx *types.Transaction) error
}

func NewGenerator(txType types.TxType, ac ...*account.Account) Generator {
	switch txType {
	case types.TransferAsset:
		return &transferAssetGenerator{account: ac}
	default:
		return nil
	}
}

func NewAssigner(assignerType AssignerType, ac *account.Account,
	utxo *types.UTXO) Assigner {
	switch assignerType {
	case NoChanges:
		return &noChangesEvenAssigner{account: ac, utxo: utxo}
	case FixAmount:
		return &fixAmountAssigner{account: ac, utxo: utxo}
	default:
		return nil
	}
}
