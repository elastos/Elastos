package wallet

import (
	"fmt"
	"os"

	"github.com/elastos/Elastos.ELA/account"
	cmdcom "github.com/elastos/Elastos.ELA/cmd/common"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"

	"github.com/urfave/cli"
)

func getConfirmedPassword(passwd string) []byte {
	var tmp []byte
	var err error
	if passwd != "" {
		tmp = []byte(passwd)
	} else {
		tmp, err = cmdcom.GetConfirmedPassword()
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
		p, err = cmdcom.GetConfirmedPassword()
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

	// generate deposit address
	if context.Bool("getdepositaddress") {
		addr := context.String("address")
		address, err := generateDepositAddress(addr, "keystore.dat")
		if err != nil {
			fmt.Println("error: get deposit address failed,", err)
			cli.ShowCommandHelpAndExit(context, "getdepositaddress", 1)
		}
		fmt.Println(address)
	}

	return nil
}

func NewCommand() *cli.Command {
	var subCommands []cli.Command
	subCommands = append(subCommands, txCommand...)
	subCommands = append(subCommands, accountCommand...)

	return &cli.Command{
		Name:        "wallet",
		Usage:       "Wallet operations",
		Description: "With ela-cli wallet, you could control your asset.",
		ArgsUsage:   "[args]",
		Flags: []cli.Flag{
			cli.BoolFlag{
				Name:  "getdepositaddress, gda",
				Usage: "generate the deposit address form main account",
			},
			cli.StringFlag{
				Name:  "address",
				Usage: "source standard address",
			},
		},
		Action:      walletAction,
		Subcommands: subCommands,
	}
}
