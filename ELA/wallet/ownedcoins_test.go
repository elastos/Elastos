// Copyright (c) 2017-2019 Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package wallet

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/stretchr/testify/assert"
	"testing"

	"github.com/elastos/Elastos.ELA/core/types"
)

var (
	op1 = &types.OutPoint{
		TxID:  common.Uint256{},
		Index: 1,
	}
	op2 = &types.OutPoint{
		TxID:  common.Uint256{},
		Index: 2,
	}
	op3 = &types.OutPoint{
		TxID:  common.Uint256{},
		Index: 3,
	}
	op4 = &types.OutPoint{
		TxID:  common.Uint256{},
		Index: 4,
	}
	op5 = &types.OutPoint{
		TxID:  common.Uint256{},
		Index: 5,
	}
)

func TestOwnedCoins(t *testing.T) {
	ownedCoins := NewOwnedCoins()

	// test append
	ownedCoins.append("vote", op1)
	item0 := ownedCoins.readHead("vote")
	assert.Equal(t, CoinLinkedItem{prev: op1, next: op1}, item0)
	item1 := ownedCoins.read("vote", op1)
	assert.Equal(t, CoinLinkedItem{prev: nil, next: nil}, item1)

	ownedCoins.append("vote", op2)
	item0 = ownedCoins.readHead("vote")
	assert.Equal(t, CoinLinkedItem{prev: op2, next: op1}, item0)
	item1 = ownedCoins.read("vote", op1)
	assert.Equal(t, CoinLinkedItem{prev: nil, next: op2}, item1)
	item2 := ownedCoins.read("vote", op2)
	assert.Equal(t, CoinLinkedItem{prev: op1, next: nil}, item2)

	ownedCoins.append("vote", op3)
	item0 = ownedCoins.readHead("vote")
	assert.Equal(t, CoinLinkedItem{prev: op3, next: op1}, item0)
	item1 = ownedCoins.read("vote", op1)
	assert.Equal(t, CoinLinkedItem{prev: nil, next: op2}, item1)
	item2 = ownedCoins.read("vote", op2)
	assert.Equal(t, CoinLinkedItem{prev: op1, next: op3}, item2)
	item3 := ownedCoins.read("vote", op3)
	assert.Equal(t, CoinLinkedItem{prev: op2, next: nil}, item3)

	// duplicate append
	ownedCoins.append("vote", op2)
	item0 = ownedCoins.readHead("vote")
	assert.Equal(t, CoinLinkedItem{prev: op3, next: op1}, item0)
	item1 = ownedCoins.read("vote", op1)
	assert.Equal(t, CoinLinkedItem{prev: nil, next: op2}, item1)
	item2 = ownedCoins.read("vote", op2)
	assert.Equal(t, CoinLinkedItem{prev: op1, next: op3}, item2)
	item3 = ownedCoins.read("vote", op3)
	assert.Equal(t, CoinLinkedItem{prev: op2, next: nil}, item3)

	// test remove
	ownedCoins.remove("vote", op2)
	item0 = ownedCoins.readHead("vote")
	assert.Equal(t, CoinLinkedItem{prev: op3, next: op1}, item0)
	item1 = ownedCoins.read("vote", op1)
	assert.Equal(t, CoinLinkedItem{prev: nil, next: op3}, item1)
	item3 = ownedCoins.read("vote", op3)
	assert.Equal(t, CoinLinkedItem{prev: op1, next: nil}, item3)
	_, exist := ownedCoins[CoinOwnership{"vote", *op2}]
	assert.Equal(t, false, exist)

	ownedCoins.remove("vote", op1)
	item0 = ownedCoins.readHead("vote")
	assert.Equal(t, CoinLinkedItem{prev: op3, next: op3}, item0)
	item3 = ownedCoins.read("vote", op3)
	assert.Equal(t, CoinLinkedItem{prev: nil, next: nil}, item3)
	_, exist = ownedCoins[CoinOwnership{"vote", *op1}]
	assert.Equal(t, false, exist)

	// duplicate remove
	ownedCoins.remove("vote", op1)
	item0 = ownedCoins.readHead("vote")
	assert.Equal(t, CoinLinkedItem{prev: op3, next: op3}, item0)
	item3 = ownedCoins.read("vote", op3)
	assert.Equal(t, CoinLinkedItem{prev: nil, next: nil}, item3)
	_, exist = ownedCoins[CoinOwnership{"vote", *op1}]
	assert.Equal(t, false, exist)

	ownedCoins.remove("vote", op3)
	item0 = ownedCoins.readHead("vote")
	assert.Equal(t, CoinLinkedItem{prev: nil, next: nil}, item0)
	_, exist = ownedCoins[CoinOwnership{"vote", *op3}]
	assert.Equal(t, false, exist)
}
