package log

import (
	"testing"
	"time"
)

func TestLogger(t *testing.T) {
	logger := NewLogger(0, 5, 20)

	run := make(chan struct{})
	go func() {
		for {
			run <- struct{}{}
		}
	}()

	timer := time.NewTimer(time.Second * 10)
out:
	for {
		select {
		case <-run:
			logger.Info("Print info log")
			logger.Infof("Print info log formatted")
			logger.Trace("Print trace log")
			logger.Tracef("Print trace log formatted")
			logger.Warn("Print warn log")
			logger.Warnf("Print warn log formatted")
			logger.Error("Print error log")
			logger.Errorf("Print error log formatted")
			logger.Debug("Print debug log")
			logger.Debugf("Print debug log formatted")

		case <-timer.C:
			break out
		}
	}
}
