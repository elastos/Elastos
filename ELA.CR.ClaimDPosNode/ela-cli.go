package main

import (
	"math/rand"
	"os"
	"sort"
	"time"

	"github.com/elastos/Elastos.ELA/cli/script"
	"github.com/elastos/Elastos.ELA/cli/transfer"
	"github.com/elastos/Elastos.ELA/cli/wallet"
	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/log"

	"github.com/urfave/cli"
)

var Version string

func init() {
	log.Init(
		config.Parameters.PrintLevel,
		config.Parameters.MaxPerLogSize,
		config.Parameters.MaxLogsSize,
	)

	//seed transaction nonce
	rand.Seed(time.Now().UnixNano())
}

func main() {
	app := cli.NewApp()
	app.Name = "ela-cli"
	app.Version = Version
	app.HelpName = "ela-cli"
	app.Usage = "command line tool for ELA blockchain"
	app.UsageText = "ela-cli [global options] command [command options] [args]"
	app.HideHelp = false
	app.HideVersion = false
	//commands
	app.Commands = []cli.Command{
		*wallet.NewCommand(),
		*transfer.NewCommand(),
		*script.NewCommand(),
	}
	sort.Sort(cli.CommandsByName(app.Commands))
	sort.Sort(cli.FlagsByName(app.Flags))

	app.Run(os.Args)
}
