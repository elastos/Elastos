// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package blockchain

import (
	"math/big"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/stretchr/testify/assert"
)

const (
	bits             = 0x1c0008ff
	blockCount       = 120
	timespanPerBlock = 120 // second
)

func init() {
	testing.Init()
}

func TestBlockChain_GetNetworkHashPS(t *testing.T) {
	firstHeader := &types.Header{
		Previous:  common.Uint256{},
		Timestamp: uint32(10000000),
		Bits:      bits,
		Height:    uint32(0),
	}
	firstHash := firstHeader.Hash()
	firstNode := NewBlockNode(firstHeader, &firstHash)

	parentNode := firstNode
	tipNode := firstNode

	for i := 1; i < blockCount; i++ {
		header := &types.Header{
			Previous:  common.Uint256{},
			Timestamp: uint32(10000000 + i*timespanPerBlock),
			Bits:      bits,
			Height:    uint32(i),
		}
		hash := header.Hash()
		node := NewBlockNode(header, &hash)
		node.Parent = parentNode
		node.WorkSum = node.WorkSum.Add(node.WorkSum, parentNode.WorkSum)

		parentNode = node
		tipNode = node
	}
	hashPS := getNetworkHashPS(tipNode)

	// calc actual hash per second
	timeDiff := big.NewInt(timespanPerBlock * (blockCount - 1))
	workDiff := new(big.Int).Mul(CalcWork(bits), big.NewInt(blockCount))
	actualHashPS := new(big.Int).Div(workDiff, timeDiff)

	assert.Equal(t, actualHashPS, hashPS)
}
