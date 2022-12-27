package main

import (
	"io"
	"os"
	"path/filepath"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	sm "github.com/elastos/Elastos.ELA.SideChain/mempool"
	"github.com/elastos/Elastos.ELA.SideChain/netsync"
	"github.com/elastos/Elastos.ELA.SideChain/peer"
	"github.com/elastos/Elastos.ELA.SideChain/server"
	ser "github.com/elastos/Elastos.ELA.SideChain/service"
	"github.com/elastos/Elastos.ELA.SideChain/spv"
	spow "github.com/elastos/Elastos.ELA.SideChain/pow"

	"github.com/elastos/Elastos.ELA/utils/elalog"
	"github.com/elastos/Elastos.ELA/p2p/addrmgr"
	"github.com/elastos/Elastos.ELA/p2p/connmgr"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/store"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/smartcontract/service"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/service/websocket"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/pow"
	nser "github.com/elastos/Elastos.ELA.SideChain.NeoVM/p2p/server"
)

const (
	defaultMaxPerLogFileSize int64 = elalog.MBSize * 20
	defaultMaxLogsFolderSize int64 = elalog.GBSize * 2
)

// configFileWriter returns the configured parameters for log file writer.
func configFileWriter() (string, int64, int64) {
	perLogFileSize := defaultMaxPerLogFileSize
	logsFolderSize := defaultMaxLogsFolderSize
	if cfg.PerLogFileSize > 0 {
		perLogFileSize = cfg.PerLogFileSize * elalog.MBSize
	}
	if cfg.LogsFolderSize > 0 {
		logsFolderSize = cfg.LogsFolderSize * elalog.MBSize
	}
	return filepath.Join(DataPath, defaultLogDir), perLogFileSize, logsFolderSize
}

// log is a logger that is initialized with no output filters.  This
// means the package will not perform any logging by default until the caller
// requests it.
var (
	fileWriter = elalog.NewFileWriter(configFileWriter())
	logWriter  = io.MultiWriter(os.Stdout, fileWriter)
	backend    = elalog.NewBackend(logWriter, elalog.Llongfile)
	level = cfg.LogLevel

	admrlog = backend.Logger("ADMR", elalog.LevelOff)
	cmgrlog = backend.Logger("CMGR", elalog.LevelOff)
	bcdblog = backend.Logger("BCDB", level)
	txmplog = backend.Logger("TXMP", level)
	synclog = backend.Logger("SYNC", level)
	peerlog = backend.Logger("PEER", level)
	minrlog = backend.Logger("MINR", level)
	spvslog = backend.Logger("SPVS", level)
	srvrlog = backend.Logger("SRVR", level)
	httplog = backend.Logger("HTTP", level)
	rpcslog = backend.Logger("RPCS", level)
	restlog = backend.Logger("REST", level)
	eladlog = backend.Logger("ELAD", level)

	sockLog = backend.Logger("SOCKET", level)
	avmlog  = backend.Logger("AVM", level)
	nsrvrlog  = backend.Logger("NVMSERVER", level)
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
	rpcslog.SetLevel(level)
	restlog.SetLevel(level)
	eladlog.SetLevel(level)
	sockLog.SetLevel(level)
	avmlog.SetLevel(level)
	nsrvrlog.SetLevel(level)
}

// The default amount of logging is none.
func init() {
	addrmgr.UseLogger(admrlog)
	connmgr.UseLogger(cmgrlog)
	blockchain.UseLogger(bcdblog)
	sm.UseLogger(txmplog)
	netsync.UseLogger(synclog)
	peer.UseLogger(peerlog)
	server.UseLogger(srvrlog)
	pow.UseLogger(minrlog)
	spow.UseLogger(minrlog)
	spv.UseLogger(spvslog)
	ser.UseLogger(httplog)

	avm.UseLogger(avmlog)
	store.UseLogger(avmlog)
	service.UseLogger(avmlog)
	websocket.UseLogger(sockLog)
	nser.UseLogger(nsrvrlog)
}
