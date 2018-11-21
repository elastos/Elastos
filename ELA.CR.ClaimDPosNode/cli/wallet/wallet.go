package wallet

import (
	"fmt"
	"github.com/elastos/Elastos.ELA/account"
	"os"

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
	password, err = GetPassword(password, true)
	if err != nil {
		return err
	}

	_, err = account.Create(name, password)
	if err != nil {
		return err
	}

	return ShowAccountInfo(name, password)
}

func walletAction(context *cli.Context) error {
	if context.NumFlags() == 0 {
		cli.ShowSubcommandHelp(context)
		return nil
	}
	// wallet name is wallet.dat by default
	name := context.String("name")
	if name == "" {
		os.Exit(1)
	}
	passwd := context.String("password")

	// create wallet
	if context.Bool("create") {
		if err := createWallet(name, []byte(passwd)); err != nil {
			fmt.Println("error: create wallet failed,", err)
			cli.ShowCommandHelpAndExit(context, "create", 1)
		}
		return nil
	}

	wallet := &account.WalletImpl{}
	password, err := GetPassword([]byte{}, false)
	if err != nil {
		return err
	}
	err = wallet.Open(account.DefaultKeystoreFile, []byte(password))
	if err != nil {
		return err
	}

	// transaction actions
	if param := context.String("transaction"); param != "" {
		switch param {
		case "create":
			if err := createTransaction(context, wallet); err != nil {
				fmt.Println("error:", err)
				os.Exit(701)
			}
		case "sign":
			if err := signTransaction(name, []byte(passwd), context, wallet); err != nil {
				fmt.Println("error:", err)
				os.Exit(702)
			}
		case "send":
			if err := sendTransaction(context); err != nil {
				fmt.Println("error:", err)
				os.Exit(703)
			}
		default:
			cli.ShowCommandHelpAndExit(context, "transaction", 700)
		}
	}

	// show account info
	if context.Bool("account") {
		if err := ShowAccountInfo(name, []byte(password)); err != nil {
			fmt.Println("error: show account info failed,", err)
			cli.ShowCommandHelpAndExit(context, "account", 3)
		}
	}

	// list accounts information
	if context.Bool("list") {
		if err := ShowAccountBalance(name, []byte(password)); err != nil {
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
			cli.StringFlag{
				Name: "transaction, t",
				Usage: "use [create, sign, send], to create, sign or send a transaction\n" +
					"\tcreate:\n" +
					"\t\tuse --to --amount --fee [--lock], or --file --fee [--lock]\n" +
					"\t\tto create a standard transaction, or multi output transaction\n" +
					"\tsign, send:\n" +
					"\t\tuse --file or --hex to specify the transaction file path or content\n",
			},
			cli.StringFlag{
				Name:  "from",
				Usage: "the spend address of the transaction",
			},
			cli.StringFlag{
				Name:  "to",
				Usage: "the receive address of the transaction",
			},
			cli.StringFlag{
				Name:  "amount",
				Usage: "the transfer amount of the transaction",
			},
			cli.StringFlag{
				Name:  "fee",
				Usage: "the transfer fee of the transaction",
			},
			cli.StringFlag{
				Name:  "lock",
				Usage: "the lock time to specify when the received asset can be spent",
			},
			cli.StringFlag{
				Name:  "hex",
				Usage: "the transaction content in hex string format to be sign or send",
			},
			cli.StringFlag{
				Name: "file, f",
				Usage: "the file path to specify a CSV file path with [address,amount] format as multi output content,\n" +
					"\tor the transaction file path with the hex string content to be sign or send",
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
				Value: account.DefaultKeystoreFile,
			},
			cli.StringFlag{
				Name:  "password, p",
				Usage: "wallet password",
			},
		},
		Action: walletAction,
		OnUsageError: func(c *cli.Context, err error, isSubcommand bool) error {
			PrintError(c, err, "wallet")
			return cli.NewExitError("", 1)
		},
	}
}
