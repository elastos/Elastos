// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package checkpoint

import (
	"bytes"
	"errors"
	"io"
	"io/ioutil"
	"math/rand"
	"os"
	"path/filepath"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/utils/test"
	"github.com/stretchr/testify/assert"
)

const (
	checkpointSavePeriod      = uint32(3)
	checkpointEffectivePeriod = uint32(2)
	checkpointExtension       = ".pt"
)

type checkpoint struct {
	data     *uint64
	height   uint32
	priority Priority
}

func (c *checkpoint) StartHeight() uint32 {
	return 0
}

func (c *checkpoint) Priority() Priority {
	return c.priority
}

func (c *checkpoint) OnInit() {
}

func (c *checkpoint) Generator() func(buf []byte) ICheckPoint {
	return func(buf []byte) ICheckPoint {
		stream := new(bytes.Buffer)
		stream.Write(buf)

		result := &checkpoint{}
		if err := result.Deserialize(stream); err != nil {
			c.LogError(err)
			return nil
		}
		return result
	}
}

func (c *checkpoint) LogError(err error) {
	//fmt.Printf("check point error: %s", err.Error())
}

func (c *checkpoint) Serialize(w io.Writer) (err error) {
	if err = common.WriteUint32(w, c.height); err != nil {
		return
	}
	return common.WriteUint64(w, *c.data)
}

func (c *checkpoint) Deserialize(r io.Reader) (err error) {
	if c.height, err = common.ReadUint32(r); err != nil {
		return
	}

	var data uint64
	if data, err = common.ReadUint64(r); err != nil {
		return
	}
	c.data = &data
	return
}

func (*checkpoint) Key() string {
	return test.DataDir
}

func (c *checkpoint) Snapshot() ICheckPoint {
	data := *c.data // deep copy here
	return &checkpoint{
		data:   &data,
		height: c.height,
	}
}

func (c *checkpoint) GetHeight() uint32 {
	return c.height
}

func (c *checkpoint) SetHeight(height uint32) {
	c.height = height
}

func (c *checkpoint) SavePeriod() uint32 {
	return checkpointSavePeriod
}

func (c *checkpoint) EffectivePeriod() uint32 {
	return checkpointEffectivePeriod
}

func (*checkpoint) DataExtension() string {
	return checkpointExtension
}

func (c *checkpoint) OnBlockSaved(block *types.DposBlock) {
	data := uint64(block.Height)
	c.data = &data
}

func (c *checkpoint) OnRollbackTo(height uint32) error {
	if height < c.GetHeight()-6 {
		return errors.New("not supported")
	}
	data := uint64(height)
	c.data = &data
	return nil
}

func TestManager_SaveAndRestore(t *testing.T) {
	data := uint64(1)
	currentHeight := uint32(10)
	pt := &checkpoint{
		data:   &data,
		height: currentHeight,
	}

	manager := NewManager(&Config{
		EnableHistory: false,
		NeedSave:      true,
	})
	manager.Register(pt)

	// save nothing
	manager.onBlockSaved(&types.DposBlock{
		Block: &types.Block{
			Header: types.Header{Height: currentHeight},
		},
	}, nil, false)

	// save current height
	currentHeight += pt.SavePeriod()
	manager.onBlockSaved(&types.DposBlock{
		Block: &types.Block{
			Header: types.Header{Height: currentHeight},
		},
	}, nil, false)

	// replace to default.pt
	currentHeight += pt.EffectivePeriod()
	manager.onBlockSaved(&types.DposBlock{
		Block: &types.Block{
			Header: types.Header{Height: currentHeight},
		},
	}, nil, false)

	// new a checkpoint manager and restore
	manager2 := NewManager(&Config{
		EnableHistory: false,
	})
	manager2.Register(&checkpoint{})
	assert.NoError(t, manager2.Restore())
	assert.Equal(t, currentHeight-pt.EffectivePeriod(),
		manager2.checkpoints[test.DataDir].GetHeight())

	cleanCheckpoints()
}

func TestManager_GetCheckpoint_DisableHistory(t *testing.T) {
	data := uint64(1)
	currentHeight := uint32(10)
	pt := &checkpoint{
		data:   &data,
		height: currentHeight,
	}

	manager := NewManager(&Config{
		EnableHistory: false,
	})
	manager.Register(pt)

	result, ok := manager.GetCheckpoint(pt.Key(), currentHeight-1)
	assert.Equal(t, nil, result)
	assert.Equal(t, false, ok)

	result, ok = manager.GetCheckpoint(pt.Key(), currentHeight)
	assert.NotEqual(t, nil, result)
	assert.Equal(t, true, ok)

	result, ok = manager.GetCheckpoint(pt.Key(), currentHeight+1)
	assert.NotEqual(t, nil, result)
	assert.Equal(t, true, ok)

	manager.Unregister(pt.Key())
}

func TestManager_GetCheckpoint_EnableHistory(t *testing.T) {
	data := uint64(1)
	currentHeight := uint32(10)
	pt := &checkpoint{
		data:   &data,
		height: 0,
	}

	manager := NewManager(&Config{
		EnableHistory:      true,
		HistoryStartHeight: currentHeight,
		NeedSave:           true,
	})
	manager.Register(pt)

	manager.OnBlockSaved(&types.DposBlock{
		Block: &types.Block{
			Header: types.Header{Height: currentHeight},
		},
	}, nil)
	currentHeight += pt.SavePeriod()

	manager.OnBlockSaved(&types.DposBlock{
		Block: &types.Block{
			Header: types.Header{Height: currentHeight},
		},
	}, nil)

	result, ok := manager.GetCheckpoint(pt.Key(),
		currentHeight-pt.SavePeriod()-1)
	assert.Equal(t, nil, result)
	assert.Equal(t, false, ok)

	// less than current height will get history checkpoint
	result, ok = manager.GetCheckpoint(pt.Key(), currentHeight-1)
	assert.NotEqual(t, nil, result)
	assert.Equal(t, true, ok)

	result, ok = manager.GetCheckpoint(pt.Key(), currentHeight)
	assert.NotEqual(t, nil, result)
	assert.Equal(t, true, ok)

	result, ok = manager.GetCheckpoint(pt.Key(), currentHeight)
	assert.NotEqual(t, nil, result)
	assert.Equal(t, true, ok)

	cleanCheckpoints()
}

func TestManager_OnRollbackTo(t *testing.T) {
	data := uint64(1)
	currentHeight := uint32(10)
	pt := &checkpoint{
		data:   &data,
		height: currentHeight,
	}

	manager := NewManager(&Config{
		EnableHistory:      true,
		HistoryStartHeight: currentHeight,
	})
	manager.Register(pt)

	originalHeight := currentHeight
	currentHeight += pt.SavePeriod()
	manager.onBlockSaved(&types.DposBlock{
		Block: &types.Block{
			Header: types.Header{Height: currentHeight},
		},
	}, nil, false)

	assert.Equal(t, currentHeight, uint32(*pt.data))
	assert.NoError(t, manager.OnRollbackTo(originalHeight))
	assert.Equal(t, originalHeight, uint32(*pt.data))

	cleanCheckpoints()
}

func TestManager_getOrderedCheckpoints(t *testing.T) {
	manager := NewManager(&Config{})

	for i := 0; i < 10; i++ {
		manager.checkpoints[randomString()] =
			&checkpoint{priority: Priority(rand.Uint32())}
	}

	orderedPoints := manager.getOrderedCheckpoints()
	for i := range orderedPoints {
		if i == 0 {
			continue
		}
		assert.True(t, orderedPoints[i-1].Priority() <=
			orderedPoints[i].Priority())
	}
}

func cleanCheckpoints() {
	var err error
	var files []os.FileInfo
	if files, err = ioutil.ReadDir(test.DataDir); err != nil {
		return
	}
	for _, f := range files {
		os.Remove(filepath.Join(test.DataDir, f.Name()))
	}
}

func randomString() string {
	a := make([]byte, 20)
	rand.Read(a)
	return common.BytesToHexString(a)
}
