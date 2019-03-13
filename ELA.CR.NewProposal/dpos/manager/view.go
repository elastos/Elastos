package manager

import (
	"time"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/dpos/log"
	"github.com/elastos/Elastos.ELA/dpos/state"
)

type ViewListener interface {
	OnViewChanged(isOnDuty bool)
}

type view struct {
	signTolerance time.Duration
	viewStartTime time.Time
	isDposOnDuty  bool
	arbitrators   state.Arbitrators

	listener ViewListener
}

func (v *view) IsOnDuty() bool {
	return v.isDposOnDuty
}

func (v *view) SetOnDuty(onDuty bool) {
	v.isDposOnDuty = onDuty
}

func (v *view) GetViewStartTime() time.Time {
	return v.viewStartTime
}

func (v *view) ResetView(t time.Time) {
	v.viewStartTime = t
}

func (v *view) ChangeView(viewOffset *uint32) {
	offset, offsetTime := v.CalculateOffsetTime(v.viewStartTime)
	*viewOffset += uint32(offset)
	v.viewStartTime = time.Now().Add(-offsetTime)
	log.Info("[ChangeView] current view offset:", *viewOffset)

	if offset > 0 {
		currentArbiter := v.arbitrators.GetNextOnDutyArbitrator(*viewOffset)

		v.isDposOnDuty = common.BytesToHexString(currentArbiter) == config.Parameters.ArbiterConfiguration.PublicKey
		log.Info("current onduty arbiter:", currentArbiter)

		v.listener.OnViewChanged(v.isDposOnDuty)
	}
}

func (v *view) CalculateOffsetTime(startTime time.Time) (uint32, time.Duration) {
	duration := time.Now().Sub(startTime)
	offset := duration / v.signTolerance
	offsetTime := duration % v.signTolerance

	return uint32(offset), offsetTime
}

func (v *view) TryChangeView(viewOffset *uint32) bool {

	now := time.Now()
	if now.After(v.viewStartTime.Add(v.signTolerance)) {
		log.Info("[TryChangeView] succeed")
		v.ChangeView(viewOffset)
		return true
	}
	return false
}

func (v *view) GetViewInterval() time.Duration {
	return v.signTolerance
}
