package checkpoint

import (
	"bytes"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/utils"
	"io/ioutil"
	"os"
	"path/filepath"
	"strconv"
	"sync"
)

const DefaultCheckpoint = "default"

// BlockListener defines events during block lifetime.
type BlockListener interface {
	// OnBlockSaved is an event fired after block saved to chain db,
	// which means block has been settled in block chain.
	OnBlockSaved(block *types.DposBlock)

	// OnRollbackTo is an event fired during the block chain rollback,
	// since we only tolerance 6 blocks rollback so out max rollback support
	// can be 6 blocks by default.
	OnRollbackTo(height uint32) error
}

// ICheckPoint is a interface defines operators that all memory state should
// implement to restore and get when needed.
type ICheckPoint interface {
	common.Serializable
	BlockListener

	// Key is the unique id in manager to identity checkpoints.
	Key() string

	// Snapshot take a snapshot of current state, this should be a deep copy
	// because we are in a async environment.
	Snapshot() ICheckPoint

	// GetHeight returns current height of checkpoint by which determines how to
	// choose between multi-history checkpoints.
	GetHeight() uint32

	// SetHeight set current height of checkpoint by which determines how to
	// choose between multi-history checkpoints.
	SetHeight(uint32)

	// SavePeriod defines who long should we save the checkpoint.
	SavePeriod() uint32

	EffectivePeriod() uint32

	// DataExtension defines extension of checkpoint related data files.
	DataExtension() string

	Generator() func(buf []byte) ICheckPoint

	LogError(err error)
}

// Config holds checkpoint related configurations.
type Config struct {
	// EnableHistory is a switch about recording history of snapshots of
	// checkpoints.
	EnableHistory bool

	// HistoryStartHeight defines the height manager should start to record
	// snapshots of checkpoints.
	HistoryStartHeight uint32

	// DataPath defines root directory path of all checkpoint related files.
	DataPath string
}

// Manager holds checkpoints save automatically.
type Manager struct {
	checkpoints map[string]ICheckPoint
	cfg         *Config
	mtx         sync.RWMutex
}

// OnBlockSaved is an event fired after block saved to chain db,
// which means block has been settled in block chain.
func (m *Manager) OnBlockSaved(block *types.DposBlock,
	filter func(point ICheckPoint) bool) {
	m.mtx.Lock()
	defer m.mtx.Unlock()

	for _, v := range m.checkpoints {
		if filter != nil && filter(v) {
			continue
		}
		v.OnBlockSaved(block)

		if block.Height >= v.GetHeight()+v.SavePeriod() {
			snapshot := v.Snapshot()
			v.SetHeight(block.Height)
			// save checkpoint to corresponding file asynchronously.
			go func() {
				if err := m.saveCheckpoint(snapshot); err != nil {
					snapshot.LogError(err)
				}
			}()
		} else if block.Height >= v.GetHeight()+v.EffectivePeriod() {

		}
	}
}

// OnRollbackTo is an event fired during the block chain rollback, since we
// only tolerance 6 blocks rollback so out max rollback support can be 6 blocks
// by default.
func (m *Manager) OnRollbackTo(height uint32) error {
	return nil
}

// Register will register a checkpoint with key in checkpoint as the unique id.
func (m *Manager) Register(checkpoint ICheckPoint) {
	m.mtx.Lock()
	defer m.mtx.Unlock()
	m.checkpoints[checkpoint.Key()] = checkpoint
}

// Unregister will unregister a checkpoint with key in checkpoint as the
// unique id.
func (m *Manager) Unregister(key string) {
	m.mtx.Lock()
	defer m.mtx.Unlock()
	delete(m.checkpoints, key)
}

// GetCheckpoint get a checkpoint by key and height. If height is lower than
// last height of checkpoints and EnableHistory in Config struct is true
// should return supported checkpoints, otherwise will return nil instead.
func (m *Manager) GetCheckpoint(key string, height uint32) (
	checkpoint ICheckPoint, found bool) {
	m.mtx.RLock()
	defer m.mtx.RUnlock()

	checkpoint, found = m.checkpoints[key]
	if height >= checkpoint.GetHeight() {
		return
	}

	if !found {
		return
	}

	if m.cfg.EnableHistory {
		return m.findHistoryCheckpoint(checkpoint, height)
	} else {
		return nil, false
	}
}

func (m *Manager) saveCheckpoint(checkpoint ICheckPoint) (err error) {
	dir := m.getCheckpointDirectory(checkpoint)
	if !utils.FileExisted(dir) {
		if err = os.Mkdir(dir, 0700); err != nil {
			return
		}
	}

	filename := m.getFileFullPath(checkpoint)
	var file *os.File
	file, err = os.OpenFile(filename,
		os.O_CREATE|os.O_WRONLY|os.O_TRUNC, 0600)
	if err != nil {
		return
	}

	buf := new(bytes.Buffer)
	if err = checkpoint.Serialize(buf); err != nil {
		return
	}

	if _, err = file.Write(buf.Bytes()); err != nil {
		return
	}

	if !m.cfg.EnableHistory {
		// remove files if successfully saved
		reserveName := m.getFileName(checkpoint, checkpoint.GetHeight())
		var files []os.FileInfo
		files, err = ioutil.ReadDir(dir)
		if err != nil {
			return err
		}

		for _, f := range files {
			if f.Name() == reserveName {
				continue
			}
			if e := os.Remove(filepath.Join(dir, f.Name())); e != nil {
				checkpoint.LogError(e)
			}
		}
	}
	return nil
}

func (m *Manager) replaceCheckpoint(checkpoint ICheckPoint) (err error) {
	return nil
}

func (m *Manager) findHistoryCheckpoint(current ICheckPoint,
	findHeight uint32) (checkpoint ICheckPoint, found bool) {
	bestHeight := current.GetHeight()
	for bestHeight > findHeight && bestHeight >= current.SavePeriod() {
		bestHeight -= current.SavePeriod()
	}
	// bestHeight still larger than findHeight means findHeight less than
	// current.SavePeriod(), then set to zero directly
	if bestHeight > findHeight {
		bestHeight = 0
	}

	path := m.getFileFullPathByHeight(current, bestHeight)
	if !utils.FileExisted(path) {
		return nil, false
	}
	file, err := os.OpenFile(path, os.O_RDONLY, 0400)
	if err != nil || file == nil {
		current.LogError(err)
		return nil, false
	}
	data, err := ioutil.ReadAll(file)
	if err != nil {
		current.LogError(err)
		return nil, false
	}
	return current.Generator()(data), true
}

func (m *Manager) getFileFullPath(checkpoint ICheckPoint) string {
	return m.getFileFullPathByHeight(checkpoint, checkpoint.GetHeight())
}

func (m *Manager) getFileFullPathByHeight(checkpoint ICheckPoint,
	height uint32) string {
	return filepath.Join(m.getCheckpointDirectory(checkpoint),
		string(os.PathSeparator), m.getFileName(checkpoint, height))
}

func (m *Manager) getFileName(checkpoint ICheckPoint, height uint32) string {
	return strconv.FormatUint(uint64(height), 10) +
		checkpoint.DataExtension()
}

func (m *Manager) getDefaultFileName(checkpoint ICheckPoint) string {
	return DefaultCheckpoint + checkpoint.DataExtension()
}

func (m *Manager) getCheckpointDirectory(checkpoint ICheckPoint) string {
	return filepath.Join(m.cfg.DataPath, checkpoint.Key())
}

func NewManager(cfg *Config) *Manager {
	m := &Manager{
		checkpoints: make(map[string]ICheckPoint),
		cfg:         cfg,
	}
	return m
}
