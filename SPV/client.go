package main

import (
	"os"
	"sort"

	"SPVWallet/log"
	"SPVWallet/cli/account"
	"SPVWallet/cli/transaction"

	"github.com/urfave/cli"
)

var Version string

func init() {
	log.Init()
}

func main() {
	app := cli.NewApp()
	app.Name = "ELA SPV WALLET"
	app.Version = Version
	app.HelpName = "ELA SPV WALLET"
	app.Usage = "command line tool for the light implementation of SPV wallet"
	app.UsageText = "wallet [global option] command [command options] [args]"
	app.HideHelp = false
	app.HideVersion = false
	//commands
	app.Commands = []cli.Command{
		*account.NewCommand(),
		*transaction.NewCommand(),
	}
	sort.Sort(cli.CommandsByName(app.Commands))
	sort.Sort(cli.FlagsByName(app.Flags))

	app.Run(os.Args)
}
