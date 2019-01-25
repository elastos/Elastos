package wallet

import (
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
	"os"

	"github.com/elastos/Elastos.ELA/account"
	clicom "github.com/elastos/Elastos.ELA/cmd/common"
	pwd "github.com/elastos/Elastos.ELA/cmd/password"
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

func createWallet(name string, password string) error {
	var p []byte
	if password == "" {
		var err error
		p, err = pwd.GetConfirmedPassword()
		if err != nil {
			return err
		}
	} else {
		p = []byte(password)
	}

	client, err := account.Create(name, p)
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

	acc, err := account.NewAccountWithPrivateKey(privateKeyBytes)
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

func generateDepositAddress(addr string, name string) (string, error) {
	var programHash *common.Uint168
	var err error

	if addr == "" {
		var fileStore account.FileStore
		fileStore.SetPath(name)
		storeAccounts, err := fileStore.LoadAccountData()
		if err != nil {
			return "", err
		}
		for _, a := range storeAccounts {
			if a.Type == account.MAINACCOUNT {
				p, err := common.HexStringToBytes(a.ProgramHash)
				if err != nil {
					return "", err
				}
				programHash, err = common.Uint168FromBytes(p)
				if err != nil {
					return "", err
				}
			}
		}
	} else {
		programHash, err = common.Uint168FromAddress(addr)
		if err != nil {
			return "", err
		}
	}

	codeHash := programHash.ToCodeHash()
	depositHash := common.Uint168FromCodeHash(byte(contract.PrefixDeposit), codeHash)
	address, err := depositHash.ToAddress()
	if err != nil {
		return "", nil
	}
	return address, nil
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
		if err := createWallet(name, passwd); err != nil {
			fmt.Println("error: create wallet failed,", err)
			cli.ShowCommandHelpAndExit(context, "create", 1)
		}
		return nil
	}

	if context.Bool("account") {
		if exist := clicom.FileExisted(name); !exist {
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
		if exist := clicom.FileExisted(name); !exist {
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

	// generate deposit address
	if context.Bool("getdepositaddress") {
		addr := context.String("address")
		address, err := generateDepositAddress(addr, name)
		if err != nil {
			fmt.Println("error: get deposit address failed,", err)
			cli.ShowCommandHelpAndExit(context, "getdepositaddress", 1)
		}
		fmt.Println(address)
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
			cli.BoolFlag{
				Name:  "getdepositaddress, gda",
				Usage: "generate the deposit address form main account",
			},
			cli.StringFlag{
				Name:  "address",
				Usage: "source standard address",
			},
		},
		Action: walletAction,
		Subcommands: []cli.Command{
			*NewTransactionCommand(),
		},
		OnUsageError: func(c *cli.Context, err error, isSubcommand bool) error {
			clicom.PrintError(c, err, "wallet")
			return cli.NewExitError("", 1)
		},
	}
}
