package wallet

import (
	"bytes"
	"errors"
	"fmt"
	"io/ioutil"
	"os"
	"strings"

	"github.com/elastos/Elastos.ELA/account"
	cmdcom "github.com/elastos/Elastos.ELA/cmd/common"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/crypto"

	"github.com/elastos/Elastos.ELA.Utility/http/jsonrpc"
	"github.com/elastos/Elastos.ELA.Utility/http/util"
	"github.com/urfave/cli"
)

var txCommand = []cli.Command{
	{
		Category:    "Transaction",
		Name:        "buildtx",
		Usage:       "Build a transaction",
		Description: "use --to --amount --fee to create a transaction",
		Flags: []cli.Flag{
			TransactionFromFlag,
			TransactionToFlag,
			TransactionAmountFlag,
			TransactionFeeFlag,
			//TransactionLockFlag,
			AccountWalletFlag,
		},
		Action: buildTx,
	},
	{
		Category:    "Transaction",
		Name:        "signtx",
		Usage:       "Sign a transaction",
		Description: "use --file or --hex to specify the transaction file path or content",
		Flags: []cli.Flag{
			TransactionHexFlag,
			TransactionFileFlag,
			AccountWalletFlag,
			AccountPasswordFlag,
		},
		Action: signTx,
	},
	{
		Category:    "Transaction",
		Name:        "sendtx",
		Usage:       "Send a transaction",
		Description: "use --file or --hex to specify the transaction file path or content",
		Flags: []cli.Flag{
			TransactionHexFlag,
			TransactionFileFlag,
		},
		Action: sendTx,
	},
	{
		Category: "Transaction",
		Name:     "showtx",
		Usage:    "Show info of raw transaction",
		Flags: []cli.Flag{
			TransactionHexFlag,
			TransactionFileFlag,
		},
		Action: showTx,
	},
}

func getTransactionHex(c *cli.Context) (string, error) {
	// If parameter with file path is not empty, read content from file
	if filePath := strings.TrimSpace(c.String("file")); filePath != "" {

		if _, err := os.Stat(filePath); err != nil {
			return "", errors.New("invalid transaction file path")
		}
		file, err := os.OpenFile(filePath, os.O_RDONLY, 0666)
		if err != nil {
			return "", errors.New("open transaction file failed")
		}
		rawData, err := ioutil.ReadAll(file)
		if err != nil {
			return "", errors.New("read transaction file failed")
		}

		content := strings.TrimSpace(string(rawData))
		// File content can not by empty
		if content == "" {
			return "", errors.New("transaction file is empty")
		}
		return content, nil
	}

	content := strings.TrimSpace(c.String("hex"))
	// Hex string content can not be empty
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

	program := txn.Programs[0]

	haveSign, needSign, err := crypto.GetSignStatus(program.Code, program.Parameter)
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

	haveSign, needSign, _ = crypto.GetSignStatus(program.Code, program.Parameter)
	fmt.Println("[", haveSign, "/", needSign, "] Transaction successfully signed")

	output(haveSign, needSign, txnSigned)

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

	result, err := jsonrpc.CallParams(cmdcom.LocalServer(), "sendrawtransaction", util.Params{"data": txHex})
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
