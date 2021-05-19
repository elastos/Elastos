package store

import (
	"github.com/elastos/Elastos.ELA.SPV/wallet/store/headers"
	"github.com/elastos/Elastos.ELA.SPV/wallet/store/sqlite"

	"github.com/elastos/Elastos.ELA/utils/elalog"
)

// UseLogger uses a specified Logger to output package logging info.
// This should be used in preference to SetLogWriter if the caller is also
// using elalog.
func UseLogger(logger elalog.Logger) {
	headers.UseLogger(logger)
	sqlite.UseLogger(logger)
}
