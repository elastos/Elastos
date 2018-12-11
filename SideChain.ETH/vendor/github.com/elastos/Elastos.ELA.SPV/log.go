package main

import (
	"io"
	"os"

	"github.com/elastos/Elastos.ELA.SPV/blockchain"
	"github.com/elastos/Elastos.ELA.SPV/peer"
	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA.SPV/sync"
	"github.com/elastos/Elastos.ELA.SPV/wallet"
	"github.com/elastos/Elastos.ELA.SPV/wallet/store"

	"github.com/elastos/Elastos.ELA.Utility/elalog"
	"github.com/elastos/Elastos.ELA.Utility/http/jsonrpc"
	"github.com/elastos/Elastos.ELA.Utility/p2p/addrmgr"
	"github.com/elastos/Elastos.ELA.Utility/p2p/connmgr"
	"github.com/elastos/Elastos.ELA.Utility/p2p/server"
)

const LogPath = "./logs-spv/"

// log is a logger that is initialized with no output filters.  This
// means the package will not perform any logging by default until the caller
// requests it.
var (
	fileWriter = elalog.NewFileWriter(
		LogPath,
		config.MaxPerLogSize,
		config.MaxLogsSize,
	)
	level   = elalog.Level(config.PrintLevel)
	backend = elalog.NewBackend(io.MultiWriter(os.Stdout, fileWriter),
		elalog.Llongfile)

	admrlog = backend.Logger("ADMR", elalog.LevelOff)
	cmgrlog = backend.Logger("CMGR", elalog.LevelOff)
	bcdblog = backend.Logger("BCDB", level)
	synclog = backend.Logger("SYNC", level)
	peerlog = backend.Logger("PEER", level)
	spvslog = backend.Logger("SPVS", elalog.LevelInfo)
	srvrlog = backend.Logger("SRVR", level)
	rpcslog = backend.Logger("RPCS", level)
	waltlog = backend.Logger("WALT", level)
)

func init() {
	addrmgr.UseLogger(admrlog)
	connmgr.UseLogger(cmgrlog)
	blockchain.UseLogger(bcdblog)
	sdk.UseLogger(spvslog)
	jsonrpc.UseLogger(rpcslog)
	peer.UseLogger(peerlog)
	server.UseLogger(srvrlog)
	store.UseLogger(bcdblog)
	sync.UseLogger(synclog)
	wallet.UseLogger(waltlog)
}
