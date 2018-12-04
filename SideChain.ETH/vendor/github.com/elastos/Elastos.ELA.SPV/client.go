package main

import (
	"os"

	"github.com/elastos/Elastos.ELA.SPV/log"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/cli/account"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/cli/transaction"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/cli/wallet"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/config"

	"github.com/urfave/cli"
)

var Version string

func init() {
	log.Init(
		config.Values().PrintLevel,
		config.Values().MaxPerLogSize,
		config.Values().MaxLogsSize,
	)
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
