package wallet

import (
	"os"

	"github.com/elastos/Elastos.ELA.SPV/wallet/client"
	"github.com/elastos/Elastos.ELA.SPV/wallet/client/account"
	"github.com/elastos/Elastos.ELA.SPV/wallet/client/transaction"
	"github.com/elastos/Elastos.ELA.SPV/wallet/client/wallet"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/urfave/cli"
)

func RunClient(version, dataDir, rpcUrl string, assetId common.Uint256) {
	client.Setup(dataDir, rpcUrl, assetId)

	app := cli.NewApp()
	app.Name = "ELASTOS SPV WALLET"
	app.Version = version
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
