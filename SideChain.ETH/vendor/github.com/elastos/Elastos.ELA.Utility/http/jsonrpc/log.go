package jsonrpc

import (
	"github.com/elastos/Elastos.ELA.Utility/elalog"
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
}
