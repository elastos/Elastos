package main

import (
	"io"
	"os"
	"path/filepath"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/mempool"
	"github.com/elastos/Elastos.ELA.SideChain/netsync"
	"github.com/elastos/Elastos.ELA.SideChain/peer"
	"github.com/elastos/Elastos.ELA.SideChain/pow"
	"github.com/elastos/Elastos.ELA.SideChain/server"
	"github.com/elastos/Elastos.ELA.SideChain/service"
	"github.com/elastos/Elastos.ELA.SideChain/spv"

	"github.com/elastos/Elastos.ELA/p2p/addrmgr"
	"github.com/elastos/Elastos.ELA/p2p/connmgr"
	"github.com/elastos/Elastos.ELA/utils/elalog"
)

const (
	defaultPerLogFileSize int64 = elalog.MBSize * 20
	defaultLogsFolderSize int64 = elalog.GBSize * 2
)

// configFileWriter returns the configured parameters for log file writer.
func configFileWriter() (string, int64, int64) {
	perLogFileSize := defaultPerLogFileSize
	logsFolderSize := defaultLogsFolderSize
	if cfg.PerLogFileSize > 0 {
		perLogFileSize = cfg.PerLogFileSize * elalog.MBSize
	}
	if cfg.LogsFolderSize > 0 {
		logsFolderSize = cfg.LogsFolderSize * elalog.MBSize
	}
	return filepath.Join(DataPath, defaultLogDir), perLogFileSize, logsFolderSize
}

// log is a logger that is initialized with no output filters.  This means the
// package will not perform any logging by default until the caller requests it.
var (
	fileWriter = elalog.NewFileWriter(configFileWriter())
	logWriter  = io.MultiWriter(os.Stdout, fileWriter)
	backend    = elalog.NewBackend(logWriter, elalog.Llongfile)

	admrlog = backend.Logger("ADMR", elalog.LevelOff)
	cmgrlog = backend.Logger("CMGR", elalog.LevelOff)
	bcdblog = backend.Logger("BCDB", cfg.LogLevel)
	txmplog = backend.Logger("TXMP", cfg.LogLevel)
	synclog = backend.Logger("SYNC", cfg.LogLevel)
	peerlog = backend.Logger("PEER", cfg.LogLevel)
	minrlog = backend.Logger("MINR", cfg.LogLevel)
	spvslog = backend.Logger("SPVS", cfg.LogLevel)
	srvrlog = backend.Logger("SRVR", cfg.LogLevel)
	httplog = backend.Logger("HTTP", cfg.LogLevel)
	eladlog = backend.Logger("ELAD", cfg.LogLevel)
)

func setLogLevel(level elalog.Level) {
	bcdblog.SetLevel(level)
	txmplog.SetLevel(level)
	synclog.SetLevel(level)
	peerlog.SetLevel(level)
	minrlog.SetLevel(level)
	spvslog.SetLevel(level)
	srvrlog.SetLevel(level)
	httplog.SetLevel(level)
	eladlog.SetLevel(level)
}

// The default amount of logging is none.
func init() {
	addrmgr.UseLogger(admrlog)
	connmgr.UseLogger(cmgrlog)
	blockchain.UseLogger(bcdblog)
	mempool.UseLogger(txmplog)
	netsync.UseLogger(synclog)
	peer.UseLogger(peerlog)
	server.UseLogger(srvrlog)
	pow.UseLogger(minrlog)
	spv.UseLogger(spvslog)
	service.UseLogger(httplog)
}
