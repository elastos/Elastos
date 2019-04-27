package state

import (
	"sync"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types/payload"
)

type degradationState byte

const (
	Normal       degradationState = 0x00
	Understaffed degradationState = 0x01
	Inactive     degradationState = 0x02
)

// degradation maintains states which will take effect during
// degradation period.
type degradation struct {
	mtx sync.Mutex

	state             degradationState
	understaffedSince uint32
	inactivateHeight  uint32
	inactiveTxs       map[common.Uint256]interface{}
}

func (d *degradation) IsUnderstaffedMode() bool {
	d.mtx.Lock()
	result := d.state == Understaffed
	d.mtx.Unlock()

	return result
}

func (d *degradation) IsInactiveMode() bool {
	d.mtx.Lock()
	result := d.state == Inactive
	d.mtx.Unlock()

	return result
}

func (d *degradation) RollbackTo(height uint32) {
	d.mtx.Lock()
	// if rollback to the height before abnormal mode was set,
	// then reset inactive related state
	needReset := height < d.inactivateHeight || height < d.understaffedSince
	d.mtx.Unlock()

	if needReset {
		d.Reset()
	}
}

func (d *degradation) InactiveModeSwitch(height uint32,
	isAbleToRecover func() bool) (bool, bool) {

	d.mtx.Lock()
	if d.state != Normal {
		d.mtx.Unlock()
		return false, false
	}
	if len(d.inactiveTxs) >= MaxNormalInactiveChangesCount {
		d.state = Inactive
		d.inactivateHeight = height
		d.mtx.Unlock()

		return true, false
	}
	d.mtx.Unlock()

	if d.IsInactiveMode() && isAbleToRecover() {
		d.Reset()

		return false, true
	}

	return false, false
}

func (d *degradation) TrySetUnderstaffed(height uint32) bool {
	d.mtx.Lock()
	if d.state != Normal {
		d.mtx.Unlock()
		return false
	}
	d.understaffedSince = height
	d.state = Understaffed
	d.mtx.Unlock()
	return true
}

func (d *degradation) TryLeaveUnderStaffed(isAbleToRecover func() bool) bool {
	if isAbleToRecover() {
		d.Reset()
		return true
	}
	return false
}

// Reset method reset all 
func (d *degradation) Reset() {
	d.mtx.Lock()
	d.state = Normal
	d.inactivateHeight = 0
	d.understaffedSince = 0
	d.mtx.Unlock()
}

func (d *degradation) AddInactivePayload(p *payload.InactiveArbitrators) bool {
	hash := p.Hash()
	d.mtx.Lock()
	_, exist := d.inactiveTxs[hash]
	if !exist {
		d.inactiveTxs[hash] = nil
	}
	d.mtx.Unlock()

	return !exist
}
