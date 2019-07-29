package _interface

import (
	"github.com/elastos/Elastos.ELA.SPV/blockchain"
	"github.com/elastos/Elastos.ELA.SPV/peer"
	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA.SPV/sync"
	"github.com/elastos/Elastos.ELA.SPV/wallet"
	"github.com/elastos/Elastos.ELA.SPV/wallet/store"
	"github.com/elastos/Elastos.ELA/utils/elalog"
)

// log is a logger that is initialized with no output filters.  This
// means the package will not perform any logging by default until the caller
// requests it.
var log elalog.Logger

// The default amount of logging is none.
func init() {
	DisableLog()
}

// DisableLog disables all library log output.  Logging output is disabled
// by default until either UseLogger or SetLogWriter are called.
func DisableLog() {
	log = elalog.Disabled
}

// UseLogger uses a specified Logger to output package logging info.
// This should be used in preference to SetLogWriter if the caller is also
// using elalog.
func UseLogger(logger elalog.Logger) {
	log = logger
	blockchain.UseLogger(logger)
	sdk.UseLogger(logger)
	peer.UseLogger(logger)
	store.UseLogger(logger)
	sync.UseLogger(logger)
	wallet.UseLogger(logger)
}
