// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package wallet

import (
	"bytes"
	"errors"
	"fmt"
	"os"
	"strings"

	"github.com/elastos/Elastos.ELA/account"
	cmdcom "github.com/elastos/Elastos.ELA/cmd/common"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/utils/http"

	"github.com/urfave/cli"
)

var txCommand = []cli.Command{
	{
		Category:    "Transaction",
		Name:        "buildtx",
		Usage:       "Build a transaction",
		Description: "use --to --amount --fee to create a transaction",
		Flags: []cli.Flag{
			cmdcom.TransactionFromFlag,
			cmdcom.TransactionToFlag,
			cmdcom.TransactionToManyFlag,
			cmdcom.TransactionAmountFlag,
			cmdcom.TransactionFeeFlag,
			cmdcom.TransactionOutputLockFlag,
			cmdcom.TransactionTxLockFlag,
			cmdcom.AccountWalletFlag,
		},
		Subcommands: buildTxCommand,
		Action:      buildTx,
	},
	{
		Category:    "Transaction",
		Name:        "signtx",
		Usage:       "Sign a transaction",
		Description: "use --file or --hex to specify the transaction file path or content",
		Flags: []cli.Flag{
			cmdcom.TransactionHexFlag,
			cmdcom.TransactionFileFlag,
			cmdcom.AccountWalletFlag,
			cmdcom.AccountPasswordFlag,
		},
		Action: signTx,
	},
	{
		Category:    "Transaction",
		Name:        "sendtx",
		Usage:       "Send a transaction",
		Description: "use --file or --hex to specify the transaction file path or content",
		Flags: []cli.Flag{
			cmdcom.TransactionHexFlag,
			cmdcom.TransactionFileFlag,
		},
		Action: sendTx,
	},
	{
		Category: "Transaction",
		Name:     "showtx",
		Usage:    "Show info of raw transaction",
		Flags: []cli.Flag{
			cmdcom.TransactionHexFlag,
			cmdcom.TransactionFileFlag,
		},
		Action: showTx,
	},
}

var buildTxCommand = []cli.Command{
	{
		Name:  "withdraw",
		Usage: "Build a tx to withdraw crc proposal",
		Flags: []cli.Flag{
			cmdcom.AccountWalletFlag,
			cmdcom.AccountPasswordFlag,
			cmdcom.CRCProposalHashFlag,
			cmdcom.CRCProposalStageFlag,
			cmdcom.TransactionAmountFlag,
			cmdcom.TransactionFeeFlag,
			cmdcom.CRCCommiteeAddrFlag,
			cmdcom.TransactionToFlag,
		},
		Action: func(c *cli.Context) error {
			if err := CreateCRCProposalWithdrawTransaction(c); err != nil {
				fmt.Println("error:", err)
				os.Exit(1)
			}
			return nil
		},
	},
	{
		Name:  "activate",
		Usage: "Build a tx to activate producer which have been inactivated",
		Flags: []cli.Flag{
			cmdcom.TransactionNodePublicKeyFlag,
			cmdcom.AccountWalletFlag,
			cmdcom.AccountPasswordFlag,
		},
		Action: func(c *cli.Context) error {
			if err := CreateActivateProducerTransaction(c); err != nil {
				fmt.Println("error:", err)
				os.Exit(1)
			}
			return nil
		},
	},
	{
		Name:  "vote",
		Usage: "Build a tx to vote for candidates using ELA",
		Flags: []cli.Flag{
			cmdcom.TransactionForFlag,
			cmdcom.TransactionAmountFlag,
			cmdcom.TransactionFromFlag,
			cmdcom.TransactionFeeFlag,
			cmdcom.AccountWalletFlag,
			cmdcom.AccountPasswordFlag,
		},
		Action: func(c *cli.Context) error {
			if c.NumFlags() == 0 {
				cli.ShowSubcommandHelp(c)
				return nil
			}
			if err := CreateVoteTransaction(c); err != nil {
				fmt.Println("error:", err)
				os.Exit(1)
			}
			return nil
		},
	},
	{
		Name:  "crosschain",
		Usage: "Build a cross chain tx",
		Flags: []cli.Flag{
			cmdcom.TransactionSAddressFlag,
			cmdcom.TransactionAmountFlag,
			cmdcom.TransactionFromFlag,
			cmdcom.TransactionToFlag,
			cmdcom.TransactionFeeFlag,
			cmdcom.AccountWalletFlag,
		},
		Action: func(c *cli.Context) error {
			if c.NumFlags() == 0 {
				cli.ShowSubcommandHelp(c)
				return nil
			}
			if err := CreateCrossChainTransaction(c); err != nil {
				fmt.Println("error:", err)
				os.Exit(1)
			}
			return nil
		},
	},
}

func getTransactionHex(c *cli.Context) (string, error) {
	if filePath := strings.TrimSpace(c.String("file")); filePath != "" {
		return cmdcom.ReadFile(filePath)
	}

	content := strings.TrimSpace(c.String("hex"))
	if content == "" {
		return "", errors.New("transaction hex string is empty")
	}

	return content, nil
}

func buildTx(c *cli.Context) error {
	if c.NumFlags() == 0 {
		cli.ShowSubcommandHelp(c)
		return nil
	}
	if err := CreateTransaction(c); err != nil {
		fmt.Println("error:", err)
		os.Exit(1)
	}
	return nil
}

func signTx(c *cli.Context) error {
	if c.NumFlags() == 0 {
		cli.ShowSubcommandHelp(c)
		return nil
	}
	walletPath := c.String("wallet")
	password, err := cmdcom.GetFlagPassword(c)
	if err != nil {
		return err
	}

	client, err := account.Open(walletPath, password)
	if err != nil {
		return err
	}

	txHex, err := getTransactionHex(c)
	if err != nil {
		return err
	}
	rawData, err := common.HexStringToBytes(txHex)
	if err != nil {
		return errors.New("decode transaction content failed")
	}

	var txn types.Transaction
	err = txn.Deserialize(bytes.NewReader(rawData))
	if err != nil {
		return errors.New("deserialize transaction failed")
	}

	if len(txn.Programs) == 0 {
		return errors.New("no program found in transaction")
	}

	haveSign, needSign, err := crypto.GetSignStatus(txn.Programs[0].Code, txn.Programs[0].Parameter)
	if err != nil {
		return err
	}
	if haveSign == needSign {
		return errors.New("transaction was fully signed, no need more sign")
	}

	txnSigned, err := client.Sign(&txn)
	if err != nil {
		return err
	}

	haveSign, needSign, _ = crypto.GetSignStatus(txn.Programs[0].Code, txn.Programs[0].Parameter)
	fmt.Println("[", haveSign, "/", needSign, "] Transaction was successfully signed")

	OutputTx(haveSign, needSign, txnSigned)

	return nil
}

func sendTx(c *cli.Context) error {
	if c.NumFlags() == 0 {
		cli.ShowSubcommandHelp(c)
		return nil
	}

	txHex, err := getTransactionHex(c)
	if err != nil {
		return err
	}

	result, err := cmdcom.RPCCall("sendrawtransaction", http.Params{"data": txHex})
	if err != nil {
		return err
	}
	fmt.Println(result.(string))

	return nil
}

func showTx(c *cli.Context) error {
	if c.NumFlags() == 0 {
		cli.ShowSubcommandHelp(c)
		return nil
	}

	txHex, err := getTransactionHex(c)
	if err != nil {
		return err
	}

	txBytes, err := common.HexStringToBytes(txHex)
	if err != nil {
		return err
	}
	var txn types.Transaction
	if err := txn.Deserialize(bytes.NewReader(txBytes)); err != nil {
		return err
	}

	fmt.Println(txn.String())

	return nil
}
