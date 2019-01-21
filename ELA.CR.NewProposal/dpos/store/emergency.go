package store

import (
	"github.com/elastos/Elastos.ELA/common"
	"io"
	"time"
)

type ConditionalAction func() error

type EmergencyConfig struct {
	EmergencyTriggerTimeSpan uint32
	EmergencyDuration        uint32
}

type EmergencyData struct {
	EmergencyStarted   bool
	EmergencyStartTime uint32

	LastConfirmedBlockTimeStamp uint32
}

type emergencyMechanism struct {
	cfg  EmergencyConfig
	data EmergencyData
}

func (d *EmergencyData) Serialize(w io.Writer) error {
	startedValue := uint8(0)
	if d.EmergencyStarted {
		startedValue = 1
	}
	if err := common.WriteUint8(w, startedValue); err != nil {
		return err
	}

	if err := common.WriteUint32(w, d.EmergencyStartTime); err != nil {
		return err
	}

	if err := common.WriteUint32(w, d.LastConfirmedBlockTimeStamp); err != nil {
		return err
	}
	return nil
}

func (d *EmergencyData) Deserialize(r io.Reader) (err error) {
	var startedValue uint8
	if startedValue, err = common.ReadUint8(r); err != nil {
		return err
	}
	d.EmergencyStarted = startedValue == 1

	if d.EmergencyStartTime, err = common.ReadUint32(r); err != nil {
		return err
	}

	d.LastConfirmedBlockTimeStamp, err = common.ReadUint32(r)
	return err
}

func (e *emergencyMechanism) IsRunning() bool {
	return e.data.EmergencyStarted
}

func (e *emergencyMechanism) ResetBlockTime(blockTime uint32) {
	e.data.LastConfirmedBlockTimeStamp = blockTime
}

func (e *emergencyMechanism) GetEmergencyData() *EmergencyData {
	return &e.data
}

func (e *emergencyMechanism) TryEnterEmergency(blockTime uint32) bool {
	if !e.data.EmergencyStarted && time.Now().After(time.Unix(int64(e.data.LastConfirmedBlockTimeStamp)+int64(e.cfg.EmergencyTriggerTimeSpan), 0)) {
		e.data.EmergencyStarted = true
		e.data.EmergencyStartTime = blockTime
		return true
	}

	return false
}

func (e *emergencyMechanism) TryLeaveEmergency() bool {
	if e.data.EmergencyStarted && time.Now().After(time.Unix(int64(e.data.EmergencyStartTime)+int64(e.cfg.EmergencyDuration), 0)) {
		e.data.EmergencyStarted = false
		e.data.EmergencyStartTime = 0
		return true
	}
	return false
}
