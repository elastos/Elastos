package spv

import (
	"github.com/elastos/Elastos.ELA.SideChain.ETH/log"
	"io"
	"os"
	"path/filepath"

	"github.com/elastos/Elastos.ELA.SPV/blockchain"
	"github.com/elastos/Elastos.ELA.SPV/peer"
	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA.SPV/sync"
	"github.com/elastos/Elastos.ELA.SPV/wallet/store"
	"github.com/elastos/Elastos.ELA/p2p/addrmgr"
	"github.com/elastos/Elastos.ELA/p2p/connmgr"
	"github.com/elastos/Elastos.ELA/p2p/server"
	"github.com/elastos/Elastos.ELA/utils/elalog"
)

const LogPath = "./logs-spv/"

// log is a logger that is initialized with no output filters.  This
// means the package will not perform any logging by default until the caller
// requests it.

func initLog(dataDir string) {

	var (
		fileWriter = elalog.NewFileWriter(
			filepath.Join(dataDir, LogPath),
			PreferConfig.Config.MaxPerLogSize,
			PreferConfig.Config.MaxLogsSize,
		)
		level   = elalog.Level(PreferConfig.Config.SpvPrintLevel)
		backend = elalog.NewBackend(io.MultiWriter(os.Stdout, fileWriter),
			elalog.Llongfile)

		admrlog = backend.Logger("ADMR", level)
		cmgrlog = backend.Logger("CMGR", level)
		bcdblog = backend.Logger("BCDB", level)
		synclog = backend.Logger("SYNC", level)
		peerlog = backend.Logger("PEER", level)
		spvslog = backend.Logger("SPVS", level)
		srvrlog = backend.Logger("SRVR", elalog.LevelOff)
	)

	addrmgr.UseLogger(admrlog)
	connmgr.UseLogger(cmgrlog)
	blockchain.UseLogger(bcdblog)
	sdk.UseLogger(spvslog)
	peer.UseLogger(peerlog)
	server.UseLogger(srvrlog)
	store.UseLogger(bcdblog)
	sync.UseLogger(synclog)

	log.Info("SPV Logs initialized at: ", "dir", filepath.Join(dataDir, LogPath))
}
