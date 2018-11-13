package peer

import (
	"fmt"

	"github.com/elastos/Elastos.ELA.Utility/elalog"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
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

// LogClosure is a closure that can be printed with %v to be used to
// generate expensive-to-create data for a detailed log level and avoid doing
// the work if the data isn't printed.
type logClosure func() string

func (c logClosure) String() string {
	return c()
}

func newLogClosure(c func() string) logClosure {
	return logClosure(c)
}

// directionString is a helper function that returns a string that represents
// the direction of a connection (inbound or outbound).
func directionString(inbound bool) string {
	if inbound {
		return "inbound"
	}
	return "outbound"
}

// messageSummary returns a human-readable string which summarizes a message.
// Not all messages have or need a summary.  This is used for debug logging.
func messageSummary(message p2p.Message) string {
	switch message := message.(type) {
	case *msg.Version:
		return fmt.Sprintf("pver %d, block %d", message.Version, message.Height)

	case *msg.VerAck:
		// No summary.

	case *msg.GetAddr:
		// No summary.

	case *msg.Addr:
		return fmt.Sprintf("%d addr", len(message.AddrList))

	case *msg.Ping:
		// No summary - perhaps add nonce.

	case *msg.Pong:
		// No summary - perhaps add nonce.
	}

	// No summary for other messages.
	return ""
}
