
package main

import (
	"fmt"
	"os"

	"github.com/elastos/Elastos.ELA.SPV/wallet/client"
	"github.com/elastos/Elastos.ELA.SPV/wallet/client/account"
	"github.com/elastos/Elastos.ELA.SPV/wallet/client/transaction"
	"github.com/elastos/Elastos.ELA.SPV/wallet/client/wallet"
	"github.com/elastos/Elastos.ELA/common/config"

	"github.com/urfave/cli"
)

var Version string

func main() {
	url := fmt.Sprint("http://127.0.0.1:", cfg.RPCPort, "/spvwallet")

	client.Setup(dataDir, url, config.ELAAssetID)

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
