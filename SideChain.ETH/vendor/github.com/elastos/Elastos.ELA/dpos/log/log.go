package log

import (
	elaLog "github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/dpos/p2p"
)

const (
	dposLogDir = "elastos/logs/dpos/"
)

var logger *elaLog.Logger

func Init(level uint8, maxPerLogSizeMb, maxLogsSizeMb int64) {
	logger = elaLog.NewLogger(dposLogDir, level, maxPerLogSizeMb, maxLogsSizeMb)
	p2p.UseLogger(logger)
}

func Debug(a ...interface{}) {
	logger.Debug(a...)
}

func Debugf(format string, a ...interface{}) {
	logger.Debugf(format, a...)
}

func Info(a ...interface{}) {
	logger.Info(a...)
}

func Warn(a ...interface{}) {
	logger.Warn(a...)
}

func Error(a ...interface{}) {
	logger.Error(a...)
}

func Fatal(a ...interface{}) {
	logger.Fatal(a...)
}

func Infof(format string, a ...interface{}) {
	logger.Infof(format, a...)
}

func Warnf(format string, a ...interface{}) {
	logger.Warnf(format, a...)
}

func Errorf(format string, a ...interface{}) {
	logger.Errorf(format, a...)
}

func Fatalf(format string, a ...interface{}) {
	logger.Fatalf(format, a...)
}
