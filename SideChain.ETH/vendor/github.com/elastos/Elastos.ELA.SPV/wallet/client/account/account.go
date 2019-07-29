package account

import (
	"errors"
	"fmt"
	"io/ioutil"
	"os"
	"strings"

	"github.com/elastos/Elastos.ELA.SPV/wallet/client"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"

	"github.com/urfave/cli"
)

const (
	MinMultiSignKeys = 3
)

func listBalanceInfo(wallet *client.Wallet) error {
	addrs, err := wallet.GetAddrs()
	if err != nil {
		return errors.New("get wallet addresses failed")
	}

	return client.ShowAccounts(addrs, nil, wallet)
}

func newSubAccount(password []byte, wallet *client.Wallet) error {
	var err error
	password, err = client.GetPassword(password, false)
	if err != nil {
		return err
	}

	programHash, err := wallet.NewSubAccount(password)
	if err != nil {
		return err
	}

	addrs, err := wallet.GetAddrs()
	if err != nil {
		return errors.New("get wallet addresses failed")
	}

	return client.ShowAccounts(addrs, programHash, wallet)
}

func addMultiSignAccount(context *cli.Context, wallet *client.Wallet, content string) error {
	// Get address content from file or cli input
	publicKeys, err := getPublicKeys(content)
	if err != nil {
		return err
	}

	if len(publicKeys) < MinMultiSignKeys {
		return errors.New(fmt.Sprint("multi sign account require at lest ", MinMultiSignKeys, " public keys"))
	}

	// Get m value
	m := context.Int("m")
	if m == 0 { // Use default m greater than half
		m = len(publicKeys)/2 + 1
	}
	if m < len(publicKeys)/2+1 || m > len(publicKeys) {
		return errors.New("M must be greater than half number of public keys, less than number of public keys")
	}

	programHash, err := wallet.AddMultiSignAccount(m, publicKeys...)
	if err != nil {
		return err
	}

	addrs, err := wallet.GetAddrs()
	if err != nil {
		return errors.New("get wallet addresses failed")
	}

	return client.ShowAccounts(addrs, programHash, wallet)
}

func getPublicKeys(content string) ([]*crypto.PublicKey, error) {
	// Content can not be empty
	if content == "" {
		return nil, errors.New("content should be the public key[s] file path or public key strings separated by comma")
	}

	// Get public key strings
	var publicKeyStrings []string
	if _, err := os.Stat(content); err == nil { // if content is a file

		file, err := os.OpenFile(content, os.O_RDONLY, 0666)
		if err != nil {
			return nil, errors.New("open public key file failed")
		}
		rawData, err := ioutil.ReadAll(file)
		if err != nil {
			return nil, errors.New("read public key file failed")
		}
		publicKeyStrings = strings.Split(strings.TrimSpace(string(rawData)), "\n")
	} else {
		publicKeyStrings = strings.Split(strings.TrimSpace(content), ",")
	}

	// Check if have duplicate public key
	keyMap := map[string]string{}
	for _, publicKeyString := range publicKeyStrings {
		if keyMap[publicKeyString] == "" {
			keyMap[publicKeyString] = publicKeyString
		} else {
			return nil, errors.New(fmt.Sprint("duplicate public key:", publicKeyString))
		}
	}

	// Decode public keys from public key strings
	var publicKeys []*crypto.PublicKey
	for _, v := range publicKeyStrings {
		keyBytes, err := common.HexStringToBytes(strings.TrimSpace(v))
		if err != nil {
			return nil, err
		}
		publicKey, err := crypto.DecodePoint(keyBytes)
		if err != nil {
			return nil, err
		}
		publicKeys = append(publicKeys, publicKey)
	}

	return publicKeys, nil
}

func accountAction(context *cli.Context) {
	if context.NumFlags() == 0 {
		cli.ShowSubcommandHelp(context)
		os.Exit(0)
	}
	pass := context.String("password")

	wallet, err := client.Open()
	if err != nil {
		fmt.Println("error: open wallet failed,", err)
		os.Exit(2)
	}

	// list accounts
	if context.Bool("list") {
		if err := client.ShowAccountInfo([]byte(pass)); err != nil {
			fmt.Println("error: list accounts info failed,", err)
			cli.ShowCommandHelpAndExit(context, "list", 3)
		}
		return
	}

	// new sub account
	if context.Bool("new") {
		if err := newSubAccount([]byte(pass), wallet); err != nil {
			fmt.Println("error: new sub account failed,", err)
			cli.ShowCommandHelpAndExit(context, "new", 5)
		}
		return
	}

	// add multi sign account
	if pubKeysStr := context.String("addmultisig"); pubKeysStr != "" {
		if err := addMultiSignAccount(context, wallet, pubKeysStr); err != nil {
			fmt.Println("error: add multi sign account failed,", err)
			cli.ShowCommandHelpAndExit(context, "addmultisig", 5)
		}
		return
	}

	// show addresses balance in this wallet
	if context.Bool("balance") {
		if err := listBalanceInfo(wallet); err != nil {
			fmt.Println("error: list balance info failed,", err)
			cli.ShowCommandHelpAndExit(context, "balance", 6)
		}
		return
	}
}

func NewCommand() cli.Command {
	return cli.Command{
		Name:        "account",
		ShortName:   "a",
		Usage:       "account [command] [args]",
		Description: "commands to create new sub account or multisig account and show accounts balances",
		ArgsUsage:   "[args]",
		Flags: append(client.CommonFlags,
			cli.BoolFlag{
				Name:  "list, l",
				Usage: "list all accounts, including address, public key and type",
			},
			cli.BoolFlag{
				Name:  "new, n",
				Usage: "create a new sub account",
			},
			cli.StringFlag{
				Name: "addmultisig, multi",
				Usage: "add a multi-sign account with signers public keys\n" +
					"\tuse -m to specify how many signatures are needed to create a valid transaction\n" +
					"\tby default M is public keys / 2 + 1, which means greater than half",
			},
			cli.IntFlag{
				Name:  "m",
				Usage: "the M value to specify how many signatures are needed to create a valid transaction",
				Value: 0,
			},
			cli.BoolFlag{
				Name:  "balance, b",
				Usage: "show accounts balances",
			},
		),
		Action: accountAction,
		OnUsageError: func(c *cli.Context, err error, subCommand bool) error {
			return cli.NewExitError(err, 1)
		},
	}
}
