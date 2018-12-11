package wallet

import (
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"os"

	"github.com/elastos/Elastos.ELA/account"
	"github.com/elastos/Elastos.ELA/cli/common"
	pwd "github.com/elastos/Elastos.ELA/cli/password"
	"github.com/elastos/Elastos.ELA/core/contract"

	"github.com/urfave/cli"
)

func getConfirmedPassword(passwd string) []byte {
	var tmp []byte
	var err error
	if passwd != "" {
		tmp = []byte(passwd)
	} else {
		tmp, err = pwd.GetConfirmedPassword()
		if err != nil {
			fmt.Fprintln(os.Stderr, err)
			os.Exit(1)
		}
	}
	return tmp
}

func createWallet(name string, password string, accountType string) error {
	var err error
	var passwd []byte
	if password == "" {
		passwd, err = pwd.GetConfirmedPassword()
		if err != nil {
			return err
		}
	}

	client, err := account.Create(name, passwd, accountType)
	if err != nil {
		return err
	}

	return ShowAccountInfo(client)
}

func importAccount(name, password, privateKeyHexStr string) error {
	privateKeyBytes, err := hex.DecodeString(privateKeyHexStr)
	if err != nil {
		return err
	}
	var passwd []byte
	if len(password) == 0 {
		passwd, err = pwd.GetConfirmedPassword()
		if err != nil {
			return err
		}
	}

	var client *account.ClientImpl
	if _, err := os.Open(name); os.IsNotExist(err) {
		client = account.NewClient(name, passwd, true)
		if client == nil {
			return errors.New("client nil")
		}

	} else {
		client, err = account.Open(name, passwd)
		if err != nil {
			return err
		}
	}

	acc, err := account.NewAccountWithPrivateKey(privateKeyBytes,
		contract.PrefixStandard)
	if err != nil {
		return err
	}

	if err := client.SaveAccount(acc); err != nil {
		return err
	}

	return ShowAccountInfo(client)
}

func exportAccount(name, password string) error {
	var err error
	var passwd []byte
	if password == "" {
		passwd, err = pwd.GetPassword()
		if err != nil {
			return err
		}
	}

	client, err := account.Open(name, passwd)
	if err != nil {
		return err
	}

	accounts := client.GetAccounts()
	privateKeys := make([]string, 0, len(accounts))
	for _, account := range accounts {
		str := hex.EncodeToString(account.PrivateKey[:])
		privateKeys = append(privateKeys, str)
	}

	data, err := json.Marshal(privateKeys)
	if err != nil {
		return err
	}

	fmt.Println(string(data))
	return nil
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
		// show account info
		if accType := context.String("type"); accType != "" {
			switch accType {
			case "standard":
			case "multisig":
			case "sidechain":
			case "pledge":
			default:
				fmt.Println("error: account type not found")
				cli.ShowCommandHelpAndExit(context, "create", 1)
			}
			if err := createWallet(name, passwd, accType); err != nil {
				fmt.Println("error: create wallet failed,", err)
				cli.ShowCommandHelpAndExit(context, "create", 1)
			}
			return nil
		}
	}

	if context.Bool("account") {
		if exist := common.FileExisted(name); !exist {
			fmt.Println(fmt.Sprintf("error: %s is not found.", name))
			cli.ShowCommandHelpAndExit(context, "account", 1)
		}
		password, err := pwd.GetPassword()
		if err != nil {
			return err
		}
		client, err := account.Open(name, password)
		if err != nil {
			return err
		}
		if err := ShowAccountInfo(client); err != nil {
			fmt.Println("error: show account info failed,", err)
			cli.ShowCommandHelpAndExit(context, "account", 1)
		}
	}

	// list accounts information
	if context.Bool("list") {
		if exist := common.FileExisted(name); !exist {
			fmt.Println(fmt.Sprintf("error: %s is not found.", name))
			cli.ShowCommandHelpAndExit(context, "account", 1)
		}
		if err := ShowAccountBalance(name); err != nil {
			fmt.Println("error: list accounts information failed,", err)
			cli.ShowCommandHelpAndExit(context, "list", 1)
		}
	}

	// import account by private key
	if str := context.String("import"); len(str) > 0 {
		if err := importAccount(name, passwd, str); err != nil {
			fmt.Println("error: import account failed,", err)
			cli.ShowCommandHelpAndExit(context, "import", 1)
		}
	}

	// export account private keys
	if context.Bool("export") {
		if err := exportAccount(name, passwd); err != nil {
			fmt.Println("error: export accounts failed,", err)
			cli.ShowCommandHelpAndExit(context, "export", 1)
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
			cli.StringFlag{
				Name:  "type, t",
				Usage: "use [standard, multisig, sidechain, pledge] to indicate account type",
				Value: "standard",
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
			cli.StringFlag{
				Name:  "import",
				Usage: "import an account by private key hex string",
			},
			cli.BoolFlag{
				Name:  "export",
				Usage: "export all account private keys in hex string",
			},
		},
		Action: walletAction,
		OnUsageError: func(c *cli.Context, err error, isSubcommand bool) error {
			common.PrintError(c, err, "wallet")
			return cli.NewExitError("", 1)
		},
	}
}
