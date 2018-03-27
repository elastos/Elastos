package main

import (
	"os"

	"SPVWallet/log"
	"SPVWallet/cli/account"
	"SPVWallet/cli/transaction"
	"SPVWallet/cli/wallet"

	"github.com/urfave/cli"
)

var Version string

func init() {
	log.Init(false)
}

func main() {
	app := cli.NewApp()
	app.Name = "ELASTOS SPV WALLET"
	app.Version = Version
	app.HelpName = "ELASTOS SPV WALLET HELP"
	app.Usage = "command line user interface"
	app.UsageText = "[global option] command [command options] [args]"
	app.HideHelp = false
	app.HideVersion = false
	//commands
	app.Commands = []cli.Command{
		wallet.NewCreateCommand(),
		wallet.NewChangePasswordCommand(),
		wallet.NewResetCommand(),
		account.NewCommand(),
		transaction.NewCommand(),
	}

	app.Run(os.Args)
}
