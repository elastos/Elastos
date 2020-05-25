// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package main

import (
	"fmt"
	"io"
	"path/filepath"
	"time"

	"github.com/elastos/Elastos.ELA/common/config/settings"
	"github.com/elastos/Elastos.ELA/common/log"
	crstate "github.com/elastos/Elastos.ELA/cr/state"
	"github.com/elastos/Elastos.ELA/dpos/state"
	"github.com/elastos/Elastos.ELA/elanet"
	"github.com/elastos/Elastos.ELA/elanet/netsync"
	"github.com/elastos/Elastos.ELA/elanet/peer"
	"github.com/elastos/Elastos.ELA/elanet/routes"
	"github.com/elastos/Elastos.ELA/p2p/addrmgr"
	"github.com/elastos/Elastos.ELA/p2p/connmgr"
	"github.com/elastos/Elastos.ELA/utils/elalog"

	"github.com/urfave/cli"
	"gopkg.in/cheggaaa/pb.v1"
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

const (
	// progressRefreshRate indicates the duration between refresh progress.
	progressRefreshRate = time.Millisecond * 500

	// startString defines the print out message when start progress.
	startString = "[ ========== BLOCKCHAIN INITIALIZE STARTED ========== ]"

	// finishString defines the print out message when finish progress.
	finishString = "[ ========== BLOCKCHAIN INITIALIZE FINISHED ========== ]"
)

// progress shows a progress bar in the terminal and print blockchain initialize
// progress into log file.
type progress struct {
	w  io.Writer
	pb *pb.ProgressBar
}

func (p *progress) Start(total uint32) {
	fmt.Fprintln(p.w, startString)
	p.pb = pb.New64(int64(total))
	p.pb.Output = p.w
	p.pb.ShowTimeLeft = false
	p.pb.ShowFinalTime = true
	p.pb.SetRefreshRate(progressRefreshRate)
	p.pb.Start()
}

func (p *progress) Increase() {
	if p.pb != nil {
		p.pb.Increment()
	}
}

func (p *progress) Stop() {
	if p.pb != nil {
		p.pb.FinishPrint(finishString)
	}
}

// newProgress creates a progress instance.
func newProgress(w io.Writer) *progress {
	return &progress{w: w}
}

// log is a logger that is initialized with no output filters.  This
// means the package will not perform any logging by default until the caller
// requests it.
var (
	logger *log.Logger
	pgBar  *progress
)

// The default amount of logging is none.
func setupLog(c *cli.Context, s *settings.Settings) {
	flagDataDir := c.String("datadir")
	path := filepath.Join(flagDataDir, nodeLogPath)

	logger = log.NewDefault(path, uint8(s.Params().PrintLevel),
		s.Config().MaxPerLogSize, s.Config().MaxLogsSize)
	pgBar = newProgress(logger.Writer())

	admrlog := wrap(logger, elalog.LevelOff)
	cmgrlog := wrap(logger, elalog.LevelOff)
	synclog := wrap(logger, elalog.Level(s.Params().PrintLevel))
	peerlog := wrap(logger, elalog.Level(s.Params().PrintLevel))
	routlog := wrap(logger, elalog.Level(s.Params().PrintLevel))
	elanlog := wrap(logger, elalog.Level(s.Params().PrintLevel))
	statlog := wrap(logger, elalog.Level(s.Params().PrintLevel))
	crstatlog := wrap(logger, elalog.Level(s.Params().PrintLevel))

	addrmgr.UseLogger(admrlog)
	connmgr.UseLogger(cmgrlog)
	netsync.UseLogger(synclog)
	peer.UseLogger(peerlog)
	routes.UseLogger(routlog)
	elanet.UseLogger(elanlog)
	state.UseLogger(statlog)
	crstate.UseLogger(crstatlog)
}
