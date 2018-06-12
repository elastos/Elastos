package log

import (
	"testing"
	"time"
)

func TestNewLogger(t *testing.T) {
	logger := NewLogger(0, 5, 20)
	start := time.Now()
	for {
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

		if start.Add(time.Second * 15).Before(time.Now()) {
			break
		}
	}
}
