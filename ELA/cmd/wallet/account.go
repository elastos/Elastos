// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

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
	"github.com/elastos/Elastos.ELA/utils"

	"github.com/urfave/cli"
)

var accountCommand = []cli.Command{
	{
		Category: "Account",
		Name:     "create",
		Aliases:  []string{"c"},
		Usage:    "Create an account",
		Flags: []cli.Flag{
			cmdcom.AccountWalletFlag,
			cmdcom.AccountPasswordFlag,
		},
		Action: createAccount,
	},
	{
		Category: "Account",
		Name:     "account",
		Aliases:  []string{"a"},
		Usage:    "Show account address and public key",
		Flags: []cli.Flag{
			cmdcom.AccountWalletFlag,
			cmdcom.AccountPasswordFlag,
		},
		Action: accountInfo,
	},
	{
		Category: "Account",
		Name:     "balance",
		Aliases:  []string{"b"},
		Usage:    "Check account balance",
		Flags: []cli.Flag{
			cmdcom.AccountWalletFlag,
		},
		Action: accountBalance,
	},
	{
		Category: "Account",
		Name:     "add",
		Usage:    "Add a standard account",
		Flags: []cli.Flag{
			cmdcom.AccountWalletFlag,
			cmdcom.AccountPasswordFlag,
		},
		Action: addAccount,
	},
	{
		Category: "Account",
		Name:     "addmultisig",
		Usage:    "Add a multi-signature account",
		Flags: []cli.Flag{
			cmdcom.AccountWalletFlag,
			cmdcom.AccountPasswordFlag,
			cmdcom.AccountMultiMFlag,
			cmdcom.AccountMultiPubKeyFlag,
		},
		Action: addMultiSigAccount,
	},
	{
		Category: "Account",
		Name:     "delete",
		Usage:    "Delete an account",
		Flags: []cli.Flag{
			cmdcom.AccountWalletFlag,
			cmdcom.AccountPasswordFlag,
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
			cmdcom.AccountWalletFlag,
			cmdcom.AccountPasswordFlag,
		},
		Action: importAccount,
	},
	{
		Category: "Account",
		Name:     "export",
		Usage:    "Export all account private keys in hex string",
		Flags: []cli.Flag{
			cmdcom.AccountWalletFlag,
			cmdcom.AccountPasswordFlag,
		},
		Action: exportAccount,
	},
	{
		Category: "Account",
		Name:     "depositaddr",
		Usage:    "Generate deposit address",
		Action:   generateDepositAddress,
	},
	{
		Category: "Account",
		Name:     "didaddr",
		Usage:    "Generate did address",
		Action:   generateDIDAddress,
	},
	{
		Category: "Account",
		Name:     "crosschainaddr",
		Usage:    "Generate cross chain address",
		Action:   generateCrossChainAddress,
	},
}

func createAccount(c *cli.Context) error {
	walletPath := c.String("wallet")
	password := c.String("password")

	var p []byte
	if password == "" {
		var err error
		p, err = utils.GetConfirmedPassword()
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
	if exist := utils.FileExisted(walletPath); !exist {
		fmt.Println(fmt.Sprintf("error: %s is not found.", walletPath))
		cli.ShowCommandHelpAndExit(c, "account", 1)
	}
	password, err := cmdcom.GetFlagPassword(c)
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
	if exist := utils.FileExisted(walletPath); !exist {
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
	password, err := cmdcom.GetFlagPassword(c)
	if err != nil {
		return err
	}

	client, err := account.Add(walletPath, password)
	if err != nil {
		return err
	}

	return ShowAccountInfo(client)
}

func addMultiSigAccount(c *cli.Context) error {
	walletPath := c.String("wallet")
	password, err := cmdcom.GetFlagPassword(c)
	if err != nil {
		return err
	}
	m := c.Int("m")
	pksStr := c.String("pubkeys")
	pksStr = strings.TrimSpace(strings.Trim(pksStr, ","))

	if pksStr == "" || m == 0 {
		return errors.New("missing arguments. pubkeys or m expected")
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

	account, err := account.AddMultiSig(walletPath, password, m, pubKeys)
	if err != nil {
		return err
	}

	fmt.Println(account.Address)
	return nil
}

func delAccount(c *cli.Context) error {
	walletPath := c.String("wallet")
	password, err := cmdcom.GetFlagPassword(c)
	if err != nil {
		return err
	}
	if c.NArg() < 1 {
		cmdcom.PrintErrorMsg("Missing argument. Account address expected.")
		cli.ShowCommandHelpAndExit(c, "delete", 1)
	}
	address := c.Args().First()
	client, err := account.Open(walletPath, password)
	if err != nil {
		return err
	}

	err = client.DeleteAccountData(address)
	if err != nil {
		return err
	}

	// reopen client after delete
	client, err = account.Open(walletPath, password)
	if err != nil {
		return err
	}
	return ShowAccountInfo(client)
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
	var client *account.Client
	if _, err := os.Open(walletPath); os.IsNotExist(err) {
		// create a keystore file
		if pwdHex == "" {
			pwd, err = utils.GetConfirmedPassword()
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
			pwd, err = utils.GetPassword()
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
	password, err := cmdcom.GetFlagPassword(c)
	if err != nil {
		return err
	}

	client, err := account.Open(walletPath, password)
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

	if contract.GetPrefixType(*programHash) != contract.PrefixStandard {
		return errors.New("standard address expected")
	}

	codeHash := programHash.ToCodeHash()
	depositHash := common.Uint168FromCodeHash(byte(contract.PrefixDeposit), codeHash)
	address, err := depositHash.ToAddress()
	if err != nil {
		return err
	}
	fmt.Println(address)

	return nil
}

func getCode(publicKey string) []byte {
	pkBytes, _ := common.HexStringToBytes(publicKey)
	pk, _ := crypto.DecodePoint(pkBytes)
	redeemScript, _ := contract.CreateStandardRedeemScript(pk)
	return redeemScript
}

func getDID(code []byte) *common.Uint168 {
	didCode := make([]byte, len(code))
	copy(didCode, code)
	didCode = append(didCode[:len(code)-1], common.DID)
	ct1, _ := contract.CreateCRIDContractByCode(didCode)
	return ct1.ToProgramHash()
}

func generateDIDAddress(c *cli.Context) error {
	if c.NArg() < 1 {
		cmdcom.PrintErrorMsg("Missing argument. Standard public key expected.")
		cli.ShowCommandHelpAndExit(c, "didaddress", 1)
	}
	publicKey := c.Args().First()

	var programHash *common.Uint168
	var err error
	var code []byte

	if publicKey == "" {
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
		code = programHash.ToCodeHash().Bytes()

	} else {
		code = getCode(publicKey)
	}

	did := getDID(code)
	didAddress, err := did.ToAddress()
	if err != nil {
		return err
	}
	fmt.Println(didAddress)

	return nil
}

func generateCrossChainAddress(c *cli.Context) error {
	if c.NArg() < 1 {
		cmdcom.PrintErrorMsg("Missing argument. The side chain genesis block hash expected.")
		cli.ShowCommandHelpAndExit(c, "crosschainaddress", 1)
	}
	hashBytes, err := common.HexStringToBytes(c.Args().First())
	if err != nil {
		return err
	}

	hash, err := common.Uint256FromBytes(common.BytesReverse(hashBytes))
	if err != nil {
		return err
	}

	code := contract.CreateCrossChainRedeemScript(*hash)
	address, err := common.ToProgramHash(byte(contract.PrefixCrossChain), code).ToAddress()
	if err != nil {
		return err
	}
	fmt.Println(address)

	return nil
}
