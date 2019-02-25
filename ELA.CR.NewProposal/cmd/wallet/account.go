package wallet

import (
	"encoding/hex"
	"errors"
	"fmt"
	"os"
	"strings"

	"github.com/elastos/Elastos.ELA/account"
	cmdcom "github.com/elastos/Elastos.ELA/cmd/common"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/crypto"

	"github.com/urfave/cli"
)

var accountCommand = []cli.Command{
	{
		Category: "Account",
		Name:     "create",
		Aliases:  []string{"c"},
		Usage:    "Create a account",
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
		Usage:    "Show account address and public key",
		Flags: []cli.Flag{
			AccountWalletFlag,
		},
		Action: accountInfo,
	},
	{
		Category: "Account",
		Name:     "balance",
		Aliases:  []string{"b"},
		Usage:    "Check account balance",
		Flags: []cli.Flag{
			AccountWalletFlag,
		},
		Action: accountBalance,
	},
	{
		Category: "Account",
		Name:     "add",
		Usage:    "Add a standard account",
		Flags: []cli.Flag{
			AccountWalletFlag,
			AccountPasswordFlag,
		},
		Action: addAccount,
	},
	{
		Category: "Account",
		Name:     "addmultisig",
		Usage:    "Add a multi-signature account",
		Flags: []cli.Flag{
			AccountWalletFlag,
			AccountPasswordFlag,
			AccountMultiMFlag,
			AccountMultiPubKeyFlag,
		},
		Action: addMultiSigAccount,
	},
	{
		Category: "Account",
		Name:     "delete",
		Usage:    "Delete an account",
		Flags: []cli.Flag{
			AccountWalletFlag,
			AccountPasswordFlag,
		},
		Action: delAccount,
	},
	//{
	//	Category: "Account",
	//	Name:     "chpwd",
	//	Usage:    "Change wallet password",
	//	Action:   changePassword,
	//},
	{
		Category:  "Account",
		Name:      "import",
		Usage:     "Import an account by private key hex string",
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
		Usage:    "Export all account private keys in hex string",
		Flags: []cli.Flag{
			AccountWalletFlag,
			AccountPasswordFlag,
		},
		Action: exportAccount,
	},
	{
		Category: "Account",
		Name:     "depositaddr",
		Usage:    "Generate deposit address",
		Action:   generateDepositAddress,
	},
}

func createAccount(c *cli.Context) error {
	walletPath := c.String("wallet")
	password := c.String("password")

	var p []byte
	if password == "" {
		var err error
		p, err = cmdcom.GetConfirmedPassword()
		if err != nil {
			return err
		}
	} else {
		p = []byte(password)
	}

	client, err := account.Create(walletPath, p)
	if err != nil {
		return err
	}

	return ShowAccountInfo(client)
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

func accountBalance(c *cli.Context) error {
	walletPath := c.String("wallet")
	if exist := cmdcom.FileExisted(walletPath); !exist {
		fmt.Println(fmt.Sprintf("error: %s is not found.", walletPath))
		cli.ShowCommandHelpAndExit(c, "account", 1)
	}
	if err := ShowAccountBalance(walletPath); err != nil {
		fmt.Println("error: check account balance failed,", err)
		cli.ShowCommandHelpAndExit(c, "list", 1)
	}
	return nil
}

func addAccount(c *cli.Context) error {
	walletPath := c.String("wallet")
	pwdHex := c.String("password")

	pwd := []byte(pwdHex)
	if pwdHex == "" {
		var err error
		pwd, err = cmdcom.GetPassword()
		if err != nil {
			return err
		}
	}

	client, err := account.Add(walletPath, pwd)
	if err != nil {
		return err
	}

	return ShowAccountInfo(client)
}

func addMultiSigAccount(c *cli.Context) error {
	walletPath := c.String("wallet")
	pwdHex := c.String("password")
	m := c.Int("m")
	pksStr := c.String("pubkeys")
	pksStr = strings.TrimSpace(strings.Trim(pksStr, ","))

	if pksStr == "" || m == 0 {
		return errors.New("missing arguments. pubkeys or m expected")
	}

	pwd := []byte(pwdHex)
	if pwdHex == "" {
		var err error
		pwd, err = cmdcom.GetPassword()
		if err != nil {
			return err
		}
	}
	pks := strings.Split(pksStr, ",")
	pubKeys := make([]*crypto.PublicKey, 0)
	for _, pk := range pks {
		pk = strings.TrimSpace(pk)
		pkBytes, err := common.HexStringToBytes(pk)
		pubKey, err := crypto.DecodePoint(pkBytes)
		if err != nil {
			return err
		}
		pubKeys = append(pubKeys, pubKey)
	}

	account, err := account.AddMultiSig(walletPath, pwd, m, pubKeys)
	if err != nil {
		return err
	}

	fmt.Println(account.Address)
	return nil
}

func delAccount(c *cli.Context) error {
	walletPath := c.String("wallet")
	pwdHex := c.String("password")
	if c.NArg() < 1 {
		cmdcom.PrintErrorMsg("Missing argument. Account address expected.")
		cli.ShowCommandHelpAndExit(c, "delete", 1)
	}
	address := c.Args().First()
	programHash, err := common.Uint168FromAddress(address)
	if err != nil {
		return err
	}

	pwd := []byte(pwdHex)
	if pwdHex == "" {
		var err error
		pwd, err = cmdcom.GetPassword()
		if err != nil {
			return err
		}
	}

	client, err := account.Open(walletPath, pwd)
	if err != nil {
		return err
	}

	err = client.DeleteAccountData(programHash.String())
	if err != nil {
		return err
	}

	// reopen client after delete
	client, err = account.Open(walletPath, pwd)
	if err != nil {
		return err
	}
	return ShowAccountInfo(client)
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

	pwd := []byte(pwdHex)
	var client *account.ClientImpl
	if _, err := os.Open(walletPath); os.IsNotExist(err) {
		// create a keystore file
		if pwdHex == "" {
			pwd, err = cmdcom.GetConfirmedPassword()
			if err != nil {
				return err
			}
		}
		client = account.NewClient(walletPath, pwd, true)
		if client == nil {
			return errors.New("client nil")
		}
	} else {
		// append to keystore file
		if pwdHex == "" {
			pwd, err = cmdcom.GetPassword()
			if err != nil {
				return err
			}
		}
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

	pwd := []byte(pwdHex)
	if pwdHex == "" {
		var err error
		pwd, err = cmdcom.GetPassword()
		if err != nil {
			return err
		}
	}

	client, err := account.Open(walletPath, pwd)
	if err != nil {
		return err
	}

	accounts := client.GetAccounts()

	fmt.Printf("%-34s %-66s\n", "ADDRESS", "PRIVATE KEY")
	fmt.Println(strings.Repeat("-", 34), strings.Repeat("-", 66))
	for _, account := range accounts {
		prefixType := contract.GetPrefixType(account.ProgramHash)
		if prefixType == contract.PrefixStandard {
			fmt.Printf("%-34s %-66s\n", account.Address, hex.EncodeToString(account.PrivKey()))
			fmt.Println(strings.Repeat("-", 34), strings.Repeat("-", 66))
		}
	}

	return nil
}

func generateDepositAddress(c *cli.Context) error {
	if c.NArg() < 1 {
		cmdcom.PrintErrorMsg("Missing argument. Standard address expected.")
		cli.ShowCommandHelpAndExit(c, "depositaddress", 1)
	}
	addr := c.Args().First()

	var programHash *common.Uint168
	var err error
	if addr == "" {
		mainAccount, err := account.GetWalletMainAccountData(account.KeystoreFileName)
		if err != nil {
			return err
		}
		p, err := common.HexStringToBytes(mainAccount.ProgramHash)
		if err != nil {
			return err
		}
		programHash, err = common.Uint168FromBytes(p)
		if err != nil {
			return err
		}
	} else {
		programHash, err = common.Uint168FromAddress(addr)
		if err != nil {
			return err
		}
	}

	codeHash := programHash.ToCodeHash()
	depositHash := common.Uint168FromCodeHash(byte(contract.PrefixDeposit), codeHash)
	address, err := depositHash.ToAddress()
	if err != nil {
		return nil
	}
	fmt.Println(address)

	return nil
}
