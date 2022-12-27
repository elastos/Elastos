package state

import (
	"bytes"
	"crypto/rand"
	"testing"

	"github.com/elastos/Elastos.ELA/common/config"

	"github.com/stretchr/testify/assert"
)

func TestArbitrators_GetSnapshot(t *testing.T) {
	var bestHeight uint32
	arbitrators, _ := NewArbitrators(&config.DefaultParams, nil,
		func() uint32 { return bestHeight }, nil, nil)

	// define three height versions:
	// firstSnapshotHeight < secondSnapshotHeight < bestHeight
	bestHeight = 30
	firstSnapshotHeight := uint32(10)
	firstSnapshotPk := randomFakePK()
	secondSnapshotHeight := uint32(20)
	secondSnapshotPk := randomFakePK()
	arbitrators.CurrentArbitrators = [][]byte{firstSnapshotPk}

	// take the first snapshot
	arbitrators.snapshot(firstSnapshotHeight)
	arbitrators.CurrentArbitrators = [][]byte{secondSnapshotPk}

	// height1
	frames := arbitrators.GetSnapshot(firstSnapshotHeight)
	assert.Equal(t, 1, len(frames))
	assert.True(t, bytes.Equal(firstSnapshotPk, frames[0].CurrentArbitrators[0]))

	// < height1
	frames = arbitrators.GetSnapshot(firstSnapshotHeight - 1)
	assert.Equal(t, []*KeyFrame(nil), frames)

	// > height1
	frames = arbitrators.GetSnapshot(firstSnapshotHeight + 1)
	assert.Equal(t, 1, len(frames))
	assert.True(t, bytes.Equal(firstSnapshotPk, frames[0].CurrentArbitrators[0]))

	// height2
	frames = arbitrators.GetSnapshot(secondSnapshotHeight)
	assert.Equal(t, 1, len(frames))
	assert.True(t, bytes.Equal(firstSnapshotPk, frames[0].CurrentArbitrators[0]))

	// bestHeight
	frames = arbitrators.GetSnapshot(bestHeight)
	assert.Equal(t, 1, len(frames))
	assert.True(t, bytes.Equal(firstSnapshotPk, frames[0].CurrentArbitrators[0]))

	// bestHeight+1
	frames = arbitrators.GetSnapshot(bestHeight + 1)
	assert.Equal(t, 1, len(frames))
	assert.True(t, bytes.Equal(secondSnapshotPk, frames[0].CurrentArbitrators[0]))

	// > bestHeight
	frames = arbitrators.GetSnapshot(bestHeight + 1)
	assert.Equal(t, 1, len(frames))
	assert.True(t, bytes.Equal(secondSnapshotPk, frames[0].CurrentArbitrators[0]))

	// take the second snapshot
	arbitrators.snapshot(secondSnapshotHeight)
	arbitrators.CurrentArbitrators = [][]byte{randomFakePK()}

	// height1
	frames = arbitrators.GetSnapshot(firstSnapshotHeight)
	assert.Equal(t, 1, len(frames))
	assert.True(t, bytes.Equal(firstSnapshotPk, frames[0].CurrentArbitrators[0]))

	// < height1
	frames = arbitrators.GetSnapshot(firstSnapshotHeight - 1)
	assert.Equal(t, []*KeyFrame(nil), frames)

	// > height1
	frames = arbitrators.GetSnapshot(firstSnapshotHeight + 1)
	assert.Equal(t, 1, len(frames))
	assert.True(t, bytes.Equal(firstSnapshotPk, frames[0].CurrentArbitrators[0]))

	// height2
	frames = arbitrators.GetSnapshot(secondSnapshotHeight)
	assert.Equal(t, 1, len(frames))
	assert.True(t, bytes.Equal(secondSnapshotPk, frames[0].CurrentArbitrators[0]))

	// > height2
	frames = arbitrators.GetSnapshot(secondSnapshotHeight + 1)
	assert.Equal(t, 1, len(frames))
	assert.True(t, bytes.Equal(secondSnapshotPk, frames[0].CurrentArbitrators[0]))

	// bestHeight
	frames = arbitrators.GetSnapshot(bestHeight)
	assert.Equal(t, 1, len(frames))
	assert.True(t, bytes.Equal(secondSnapshotPk,
		frames[0].CurrentArbitrators[0]))

	// > bestHeight
	frames = arbitrators.GetSnapshot(bestHeight + 1)
	assert.Equal(t, 1, len(frames))
	assert.True(t, bytes.Equal(arbitrators.KeyFrame.CurrentArbitrators[0],
		frames[0].CurrentArbitrators[0]))

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
	pk := make([]byte, 33)
	rand.Read(pk)
	return pk
}
