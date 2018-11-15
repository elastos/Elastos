package main

import (
	"io"
	"os"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/mempool"
	"github.com/elastos/Elastos.ELA.SideChain/netsync"
	"github.com/elastos/Elastos.ELA.SideChain/peer"
	"github.com/elastos/Elastos.ELA.SideChain/pow"
	"github.com/elastos/Elastos.ELA.SideChain/server"
	"github.com/elastos/Elastos.ELA.SideChain/service"
	"github.com/elastos/Elastos.ELA.SideChain/spv"

	"github.com/elastos/Elastos.ELA.Utility/elalog"
	"github.com/elastos/Elastos.ELA.Utility/http/jsonrpc"
	"github.com/elastos/Elastos.ELA.Utility/http/restful"
	"github.com/elastos/Elastos.ELA.Utility/p2p/addrmgr"
	"github.com/elastos/Elastos.ELA.Utility/p2p/connmgr"
)

const (
	defaultMaxPerLogFileSize int64 = elalog.MBSize * 20
	defaultMaxLogsFolderSize int64 = elalog.GBSize * 2
)

// configFileWriter returns the configured parameters for log file writer.
func configFileWriter() (string, int64, int64) {
	maxPerLogFileSize := defaultMaxPerLogFileSize
	maxLogsFolderSize := defaultMaxLogsFolderSize
	if cfg.MaxPerLogFileSize > 0 {
		maxPerLogFileSize = cfg.MaxPerLogFileSize * elalog.MBSize
	}
	if cfg.MaxLogsFolderSize > 0 {
		maxLogsFolderSize = cfg.MaxLogsFolderSize * elalog.MBSize
	}
	return defaultLogDir, maxPerLogFileSize, maxLogsFolderSize
}

// log is a logger that is initialized with no output filters.  This
// means the package will not perform any logging by default until the caller
// requests it.
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
	rpcslog = backend.Logger("RPCS", cfg.LogLevel)
	restlog = backend.Logger("REST", cfg.LogLevel)
	eladlog = backend.Logger("ELAD", cfg.LogLevel)
)

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
	jsonrpc.UseLogger(rpcslog)
	restful.UseLogger(restlog)
}
