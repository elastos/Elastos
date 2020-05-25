// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package wallet

import (
	"bytes"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"

	"github.com/stretchr/testify/assert"
)

var (
	ccp = NewCoinCheckPoint()

	previousTxID *common.Uint256

	recipientAddr = "EYGv9wNyEMtVHAkGJvdkFLb7FJneRWdbEu"
	recipient     *common.Uint168
	senderAddr    = "EYXFef272UYHKKD56NGe9BxR7pkcuiZja4"
	sender        *common.Uint168

	prevOp types.OutPoint

	tx1    = new(types.Transaction)
	block1 *types.DposBlock
)

func TestInitBlock(t *testing.T) {
	var err error
	previousTxID, err = common.Uint256FromHexString("a704c4c04c70043a2cce34fa95e20f3d33b0a3dc95dd948dee573673b701c7e7")
	assert.NoError(t, err)
	recipient, err = common.Uint168FromAddress(recipientAddr)
	assert.NoError(t, err)
	sender, err = common.Uint168FromAddress(senderAddr)
	assert.NoError(t, err)

	ccp.height = 99
	prevOp = types.OutPoint{
		TxID:  *previousTxID,
		Index: 0,
	}
	ccp.coins[prevOp] = &Coin{
		TxVersion: 0,
		Output: &types.Output{
			Value:       common.Fixed64(52),
			ProgramHash: *sender,
		},
	}
	assert.Equal(t, uint32(99), ccp.height)
	assert.Equal(t, 1, len(ccp.coins))

	tx1 := &types.Transaction{
		Inputs: []*types.Input{
			{
				Previous: prevOp,
				Sequence: 0,
			},
		},
		Outputs: []*types.Output{
			{
				ProgramHash: *recipient,
				Value:       common.Fixed64(50),
			},
			{
				ProgramHash: *sender,
				Value:       common.Fixed64(1),
			},
		},
	}

	block1 = &types.DposBlock{
		Block: &types.Block{
			Header: types.Header{
				Height: 100,
			},
			Transactions: []*types.Transaction{
				tx1,
			},
		},
	}

	addressBook[senderAddr] = nil
	addressBook[recipientAddr] = nil
}

func TestCoinsCheckPoint_BlockSaved(t *testing.T) {
	ccp.OnBlockSaved(block1)

	verifyCoins(ccp.coins, t)
}

func TestCoinsCheckPoint_Serialize_Deserialize(t *testing.T) {
	buf := new(bytes.Buffer)
	err := ccp.Serialize(buf)
	assert.NoError(t, err)
	verifyCoins(ccp.coins, t)

	ccp2 := NewCoinCheckPoint()
	err = ccp2.Deserialize(buf)
	assert.NoError(t, err)
	verifyCoins(ccp2.coins, t)
}

func TestCoinsCheckPoint_ListCoins(t *testing.T) {
	senderCoins := ccp.ListCoins(senderAddr)
	recipientCoins := ccp.ListCoins(recipientAddr)
	coins := make(map[types.OutPoint]*Coin, 0)
	for k, v := range senderCoins {
		coins[k] = v
	}
	for k, v := range recipientCoins {
		coins[k] = v
	}

	verifyCoins(coins, t)
}

func verifyCoins(coins map[types.OutPoint]*Coin, t *testing.T) {
	assert.Equal(t, 2, len(coins))
	_, exist := coins[prevOp]
	assert.Equal(t, false, exist)
	recipientOp := types.OutPoint{
		TxID:  tx1.Hash(),
		Index: 0,
	}
	coin1, exist := coins[recipientOp]
	assert.Equal(t, true, exist)
	assert.Equal(t, uint32(100), coin1.Height)
	assert.Equal(t, *recipient, coin1.Output.ProgramHash)
	assert.Equal(t, common.Fixed64(50), coin1.Output.Value)

	senderOp := types.OutPoint{
		TxID:  tx1.Hash(),
		Index: 1,
	}
	coin2, exist := coins[senderOp]
	assert.Equal(t, true, exist)
	assert.Equal(t, uint32(100), coin2.Height)
	assert.Equal(t, *sender, coin2.Output.ProgramHash)
	assert.Equal(t, common.Fixed64(1), coin2.Output.Value)
}
