// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package mempool

import (
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"

	"github.com/stretchr/testify/assert"
)

var (
	simpleGetString = func(tx *types.Transaction) (interface{}, error) {
		return "simple string", nil
	}
	simpleGetHash = func(tx *types.Transaction) (interface{}, error) {
		return common.Uint256{}, nil
	}
	simpleGetProgramHash = func(tx *types.Transaction) (interface{}, error) {
		return common.Uint168{}, nil
	}
)

func TestConflictSlot_AppendTx_keyType_string(t *testing.T) {
	tx := &types.Transaction{
		TxType: types.TransferAsset,
	}

	slot := newConflictSlot(str,
		keyTypeFuncPair{types.TransferAsset, simpleGetString})
	assert.NoError(t, slot.AppendTx(tx))
	assert.Equal(t, 1, len(slot.stringSet))
	assert.Equal(t, 0, len(slot.hashSet))
	assert.Equal(t, 0, len(slot.programHashSet))

	slot = newConflictSlot(str,
		keyTypeFuncPair{types.TransferAsset, simpleGetHash})
	assert.Error(t, slot.AppendTx(tx),
		"keyType and getKeyFunc not matched")

	slot = newConflictSlot(str,
		keyTypeFuncPair{types.TransferAsset, simpleGetProgramHash})
	assert.Error(t, slot.AppendTx(tx),
		"keyType and getKeyFunc not matched")
}

func TestConflictSlot_AppendTx_keyType_hash(t *testing.T) {
	tx := &types.Transaction{
		TxType: types.TransferAsset,
	}

	slot := newConflictSlot(hash,
		keyTypeFuncPair{types.TransferAsset, simpleGetHash})
	assert.NoError(t, slot.AppendTx(tx))
	assert.Equal(t, 0, len(slot.stringSet))
	assert.Equal(t, 1, len(slot.hashSet))
	assert.Equal(t, 0, len(slot.programHashSet))

	slot = newConflictSlot(hash,
		keyTypeFuncPair{types.TransferAsset, simpleGetString})
	assert.Error(t, slot.AppendTx(tx),
		"keyType and getKeyFunc not matched")

	slot = newConflictSlot(hash,
		keyTypeFuncPair{types.TransferAsset, simpleGetProgramHash})
	assert.Error(t, slot.AppendTx(tx),
		"keyType and getKeyFunc not matched")
}

func TestConflictSlot_AppendTx_keyType_programHash(t *testing.T) {
	tx := &types.Transaction{
		TxType: types.TransferAsset,
	}

	slot := newConflictSlot(programHash,
		keyTypeFuncPair{types.TransferAsset, simpleGetProgramHash})
	assert.NoError(t, slot.AppendTx(tx))
	assert.Equal(t, 0, len(slot.stringSet))
	assert.Equal(t, 0, len(slot.hashSet))
	assert.Equal(t, 1, len(slot.programHashSet))

	slot = newConflictSlot(programHash,
		keyTypeFuncPair{types.TransferAsset, simpleGetString})
	assert.Error(t, slot.AppendTx(tx),
		"keyType and getKeyFunc not matched")

	slot = newConflictSlot(programHash,
		keyTypeFuncPair{types.TransferAsset, simpleGetHash})
	assert.Error(t, slot.AppendTx(tx),
		"keyType and getKeyFunc not matched")
}

func TestConflictSlot_VerifyTx(t *testing.T) {
	// defined slot allowed TransferAsset and CRCProposal tx
	slot := newConflictSlot(str,
		keyTypeFuncPair{types.TransferAsset, simpleGetString},
		keyTypeFuncPair{types.CRCProposal, simpleGetString})

	// defined a tx that is not supported  by the slot
	tx1 := &types.Transaction{
		TxType: types.CancelProducer,
	}
	tx2 := &types.Transaction{
		TxType: types.TransferAsset,
	}
	tx3 := &types.Transaction{
		TxType: types.CRCProposal,
	}

	assert.NoError(t, slot.VerifyTx(tx1))
	assert.NoError(t, slot.AppendTx(tx1))
	assert.Equal(t, 0, len(slot.stringSet),
		"unsupported will return no error and have no effect to this slot")

	assert.NoError(t, slot.VerifyTx(tx2))
	assert.NoError(t, slot.AppendTx(tx2))
	assert.Equal(t, 1, len(slot.stringSet))

	assert.Error(t, slot.VerifyTx(tx3),
		"same key shall be  added only once")
}

func TestConflictSlot_RemoveTx(t *testing.T) {
	tx := &types.Transaction{
		TxType: types.TransferAsset,
	}
	slot := newConflictSlot(str,
		keyTypeFuncPair{types.TransferAsset, simpleGetString})

	assert.NoError(t, slot.AppendTx(tx))
	assert.Equal(t, 1, len(slot.stringSet))

	assert.NoError(t, slot.RemoveTx(tx))
	assert.Equal(t, 0, len(slot.stringSet))
}
