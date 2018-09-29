package main

import (
	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/logger"
	"github.com/elastos/Elastos.ELA.SideChain/mempool"
	"github.com/elastos/Elastos.ELA.SideChain/netsync"
	"github.com/elastos/Elastos.ELA.SideChain/peer"
	"github.com/elastos/Elastos.ELA.SideChain/pow"
	"github.com/elastos/Elastos.ELA.SideChain/servers"
	"github.com/elastos/Elastos.ELA.SideChain/servers/httpjsonrpc"
	"github.com/elastos/Elastos.ELA.SideChain/servers/httprestful"
)

const LogPath = "./logs/"

// log is a logger that is initialized with no output filters.  This
// means the package will not perform any logging by default until the caller
// requests it.
var (
	elalog = logger.NewLog(
		LogPath,
		config.Parameters.PrintLevel,
		config.Parameters.MaxPerLogSize,
		config.Parameters.MaxLogsSize,
	)

	bcdblog = elalog.Logger("BCDB")
	txmplog = elalog.Logger("TXMP")
	synclog = elalog.Logger("SYNC")
	peerlog = elalog.Logger("PEER")
	minrlog = elalog.Logger("MINR")
	srvrlog = elalog.Logger("SRVR")
	httplog = elalog.Logger("HTTP")
	eladlog = elalog.Logger("ELAD")
)

// The default amount of logging is none.
func init() {
	blockchain.UseLogger(bcdblog)
	mempool.UseLogger(txmplog)
	netsync.UseLogger(synclog)
	peer.UseLogger(peerlog)
	pow.UseLogger(minrlog)
	servers.UseLogger(httplog)
	httpjsonrpc.UseLogger(httplog)
	httprestful.UseLogger(httplog)
}
