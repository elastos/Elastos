package log

import (
	elaLog "github.com/elastos/Elastos.ELA/log"
)

const (
	OutputPath = "./ArbiterLogs/" // The log files output path
)

var logger *elaLog.Logger

func Init(level uint8, maxPerLogSizeMb, maxLogsSizeMb int64) {
	logger = elaLog.NewLogger(OutputPath, level, maxPerLogSizeMb, maxLogsSizeMb)
}

func Debug(a ...interface{}) {
	logger.Debug(a...)
}

func Debugf(format string, a ...interface{}) {
	logger.Debugf("%s %s:%d "+format, a...)
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

func SetPrintLevel(level uint8) {
	logger.SetPrintLevel(level)
}
