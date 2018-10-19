package wallet

import (
	"github.com/elastos/Elastos.ELA.SPV/util"
	"os"

	"github.com/elastos/Elastos.ELA.SPV/wallet/client"
	"github.com/elastos/Elastos.ELA.SPV/wallet/client/account"
	"github.com/elastos/Elastos.ELA.SPV/wallet/client/transaction"
	"github.com/elastos/Elastos.ELA.SPV/wallet/client/wallet"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/urfave/cli"
)

func RunClient(version, rpcUrl string, assetId common.Uint256, newBlockHeader func() util.BlockHeader) {
	client.Setup(rpcUrl, assetId, newBlockHeader)

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
