// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package state

import (
	"bytes"
	"github.com/elastos/Elastos.ELA/crypto"
	"testing"

	"github.com/elastos/Elastos.ELA/common/config"

	"github.com/stretchr/testify/assert"
)

func TestArbitrators_GetSnapshot(t *testing.T) {
	var bestHeight uint32

	arbitrators, _ := NewArbitrators(&config.DefaultParams,
		nil, nil)
	arbitrators.RegisterFunction(func() uint32 { return bestHeight },
		nil, nil)

	// define three height versions:
	// firstSnapshotHeight < secondSnapshotHeight < bestHeight
	bestHeight = 30
	firstSnapshotHeight := uint32(10)
	firstSnapshotPk := randomFakePK()
	secondSnapshotHeight := uint32(20)
	secondSnapshotPk := randomFakePK()
	ar, _ := NewOriginArbiter(Origin, firstSnapshotPk)
	arbitrators.currentArbitrators = []ArbiterMember{ar}

	// take the first snapshot
	arbitrators.snapshot(firstSnapshotHeight)
	ar, _ = NewOriginArbiter(Origin, secondSnapshotPk)
	arbitrators.currentArbitrators = []ArbiterMember{ar}

	// height1
	frames := arbitrators.GetSnapshot(firstSnapshotHeight)
	assert.Equal(t, 1, len(frames))
	assert.True(t, bytes.Equal(firstSnapshotPk,
		frames[0].CurrentArbitrators[0].GetNodePublicKey()))

	// < height1
	frames = arbitrators.GetSnapshot(firstSnapshotHeight - 1)
	assert.Equal(t, []*CheckPoint{}, frames)

	// > height1
	frames = arbitrators.GetSnapshot(firstSnapshotHeight + 1)
	assert.Equal(t, 1, len(frames))
	assert.True(t, bytes.Equal(firstSnapshotPk,
		frames[0].CurrentArbitrators[0].GetNodePublicKey()))

	// height2
	frames = arbitrators.GetSnapshot(secondSnapshotHeight)
	assert.Equal(t, 1, len(frames))
	assert.True(t, bytes.Equal(firstSnapshotPk,
		frames[0].CurrentArbitrators[0].GetNodePublicKey()))

	// bestHeight
	frames = arbitrators.GetSnapshot(bestHeight)
	assert.Equal(t, 1, len(frames))
	assert.True(t, bytes.Equal(firstSnapshotPk,
		frames[0].CurrentArbitrators[0].GetNodePublicKey()))

	// bestHeight+1
	frames = arbitrators.GetSnapshot(bestHeight + 1)
	assert.Equal(t, 1, len(frames))
	assert.True(t, bytes.Equal(secondSnapshotPk,
		frames[0].CurrentArbitrators[0].GetNodePublicKey()))

	// > bestHeight
	frames = arbitrators.GetSnapshot(bestHeight + 1)
	assert.Equal(t, 1, len(frames))
	assert.True(t, bytes.Equal(secondSnapshotPk,
		frames[0].CurrentArbitrators[0].GetNodePublicKey()))

	// take the second snapshot
	arbitrators.snapshot(secondSnapshotHeight)
	ar, _ = NewOriginArbiter(Origin, randomFakePK())
	arbitrators.currentArbitrators = []ArbiterMember{ar}

	// height1
	frames = arbitrators.GetSnapshot(firstSnapshotHeight)
	assert.Equal(t, 1, len(frames))
	assert.True(t, bytes.Equal(firstSnapshotPk,
		frames[0].CurrentArbitrators[0].GetNodePublicKey()))

	// < height1
	frames = arbitrators.GetSnapshot(firstSnapshotHeight - 1)
	assert.Equal(t, []*CheckPoint{}, frames)

	// > height1
	frames = arbitrators.GetSnapshot(firstSnapshotHeight + 1)
	assert.Equal(t, 1, len(frames))
	assert.True(t, bytes.Equal(firstSnapshotPk,
		frames[0].CurrentArbitrators[0].GetNodePublicKey()))

	// height2
	frames = arbitrators.GetSnapshot(secondSnapshotHeight)
	assert.Equal(t, 1, len(frames))
	assert.True(t, bytes.Equal(secondSnapshotPk,
		frames[0].CurrentArbitrators[0].GetNodePublicKey()))

	// > height2
	frames = arbitrators.GetSnapshot(secondSnapshotHeight + 1)
	assert.Equal(t, 1, len(frames))
	assert.True(t, bytes.Equal(secondSnapshotPk,
		frames[0].CurrentArbitrators[0].GetNodePublicKey()))

	// bestHeight
	frames = arbitrators.GetSnapshot(bestHeight)
	assert.Equal(t, 1, len(frames))
	assert.True(t, bytes.Equal(secondSnapshotPk,
		frames[0].CurrentArbitrators[0].GetNodePublicKey()))

	// > bestHeight
	frames = arbitrators.GetSnapshot(bestHeight + 1)
	assert.Equal(t, 1, len(frames))
	assert.True(t, bytes.Equal(arbitrators.currentArbitrators[0].
		GetNodePublicKey(), frames[0].CurrentArbitrators[0].GetNodePublicKey()))

	// take snapshot more than MaxSnapshotLength
	loopSnapshotHeight := bestHeight
	bestHeight += 50
	for i := loopSnapshotHeight; i < loopSnapshotHeight+MaxSnapshotLength; i++ {
		arbitrators.snapshot(i)
	}
	assert.Equal(t, MaxSnapshotLength, len(arbitrators.snapshots))
	assert.Equal(t, MaxSnapshotLength, len(arbitrators.snapshotKeysDesc))
	_, exist := arbitrators.snapshots[firstSnapshotHeight]
	assert.False(t, exist)
	_, exist = arbitrators.snapshots[secondSnapshotHeight]
	assert.False(t, exist)
}

func randomFakePK() []byte {
	_, pub, _ := crypto.GenerateKeyPair()
	result, _ := pub.EncodePoint(true)
	return result
}
