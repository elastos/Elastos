package store

import (
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

type ConditionalAction func() error

type EmergencyConfig struct {
	EmergencyDuration uint32
}

type EmergencyData struct {
	EmergencyStarted     bool
	EmergencyStartHeight uint32
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

	if err := common.WriteUint32(w, d.EmergencyStartHeight); err != nil {
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

	if d.EmergencyStartHeight, err = common.ReadUint32(r); err != nil {
		return err
	}

	return err
}

func (e *emergencyMechanism) IsRunning() bool {
	return e.data.EmergencyStarted
}

func (e *emergencyMechanism) GetEmergencyData() *EmergencyData {
	return &e.data
}

func (e *emergencyMechanism) TryEnterEmergency(blockHeight uint32) bool {
	if !e.data.EmergencyStarted {
		e.data.EmergencyStarted = true
		e.data.EmergencyStartHeight = blockHeight
		return true
	}

	return false
}

func (e *emergencyMechanism) TryLeaveEmergency(blockHeight uint32) bool {
	if e.data.EmergencyStarted &&
		blockHeight > e.data.EmergencyStartHeight+e.cfg.EmergencyDuration {
		e.data.EmergencyStarted = false
		e.data.EmergencyStartHeight = 0
		return true
	}
	return false
}
