package wallet

import (
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"os"

	"github.com/elastos/Elastos.ELA/account"
	cmdcom "github.com/elastos/Elastos.ELA/cmd/common"

	"github.com/urfave/cli"
)

var accountCommand = []cli.Command{
	{
		Category: "Account",
		Name:     "create",
		Aliases:  []string{"c"},
		Usage:    "create a account",
		Flags: []cli.Flag{
			AccountWalletFlag,
			AccountPasswordFlag,
		},
		Action: createAccount,
	},
	{
		Category: "Account",
		Name:     "account",
		Aliases:  []string{"a"},
		Usage:    "show account address, public key and program hash",
		Flags: []cli.Flag{
			AccountWalletFlag,
		},
		Action: accountInfo,
	},
	{
		Category: "Account",
		Name:     "list",
		Aliases:  []string{"l"},
		Usage:    "list wallet information [account, balance, verbose]",
		Flags: []cli.Flag{
			AccountWalletFlag,
		},
		Action: listAccount,
	},
	{
		Category: "Account",
		Name:     "add",
		Usage:    "add a new account",
		Action:   addAccount,
	},
	{
		Category: "Account",
		Name:     "chpwd",
		Usage:    "change wallet password",
		Action:   changePassword,
	},
	{
		Category:  "Account",
		Name:      "import",
		Usage:     "import an account by private key hex string",
		ArgsUsage: "[args]",
		Flags: []cli.Flag{
			AccountWalletFlag,
			AccountPasswordFlag,
		},
		Action: importAccount,
	},
	{
		Category: "Account",
		Name:     "export",
		Usage:    "export all account private keys in hex string",
		Flags: []cli.Flag{
			AccountWalletFlag,
			AccountPasswordFlag,
		},
		Action: exportAccount,
	},
	{
		Category: "Account",
		Name:     "multisigaddr",
		Usage:    "generate multi-signature address",
		Flags: []cli.Flag{
			AccountWalletFlag,
		},
	},
}

func createAccount(c *cli.Context) error {
	walletPath := c.String("wallet")
	pwd := c.String("password")
	if err := createWallet(walletPath, pwd); err != nil {
		fmt.Println("error: create wallet failed,", err)
		cli.ShowCommandHelpAndExit(c, "create", 1)
	}
	return nil
}

func accountInfo(c *cli.Context) error {
	walletPath := c.String("wallet")
	if exist := cmdcom.FileExisted(walletPath); !exist {
		fmt.Println(fmt.Sprintf("error: %s is not found.", walletPath))
		cli.ShowCommandHelpAndExit(c, "account", 1)
	}
	password, err := cmdcom.GetPassword()
	if err != nil {
		return err
	}
	client, err := account.Open(walletPath, password)
	if err != nil {
		return err
	}
	if err := ShowAccountInfo(client); err != nil {
		fmt.Println("error: show account info failed,", err)
		cli.ShowCommandHelpAndExit(c, "account", 1)
	}
	return nil
}

func listAccount(c *cli.Context) error {
	walletPath := c.String("wallet")
	if exist := cmdcom.FileExisted(walletPath); !exist {
		fmt.Println(fmt.Sprintf("error: %s is not found.", walletPath))
		cli.ShowCommandHelpAndExit(c, "account", 1)
	}
	if err := ShowAccountBalance(walletPath); err != nil {
		fmt.Println("error: list accounts information failed,", err)
		cli.ShowCommandHelpAndExit(c, "list", 1)
	}
	return nil
}

func addAccount(c *cli.Context) error {
	// todo
	return nil
}

func changePassword(c *cli.Context) error {
	// todo
	return nil
}

func importAccount(c *cli.Context) error {
	walletPath := c.String("wallet")
	pwdHex := c.String("password")

	if c.NArg() < 1 {
		cmdcom.PrintErrorMsg("Missing argument. PrivateKey hex expected.")
		cli.ShowCommandHelpAndExit(c, "import", 1)
	}
	privateKeyHex := c.Args().First()

	privateKeyBytes, err := hex.DecodeString(privateKeyHex)
	if err != nil {
		return err
	}

	var pwd []byte
	if pwdHex == "" {
		pwd, err = cmdcom.GetConfirmedPassword()
		if err != nil {
			return err
		}
	} else {
		pwd = []byte(pwdHex)
	}

	var client *account.ClientImpl
	if _, err := os.Open(walletPath); os.IsNotExist(err) {
		client = account.NewClient(walletPath, pwd, true)
		if client == nil {
			return errors.New("client nil")
		}
	} else {
		client, err = account.Open(walletPath, pwd)
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

func exportAccount(c *cli.Context) error {
	walletPath := c.String("wallet")
	pwdHex := c.String("password")

	var pwd []byte
	if pwdHex == "" {
		var err error
		pwd, err = cmdcom.GetPassword()
		if err != nil {
			return err
		}
	} else {
		pwd = []byte(pwdHex)
	}

	client, err := account.Open(walletPath, pwd)
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
