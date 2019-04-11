package common

import (
	"github.com/urfave/cli"
)

var (
	// rpc flags
	RPCUserFlag = cli.StringFlag{
		Name:  "rpcuser",
		Usage: "username for JSON-RPC connections",
	}
	RPCPasswordFlag = cli.StringFlag{
		Name:  "rpcpassword",
		Usage: "password for JSON-RPC connections",
	}
	RPCPortFlag = cli.StringFlag{
		Name:  "rpcport",
		Usage: "JSON-RPC server listening port `<number>`",
	}

	// config flags
	ConfigFileFlag = cli.StringFlag{
		Name:  "config",
		Usage: "config `<file>` path, ",
		Value: DefaultConfigPath,
	}
	DataDirFlag = cli.StringFlag{
		Name:  "datadir",
		Usage: "block data and logs storage `<path>`",
		Value: DefaultDataDir,
	}
)
