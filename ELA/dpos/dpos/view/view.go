package view

import (
	"github.com/elastos/Elastos.ELA/dpos/config"
	"time"

	. "github.com/elastos/Elastos.ELA/dpos/dpos/arbitrator"
	"github.com/elastos/Elastos.ELA/dpos/log"
)

type ViewListener interface {
	OnViewChanged(isOnDuty bool)
}

type View struct {
	signTolerance time.Duration
	viewStartTime time.Time
	isDposOnDuty  bool

	listener ViewListener
}

func (v *View) Initialize(tolerance time.Duration, listener ViewListener) {
	v.signTolerance = tolerance
	v.listener = listener
}

func (v *View) IsOnDuty() bool {
	return v.isDposOnDuty
}

func (v *View) SetOnDuty(onDuty bool) {
	v.isDposOnDuty = onDuty
}

func (v *View) GetViewStartTime() time.Time {
	return v.viewStartTime
}

func (v *View) ResetView(t time.Time) {
	v.viewStartTime = t
}

func (v *View) ChangeView(viewOffset *uint32) {
	offset, offsetTime := v.CalculateOffsetTime(v.viewStartTime)
	*viewOffset += uint32(offset)
	v.viewStartTime = time.Now().Add(-offsetTime)
	log.Info("[ChangeView] current view offset:", viewOffset)

	if offset > 0 {
		currentArbiter := GetCurrentArbitrator(*viewOffset)
		v.isDposOnDuty = currentArbiter == config.Parameters.Name
		log.Info("current onduty arbiter:", currentArbiter)

		v.listener.OnViewChanged(v.isDposOnDuty)
	}
}

func (v *View) CalculateOffsetTime(startTime time.Time) (uint32, time.Duration) {
	duration := time.Now().Sub(startTime)
	offset := duration / v.signTolerance
	offsetTime := duration % v.signTolerance

	return uint32(offset), offsetTime
}

func (v *View) TryChangeView(viewOffset *uint32) bool {
	if time.Now().After(v.viewStartTime.Add(v.signTolerance)) {
		log.Info("[TryChangeView] succeed")
		v.ChangeView(viewOffset)
		return true
	}
	return false
}

func (v *View) GetViewInterval() time.Duration {
	return v.signTolerance
}
