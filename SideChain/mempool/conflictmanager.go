// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package mempool

import (
	"errors"
	"fmt"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/types"
)

const (
	SlotRechargeToSidechainTxHash = "RechargeToSidechainTxHash"
	SlotTxInputsReferKeys         = "TxInputsReferKeys"
)

type Conflict struct {
	Name string
	Slot *conflictSlot
}

// conflictManager hold a set of conflict slots, and refer some query methods.
type conflictManager struct {
	chain         *blockchain.BlockChain
	conflictSlots []*Conflict
}

func (m *conflictManager) VerifyTx(tx *types.Transaction) error {
	for _, v := range m.conflictSlots {
		if err := v.Slot.VerifyTx(m.chain, tx); err != nil {
			return errors.New(fmt.Sprintf("Slot %s verify tx error: %s",
				v.Name, err.Error()))
		}
	}
	return nil
}

func (m *conflictManager) AppendTx(tx *types.Transaction) error {
	for _, v := range m.conflictSlots {
		if err := v.Slot.AppendTx(m.chain, tx); err != nil {
			return errors.New(fmt.Sprintf("Slot %s append tx error:%s",
				v.Name, err.Error()))
		}
	}
	return nil
}

func (m *conflictManager) removeTx(tx *types.Transaction) error {
	for _, v := range m.conflictSlots {
		if err := v.Slot.RemoveTx(m.chain, tx); err != nil {
			return errors.New(fmt.Sprintf("Slot %s remove tx error:%s",
				v.Name, err.Error()))
		}
	}
	return nil
}

func (m *conflictManager) GetTx(key interface{},
	slotName string) *types.Transaction {
	for _, v := range m.conflictSlots {
		if v.Name == slotName {
			return v.Slot.GetTx(key)
		}
	}
	return nil
}

func (m *conflictManager) ContainsKey(key interface{}, slotName string) bool {
	for _, v := range m.conflictSlots {
		if v.Name == slotName {
			return v.Slot.Contains(key)
		}
	}
	return false
}

func (m *conflictManager) RemoveKey(key interface{},
	slotName string) error {
	for _, v := range m.conflictSlots {
		if v.Name == slotName {
			return v.Slot.removeKey(key)
		}
	}
	return errors.New(fmt.Sprintf("Slot %s not exist", slotName))
}

func (m *conflictManager) Empty() bool {
	for _, v := range m.conflictSlots {
		if !v.Slot.Empty() {
			return false
		}
	}
	return true
}

func (m *conflictManager) AddConflictSlot(conflict *Conflict) {
	m.conflictSlots = append(m.conflictSlots, conflict)
}

func newConflictManager(chain *blockchain.BlockChain) conflictManager {
	return conflictManager{
		chain: chain,
		conflictSlots: []*Conflict{
			// recharge to side chain transaction Hash
			{
				Name: SlotRechargeToSidechainTxHash,
				Slot: NewConflictSlot(Hash,
					KeyTypeFuncPair{
						Type: types.RechargeToSideChain,
						Func: addRechargeToSideChainTransactionHash,
					},
				),
			},
			// tx inputs refer keys
			{
				Name: SlotTxInputsReferKeys,
				Slot: NewConflictSlot(StrArray,
					KeyTypeFuncPair{
						Type: allType,
						Func: strArrayTxReferences,
					},
				),
			},
		},
	}
}
