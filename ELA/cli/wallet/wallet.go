package wallet

import (
	"fmt"
	"os"

	"github.com/elastos/Elastos.ELA/account"
	"github.com/elastos/Elastos.ELA/cli/common"
	"github.com/elastos/Elastos.ELA/util"

	"github.com/urfave/cli"
)

func getConfirmedPassword(passwd string) []byte {
	var tmp []byte
	var err error
	if passwd != "" {
		tmp = []byte(passwd)
	} else {
		tmp, err = GetConfirmedPassword()
		if err != nil {
			fmt.Fprintln(os.Stderr, err)
			os.Exit(1)
		}
	}
	return tmp
}

func createWallet(name string, password []byte) error {
	var err error
	password, err = util.GetPassword(password, true)
	if err != nil {
		return err
	}

	client, err := account.Create(name, password)
	if err != nil {
		return err
	}

	return ShowAccountInfo(client)
}

func walletAction(context *cli.Context) error {
	if context.NumFlags() == 0 {
		cli.ShowSubcommandHelp(context)
		return nil
	}
	// wallet name is keystore.dat by default
	name := context.String("name")
	passwd := context.String("password")

	// create wallet
	if context.Bool("create") {
		if err := createWallet(name, []byte(passwd)); err != nil {
			fmt.Println("error: create wallet failed,", err)
			cli.ShowCommandHelpAndExit(context, "create", 1)
		}
		return nil
	}

	// show account info
	if context.Bool("account") {
		password, err := util.GetPassword([]byte(passwd), false)
		if err != nil {
			return err
		}
		client, err := account.Open(name, password)
		if err != nil {
			return err
		}
		if err := ShowAccountInfo(client); err != nil {
			fmt.Println("error: show account info failed,", err)
			cli.ShowCommandHelpAndExit(context, "account", 3)
		}
	}

	// list accounts information
	if context.Bool("list") {
		if err := ShowAccountBalance(name); err != nil {
			fmt.Println("error: list accounts information failed,", err)
			cli.ShowCommandHelpAndExit(context, "list", 6)
		}
	}

	return nil
}

func NewCommand() *cli.Command {
	return &cli.Command{
		Name:        "wallet",
		Usage:       "user wallet operation",
		Description: "With ela-cli wallet, you could control your asset.",
		ArgsUsage:   "[args]",
		Flags: []cli.Flag{
			cli.BoolFlag{
				Name:  "create, c",
				Usage: "create wallet",
			},
			cli.BoolFlag{
				Name:  "list, l",
				Usage: "list wallet information [account, balance, verbose]",
			},
			cli.IntFlag{
				Name:  "addaccount",
				Usage: "add new account address",
			},
			cli.StringFlag{
				Name:  "addmultisigaccount",
				Usage: "add new multi-sign account address",
			},
			cli.BoolFlag{
				Name:  "account, a",
				Usage: "show account address, public key and program hash",
			},
			cli.BoolFlag{
				Name:  "changepassword",
				Usage: "change wallet password",
			},
			cli.BoolFlag{
				Name:  "reset",
				Usage: "reset wallet",
			},
			cli.StringFlag{
				Name:  "name, n",
				Usage: "wallet name",
				Value: account.KeystoreFileName,
			},
			cli.StringFlag{
				Name:  "password, p",
				Usage: "wallet password",
			},
		},
		Action: walletAction,
		OnUsageError: func(c *cli.Context, err error, isSubcommand bool) error {
			common.PrintError(c, err, "wallet")
			return cli.NewExitError("", 1)
		},
	}
}
