package main

import (
	"github.com/elastos/Elastos.ELA.Utility/elalog"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/elanet"
	"github.com/elastos/Elastos.ELA/elanet/netsync"
	"github.com/elastos/Elastos.ELA/elanet/peer"
	"github.com/elastos/Elastos.ELA/p2p/addrmgr"
	"github.com/elastos/Elastos.ELA/p2p/connmgr"
)

type logWrapper struct {
	level  elalog.Level
	logger elalog.Logger
}

func (l *logWrapper) Debug(a ...interface{}) {
	if l.level > elalog.LevelDebug {
		return
	}
	l.logger.Debug(a...)
}

func (l *logWrapper) Debugf(format string, a ...interface{}) {
	if l.level > elalog.LevelDebug {
		return
	}
	l.logger.Debugf(format, a...)
}

func (l *logWrapper) Info(a ...interface{}) {
	if l.level > elalog.LevelInfo {
		return
	}
	l.logger.Info(a...)
}

func (l *logWrapper) Infof(format string, a ...interface{}) {
	if l.level > elalog.LevelInfo {
		return
	}
	l.logger.Infof(format, a...)
}

func (l *logWrapper) Warn(a ...interface{}) {
	if l.level > elalog.LevelWarn {
		return
	}
	l.logger.Warn(a...)
}

func (l *logWrapper) Warnf(format string, a ...interface{}) {
	if l.level > elalog.LevelWarn {
		return
	}
	l.logger.Warnf(format, a...)
}

func (l *logWrapper) Error(a ...interface{}) {
	if l.level > elalog.LevelError {
		return
	}
	l.logger.Error(a...)
}

func (l *logWrapper) Errorf(format string, a ...interface{}) {
	if l.level > elalog.LevelError {
		return
	}
	l.logger.Errorf(format, a...)
}

func (l *logWrapper) Fatal(a ...interface{}) {
	if l.level > elalog.LevelFatal {
		return
	}
	l.logger.Fatal(a...)
}

func (l *logWrapper) Fatalf(format string, a ...interface{}) {
	if l.level > elalog.LevelFatal {
		return
	}
	l.logger.Fatalf(format, a...)
}

// Level returns the current logging level.
func (l *logWrapper) Level() elalog.Level {
	return l.level
}

// SetLevel changes the logging level to the passed level.
func (l *logWrapper) SetLevel(level elalog.Level) {
	l.level = level
}

func wrap(logger *log.Logger, level elalog.Level) *logWrapper {
	return &logWrapper{logger: logger, level: level}
}

// log is a logger that is initialized with no output filters.  This
// means the package will not perform any logging by default until the caller
// requests it.
var (
	logger = log.NewDefault(cfg.PrintLevel, cfg.MaxPerLogSize, cfg.MaxLogsSize)
	level  = elalog.Level(cfg.PrintLevel)

	admrlog = wrap(logger, elalog.LevelOff)
	cmgrlog = wrap(logger, elalog.LevelOff)
	synclog = wrap(logger, level)
	peerlog = wrap(logger, level)
	elanlog = wrap(logger, level)
)

// The default amount of logging is none.
func init() {
	addrmgr.UseLogger(admrlog)
	connmgr.UseLogger(cmgrlog)
	netsync.UseLogger(synclog)
	peer.UseLogger(peerlog)
	elanet.UseLogger(elanlog)
}
