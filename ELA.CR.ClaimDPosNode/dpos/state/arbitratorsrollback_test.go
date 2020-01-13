// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package state

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/stretchr/testify/assert"
	"testing"
)

var abt *arbitrators
var abtList [][]byte

func initArbiters() {
	arbitratorsStr := []string{
		"023a133480176214f88848c6eaa684a54b316849df2b8570b57f3a917f19bbc77a",
		"030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9",
		"0288e79636e41edce04d4fa95d8f62fed73a76164f8631ccc42f5425f960e4a0c7",
		"03e281f89d85b3a7de177c240c4961cb5b1f2106f09daa42d15874a38bbeae85dd",
		"0393e823c2087ed30871cbea9fa5121fa932550821e9f3b17acef0e581971efab0",
	}

	abtList = make([][]byte, 0)
	for _, v := range arbitratorsStr {
		a, _ := common.HexStringToBytes(v)
		abtList = append(abtList, a)
	}

	activeNetParams := &config.DefaultParams
	activeNetParams.CRCArbiters = arbitratorsStr
	bestHeight := uint32(0)

	abt, _ = NewArbitrators(activeNetParams,
		nil, nil)
	abt.RegisterFunction(func() uint32 { return bestHeight }, nil, nil)
	abt.State = NewState(activeNetParams, nil, nil)
}

func checkPointEqual(first, second *CheckPoint) bool {
	if !stateKeyFrameEqual(&first.StateKeyFrame, &second.StateKeyFrame) {
		return false
	}

	if first.Height != second.Height || first.DutyIndex != second.DutyIndex ||
		first.crcChangedHeight != second.crcChangedHeight ||
		first.accumulativeReward != second.accumulativeReward ||
		first.finalRoundChange != second.finalRoundChange ||
		first.clearingHeight != second.clearingHeight ||
		len(first.crcArbiters) != len(second.crcArbiters) ||
		len(first.arbitersRoundReward) != len(second.arbitersRoundReward) ||
		len(first.illegalBlocksPayloadHashes) !=
			len(second.illegalBlocksPayloadHashes) {
		return false
	}

	//	rewardEqual
	if !arrayEqual(first.CurrentArbitrators, second.CurrentArbitrators) ||
		!arrayEqual(first.CurrentArbitrators, second.CurrentArbitrators) ||
		!arrayEqual(first.CurrentArbitrators, second.CurrentArbitrators) ||
		!arrayEqual(first.CurrentArbitrators, second.CurrentArbitrators) ||
		!rewardEqual(&first.CurrentReward, &second.CurrentReward) ||
		!rewardEqual(&first.NextReward, &second.NextReward) {
		return false
	}

	for k, v := range first.crcArbiters {
		a, ok := second.crcArbiters[k]
		if !ok {
			return false
		}
		if !arbiterMemberEqual(v, a) {
			return false
		}
	}

	for k, v := range first.arbitersRoundReward {
		a, ok := second.arbitersRoundReward[k]
		if !ok {
			return false
		}
		if v != a {
			return false
		}
	}

	for k, _ := range first.illegalBlocksPayloadHashes {
		_, ok := second.illegalBlocksPayloadHashes[k]
		if !ok {
			return false
		}
	}

	return true
}

func checkResult(t *testing.T, A, B, C, D *CheckPoint) {
	assert.Equal(t, true, checkPointEqual(A, C))
	assert.Equal(t, false, checkPointEqual(A, B))
	assert.Equal(t, true, checkPointEqual(B, D))
	assert.Equal(t, false, checkPointEqual(B, C))
}

func getRegisterProducerTx(ownerPublicKey []byte, nodePublicKey []byte) *types.Transaction {
	return &types.Transaction{
		TxType: types.RegisterProducer,
		Payload: &payload.ProducerInfo{
			OwnerPublicKey: ownerPublicKey,
			NodePublicKey:  nodePublicKey,
		},
	}
}

func TestArbitrators_RollbackRegisterProducer(t *testing.T) {
	initArbiters()

	currentHeight := abt.chainParams.VoteStartHeight
	block1 := &types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			getRegisterProducerTx(abtList[0], abtList[0]),
			getRegisterProducerTx(abtList[1], abtList[1]),
			getRegisterProducerTx(abtList[2], abtList[2]),
			getRegisterProducerTx(abtList[3], abtList[3]),
		},
	}
	arbiterStateA := abt.Snapshot()
	assert.Equal(t, 0, len(abt.PendingProducers))

	// process
	abt.ProcessBlock(block1, nil)
	arbiterStateB := abt.Snapshot()
	assert.Equal(t, 4, len(abt.PendingProducers))

	// rollback
	currentHeight--
	err := abt.RollbackTo(currentHeight)
	assert.NoError(t, err)
	arbiterStateC := abt.Snapshot()

	// reprocess
	currentHeight++
	abt.ProcessBlock(block1, nil)
	arbiterStateD := abt.Snapshot()

	checkResult(t, arbiterStateA, arbiterStateB, arbiterStateC, arbiterStateD)

	for i := uint32(0); i < 4; i++ {
		currentHeight++
		blockEx := &types.Block{Header: types.Header{Height: currentHeight}}
		abt.ProcessBlock(blockEx, nil)
	}
	assert.Equal(t, 4, len(abt.PendingProducers))
	assert.Equal(t, 0, len(abt.ActivityProducers))
	arbiterStateA2 := abt.Snapshot()

	// process
	currentHeight++
	blockEx := &types.Block{Header: types.Header{Height: currentHeight}}
	abt.ProcessBlock(blockEx, nil)
	arbiterStateB2 := abt.Snapshot()
	assert.Equal(t, 0, len(abt.PendingProducers))
	assert.Equal(t, 4, len(abt.ActivityProducers))

	// rollback
	currentHeight--
	err = abt.RollbackTo(currentHeight)
	assert.NoError(t, err)
	arbiterStateC2 := abt.Snapshot()

	// reprocess
	currentHeight++
	abt.ProcessBlock(blockEx, nil)
	arbiterStateD2 := abt.Snapshot()

	checkResult(t, arbiterStateA2, arbiterStateB2, arbiterStateC2, arbiterStateD2)
}
