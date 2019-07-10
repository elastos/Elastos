package checkpoint

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
)

// ICheckPoint is a interface defines operators that all memory state should
// implement to restore and get when needed.
type ICheckPoint interface {
	common.Serializable

	// Key is the unique id in manager to identity checkpoints.
	Key() string

	// Snapshot take a snapshot of current state, this should be a deep copy
	// because we are in a async environment.
	Snapshot() ICheckPoint

	// Height defines current height of checkpoint by which determines how to
	// choose between multi-history checkpoints.
	Height() uint32

	// Period defines who long should we save the checkpoint.
	Period() uint32

	// DataPath defines relative path the checkpoint related data should save
	// to.
	DataPath() string
}

// BlockListener defines events during block lifetime.
type BlockListener interface {
	// OnBlockSaved is an event fired after block saved to chain db,
	// which means block has been settled in block chain.
	OnBlockSaved(block *types.Block, confirm *payload.Confirm)

	// OnRollbackTo is an event fired during the block chain rollback,
	// since we only tolerance 6 blocks rollback so out max rollback support
	// can be 6 blocks by default.
	OnRollbackTo(height uint32)
}

// Config holds checkpoint related configurations.
type Config struct {
	// EnableHistory is a switch about recording history of snapshots of
	// checkpoints.
	EnableHistory bool

	// HistoryStartHeight defines the height manager should start to record
	// snapshots of checkpoints.
	HistoryStartHeight uint32
}

// Manager holds checkpoints save automatically.
type Manager struct {
	checkpoints map[string]ICheckPoint
	cfg         *Config
}

// OnBlockSaved is an event fired after block saved to chain db,
// which means block has been settled in block chain.
func (m *Manager) OnBlockSaved(block *types.Block, confirm *payload.Confirm) {

}

// OnRollbackTo is an event fired during the block chain rollback, since we
// only tolerance 6 blocks rollback so out max rollback support can be 6 blocks
// by default.
func (m *Manager) OnRollbackTo(height uint32) {

}

// Register will register a checkpoint with key in checkpoint as the unique id.
func (m *Manager) Register(checkpoint ICheckPoint) {
	m.checkpoints[checkpoint.Key()] = checkpoint
}

// Unregister will unregister a checkpoint with key in checkpoint as the
// unique id.
func (m *Manager) Unregister(key string) {
	delete(m.checkpoints, key)
}

// GetCheckpoint get a checkpoint by key and height. If height is lower than
// last height of checkpoints and EnableHistory in Config struct is true
// should return supported checkpoints, otherwise will return nil instead.
func (m *Manager) GetCheckpoint(key string, height uint32) (
	checkpoint ICheckPoint, found bool) {
	return
}
