package store

import "time"

type ConditionalAction func() error

type EmergencyConfig struct {
	EmergencyTriggerTimeSpan uint32
	EmergencyDuration        uint32
}

type emergencyMechanism struct {
	cfg EmergencyConfig

	lastConfirmedBlockTimeStamp uint32

	emergencyStarted   bool
	emergencyStartTime uint32
}

func (e *emergencyMechanism) IsRunning() bool {
	return e.emergencyStarted
}

func (e *emergencyMechanism) ResetBlockTime(blockTime uint32) {
	e.lastConfirmedBlockTimeStamp = blockTime
}

func (e *emergencyMechanism) TryEnterEmergency(blockTime uint32) bool {
	if !e.emergencyStarted && time.Now().After(time.Unix(int64(e.lastConfirmedBlockTimeStamp)+int64(e.cfg.EmergencyTriggerTimeSpan), 0)) {
		e.emergencyStarted = true
		e.emergencyStartTime = blockTime
		return true
	}

	return false
}

func (e *emergencyMechanism) TryLeaveEmergency() bool {
	if e.emergencyStarted && time.Now().After(time.Unix(int64(e.emergencyStartTime)+int64(e.cfg.EmergencyDuration), 0)) {
		e.emergencyStarted = false
		e.emergencyStartTime = 0
		return true
	}
	return false
}
