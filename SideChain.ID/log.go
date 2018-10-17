package main

import (
	"io"
	"os"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/mempool"
	"github.com/elastos/Elastos.ELA.SideChain/netsync"
	"github.com/elastos/Elastos.ELA.SideChain/peer"
	"github.com/elastos/Elastos.ELA.SideChain/pow"
	"github.com/elastos/Elastos.ELA.SideChain/server"
	"github.com/elastos/Elastos.ELA.SideChain/service"
	"github.com/elastos/Elastos.ELA.SideChain/service/httpjsonrpc"
	"github.com/elastos/Elastos.ELA.SideChain/service/httprestful"

	"github.com/elastos/Elastos.ELA.Utility/elalog"
	"github.com/elastos/Elastos.ELA.Utility/p2p/addrmgr"
	"github.com/elastos/Elastos.ELA.Utility/p2p/connmgr"
)

const LogPath = "./logs/"

// log is a logger that is initialized with no output filters.  This
// means the package will not perform any logging by default until the caller
// requests it.
var (
	fileWriter = elalog.NewFileWriter(
		LogPath,
		config.Parameters.MaxPerLogSize,
		config.Parameters.MaxLogsSize,
	)
	level   = elalog.Level(config.Parameters.PrintLevel)
	backend = elalog.NewBackend(io.MultiWriter(os.Stdout, fileWriter),
		elalog.Llongfile)

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
	service.UseLogger(httplog)
	httpjsonrpc.UseLogger(rpcslog)
	httprestful.UseLogger(restlog)
}
