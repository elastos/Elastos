package transfer

import (
	"fmt"
	"os"

	"github.com/elastos/Elastos.ELA/account"
	"github.com/elastos/Elastos.ELA/cli/common"

	"github.com/urfave/cli"
)

func transferAction(context *cli.Context) error {
	if context.NumFlags() == 0 {
		cli.ShowSubcommandHelp(context)
		return nil
	}

	// wallet name is keystore.dat by default
	name := context.String("name")
	passwd := context.String("password")

	// transaction actions
	if param := context.String("transaction"); param != "" {
		switch param {
		case "create":
			if err := createTransaction(context); err != nil {
				fmt.Println("error:", err)
				os.Exit(701)
			}
		case "sign":
			password, err := common.GetPassword([]byte(passwd), false)
			if err != nil {
				return err
			}
			client, err := account.Open(name, password)
			if err != nil {
				return err
			}
			if err := signTransaction(context, client); err != nil {
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

	return nil
}

func NewCommand() *cli.Command {
	return &cli.Command{
		Name:        "transfer",
		Usage:       "user transfer operation",
		Description: "With ela-cli transfer, you could control assert through transaction.",
		ArgsUsage:   "[args]",
		Flags: []cli.Flag{
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
		Action: transferAction,
		OnUsageError: func(c *cli.Context, err error, isSubcommand bool) error {
			common.PrintError(c, err, "transfer")
			return cli.NewExitError("", 1)
		},
	}
}
