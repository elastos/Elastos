// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

// Copyright (c) 2013-2016 The btcsuite developers
// Copyright (c) 2017-2019 Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package blockchain

import (
	"github.com/elastos/Elastos.ELA/core/types"

	"github.com/btcsuite/btcd/wire"
)

const (
	// MaxBlockWeight defines the maximum block weight, where "block
	// weight" is interpreted as defined in BIP0141. A block's weight is
	// calculated as the sum of the of bytes in the existing transactions
	// and header, plus the weight of each byte within a transaction. The
	// weight of a "base" byte is 4, while the weight of a witness byte is
	// 1. As a result, for a block to be valid, the BlockWeight MUST be
	// less than, or equal to MaxBlockWeight.
	MaxBlockWeight = 4000000

	// MaxBlockBaseSize is the maximum number of bytes within a block
	// which can be allocated to non-witness data.
	MaxBlockBaseSize = 1000000

	// MaxBlockSigOpsCost is the maximum number of signature operations
	// allowed for a block. It is calculated via a weighted algorithm which
	// weights segregated witness sig ops lower than regular sig ops.
	MaxBlockSigOpsCost = 80000

	// WitnessScaleFactor determines the level of "discount" witness data
	// receives compared to "base" data. A scale factor of 4, denotes that
	// witness data is 1/4 as cheap as regular non-witness data.
	WitnessScaleFactor = 4

	// MinTxOutputWeight is the minimum possible weight for a transaction
	// output.
	MinTxOutputWeight = WitnessScaleFactor * wire.MinTxOutPayload

	// MaxOutputsPerBlock is the maximum number of transaction outputs there
	// can be in a block of max weight size.
	MaxOutputsPerBlock = MaxBlockWeight / MinTxOutputWeight
)

// GetBlockWeight computes the value of the weight metric for a given block.
// Currently the weight metric is simply the sum of the block's serialized size
// without any witness data scaled proportionally by the WitnessScaleFactor,
// and the block's serialized size including any witness data.
func GetBlockWeight(blk *types.Block) int64 {

	baseSize := blk.SerializeSizeStripped()

	// (baseSize * 3) + totalSize
	return int64(baseSize)
}

// GetTransactionWeight computes the value of the weight metric for a given
// transaction. Currently the weight metric is simply the sum of the
// transactions's serialized size without any witness data scaled
// proportionally by the WitnessScaleFactor, and the transaction's serialized
// size including any witness data.
func GetTransactionWeight(tx *types.Transaction) int64 {
	baseSize := tx.SerializeSizeStripped()
	return int64(baseSize)
}
