package account

import (
	"os"
	"fmt"
	"errors"
	"strings"
	"io/ioutil"

	"SPVWallet/log"
	"SPVWallet/crypto"
	. "SPVWallet/wallet"
	. "SPVWallet/core"
	. "SPVWallet/cli/common"

	"github.com/urfave/cli"
	"SPVWallet/core/transaction"
)

const (
	MinMultiSignKeys = 3
)

func createWallet(password []byte) error {

	var err error
	password, err = GetPassword(password, true)
	if err != nil {
		return err
	}

	_, err = Create(password)
	if err != nil {
		return err
	}
	return showAccountInfo(password)
}

func changePassword(password []byte, wallet Wallet) error {

	// Verify old password
	oldPassword, err := GetPassword(password, false)
	if err != nil {
		return err
	}

	err = wallet.VerifyPassword(oldPassword)
	if err != nil {
		return err
	}

	// Input new password
	fmt.Println("--PLEASE INPUT NEW PASSWORD--")
	newPassword, err := GetPassword(nil, true)
	if err != nil {
		return err
	}

	if err := wallet.ChangePassword(oldPassword, newPassword); err != nil {
		return errors.New("failed to change password")
	}

	fmt.Println("--PASSWORD CHANGED SUCCESSFUL--")

	return nil
}

func showAccountInfo(password []byte) error {

	var err error
	password, err = GetPassword(password, false)
	if err != nil {
		return err
	}

	keyStore, err := OpenKeystore(password)
	if err != nil {
		return err
	}

	// print header
	fmt.Printf("%5s %34s %66s %6s\n", "INDEX", "ADDRESS", "PUBLIC KEY", "TYPE")
	fmt.Println("-----", strings.Repeat("-", 34), strings.Repeat("-", 66), "------")

	// print accounts
	for i, account := range keyStore.GetAccounts() {
		accountType := "SUB"
		if i == 0 {
			accountType = "MASTER"
		}
		// print content
		publicKey := account.PublicKey()
		publicKeyBytes, _ := publicKey.EncodePoint(true)
		fmt.Printf("%-5d %-34s %-66s %6s\n", i, account.Address(), BytesToHexString(publicKeyBytes), accountType)
		// print divider line
		fmt.Println("-----", strings.Repeat("-", 34), strings.Repeat("-", 66), "------")
	}

	return nil
}

func listBalanceInfo(wallet Wallet) error {
	scripts, err := wallet.GetScripts()
	if err != nil {
		log.Error("Get addresses error:", err)
		return errors.New("get wallet addresses failed")
	}
	// print header
	fmt.Printf("%5s %34s %-32s\n", "INDEX", "ADDRESS", "BALANCE")
	fmt.Println("-----", strings.Repeat("-", 34), strings.Repeat("-", 32))

	for i, script := range scripts {
		balance := Fixed64(0)
		programHash, err := transaction.ToProgramHash(script)
		if err != nil {
			return errors.New("parse address script failed")
		}
		address, _ := programHash.ToAddress()
		UTXOs, err := wallet.GetAddressUTXOs(programHash)
		if err != nil {
			return errors.New("get " + address + " UTXOs failed")
		}
		for _, utxo := range UTXOs {
			balance += utxo.Value
		}

		fmt.Printf("%-5d %34s %-32s\n", i+1, address, balance.String())
		fmt.Println("-----", strings.Repeat("-", 34), strings.Repeat("-", 32))
	}
	return nil
}

func newSubAccount(password []byte, wallet Wallet) error {
	var err error
	password, err = GetPassword(password, false)
	if err != nil {
		return err
	}

	account, err := wallet.NewSubAccount(password)
	if err != nil {
		return err
	}

	address, err := account.ToAddress()
	if err != nil {
		return err
	}

	fmt.Println(address)
	return nil
}

func addMultiSignAccount(context *cli.Context, wallet Wallet, content string) error {
	// Get address content from file or cli input
	publicKeys, err := getPublicKeys(content)
	if err != nil {
		return err
	}

	if len(publicKeys) < MinMultiSignKeys {
		return errors.New(fmt.Sprint("multi sign account require at lest ", MinMultiSignKeys, " public keys"))
	}

	// Get M value
	M := context.Int("m")
	if M == 0 { // Use default M greater than half
		M = len(publicKeys)/2 + 1
	}
	if M < len(publicKeys)/2+1 || M > len(publicKeys) {
		return errors.New("M must be greater than half number of public keys, less than number of public keys")
	}

	programHash, err := wallet.AddMultiSignAccount(M, publicKeys...)
	if err != nil {
		return err
	}

	address, err := programHash.ToAddress()
	if err != nil {
		return err
	}
	fmt.Println(address)
	return nil
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
		keyBytes, err := HexStringToBytes(strings.TrimSpace(v))
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

func resetData(wallet Wallet) error {
	err := wallet.Reset()
	if err != nil {
		return err
	}
	fmt.Println("Wallet data has been reset")
	return listBalanceInfo(wallet)
}

func accountAction(context *cli.Context) {
	if context.NumFlags() == 0 {
		cli.ShowSubcommandHelp(context)
		os.Exit(0)
	}
	pass := context.String("password")

	// create wallet
	if context.Bool("create") {
		if err := createWallet([]byte(pass)); err != nil {
			fmt.Println("error: create wallet failed,", err)
			cli.ShowCommandHelpAndExit(context, "create", 1)
		}
		return
	}

	wallet, err := Open()
	if err != nil {
		fmt.Println("error: open wallet failed, ", err)
		os.Exit(2)
	}

	// list accounts
	if context.Bool("list") {
		if err := showAccountInfo([]byte(pass)); err != nil {
			fmt.Println("error: list accounts info failed, ", err)
			cli.ShowCommandHelpAndExit(context, "list", 3)
		}
		return
	}

	// change password
	if context.Bool("changepassword") {
		if err := changePassword([]byte(pass), wallet); err != nil {
			fmt.Println("error: change password failed, ", err)
			cli.ShowCommandHelpAndExit(context, "changepassword", 4)
		}
		return
	}

	// new sub account
	if context.Bool("newsubaccount") {
		if err := newSubAccount([]byte(pass), wallet); err != nil {
			fmt.Println("error: new sub account failed, ", err)
			cli.ShowCommandHelpAndExit(context, "newsubaccount", 5)
		}
		return
	}

	// add multi sign account
	if pubKeysStr := context.String("createmultisigaccount"); pubKeysStr != "" {
		if err := addMultiSignAccount(context, wallet, pubKeysStr); err != nil {
			fmt.Println("error: add multi sign account failed, ", err)
			cli.ShowCommandHelpAndExit(context, "createmultisigaccount", 5)
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

	// reset all data in blockchain, including headers and transactions
	if context.Bool("reset") {
		if err := resetData(wallet); err != nil {
			fmt.Println("error: reset wallet data failed:", err)
			cli.ShowCommandHelpAndExit(context, "reset", 6)
		}
	}
}

func NewCommand() *cli.Command {
	return &cli.Command{
		Name:      "account",
		ShortName: "a",
		Usage:     "account [command] [args]",
		Description: "create wallet, change password, new sub account, create multisig address\n" +
			"\tdelete account, list address's balances or reset wallet",
		ArgsUsage: "[args]",
		Flags: append(CommonFlags,
			cli.BoolFlag{
				Name:  "create, c",
				Usage: "create wallet",
			},
			cli.BoolFlag{
				Name:  "list, l",
				Usage: "list all accounts",
			},
			cli.BoolFlag{
				Name:  "changepassword",
				Usage: "change wallet password",
			},
			cli.BoolFlag{
				Name:  "newsubaccount",
				Usage: "create a new sub account",
			},
			cli.StringFlag{
				Name: "createmultisigaccount",
				Usage: "add a multi-sign account with other signers public keys\n" +
					"\tuse -m to specify how many signatures are needed to create a valid transaction\n" +
					"\tby default M is public keys / 2 + 1, witch means greater than half",
			},
			cli.IntFlag{
				Name:  "m",
				Usage: "the M value to specify how many signatures are needed to create a valid transaction",
				Value: 0,
			},
			cli.BoolFlag{
				Name:  "balance, b",
				Usage: "list balances",
			},
			cli.BoolFlag{
				Name:  "reset",
				Usage: "clear all data, including blockchain, utxos stxos and transactions",
			},
		),
		Action: accountAction,
		OnUsageError: func(c *cli.Context, err error, subCommand bool) error {
			return cli.NewExitError(err, 1)
		},
	}
}
