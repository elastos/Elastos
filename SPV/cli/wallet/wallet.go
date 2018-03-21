package wallet

import (
	"fmt"
	. "SPVWallet/wallet"
	. "SPVWallet/cli/common"

	"github.com/urfave/cli"
)

func createWallet(context *cli.Context) {
	password := []byte(context.String("password"))

	var err error
	password, err = GetPassword(password, true)
	if err != nil {
		fmt.Println("--GET PASSWORD FAILED--")
		return
	}

	_, err = Create(password)
	if err != nil {
		fmt.Println("--CREAT WALLET FAILED--")
		return
	}

	ShowAccountInfo(password)
}

func changePassword(context *cli.Context) {
	password := []byte(context.String("password"))

	// Verify old password
	oldPassword, err := GetPassword(password, false)
	if err != nil {
		fmt.Println("--GET PASSWORD FAILED--")
		return
	}

	wallet, err := Open()
	if err != nil {
		fmt.Println("--OPEN WALLET FAILED--")
		return
	}

	err = wallet.VerifyPassword(oldPassword)
	if err != nil {
		fmt.Println("--PASSWORD WRONG--")
		return
	}

	// Input new password
	fmt.Println("--PLEASE INPUT NEW PASSWORD--")
	newPassword, err := GetPassword(nil, true)
	if err != nil {
		fmt.Println("--GET NEW PASSWROD FAILED--")
		return
	}

	if err := wallet.ChangePassword(oldPassword, newPassword); err != nil {
		fmt.Println("--CHANGED WALLET PASSWROD FAILED--")
		return
	}

	fmt.Println("--PASSWORD CHANGED SUCCESSFUL--")
}

func resetDatabase(context *cli.Context) {
	password := []byte(context.String("password"))

	// Verify old password
	oldPassword, err := GetPassword(password, false)
	if err != nil {
		fmt.Println("--GET PASSWORD FAILED--")
		return
	}

	wallet, err := Open()
	if err != nil {
		fmt.Println("--OPEN WALLET FAILED--")
		return
	}

	err = wallet.VerifyPassword(oldPassword)
	if err != nil {
		fmt.Println("--PASSWORD WRONG--")
		return
	}

	err = wallet.Reset()
	if err != nil {
		fmt.Println("--WALLET DATABASE RESET FAILED--")
		return
	}

	fmt.Println("--WALLET DATABASE HAS BEEN RESET--")
}

func NewCreateCommand() cli.Command {
	return cli.Command{
		Name:   "create",
		Usage:  "create wallet",
		Flags:  append(CommonFlags),
		Action: createWallet,
		OnUsageError: func(c *cli.Context, err error, subCommand bool) error {
			return cli.NewExitError(err, 1)
		},
	}
}

func NewChangePasswordCommand() cli.Command {
	return cli.Command{
		Name:   "changepassword",
		Usage:  "change wallet password",
		Flags:  append(CommonFlags),
		Action: changePassword,
		OnUsageError: func(c *cli.Context, err error, subCommand bool) error {
			return cli.NewExitError(err, 1)
		},
	}
}

func NewResetCommand() cli.Command {
	return cli.Command{
		Name:   "reset",
		Usage:  "reset wallet database including transactions, utxos and stxos",
		Flags:  append(CommonFlags),
		Action: resetDatabase,
		OnUsageError: func(c *cli.Context, err error, subCommand bool) error {
			return cli.NewExitError(err, 1)
		},
	}
}
