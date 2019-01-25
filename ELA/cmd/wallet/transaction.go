package wallet

import (
	"bytes"
	"encoding/hex"
	"errors"
	"fmt"
	"os"

	"github.com/elastos/Elastos.ELA/account"
	clicom "github.com/elastos/Elastos.ELA/cmd/common"
	"github.com/elastos/Elastos.ELA/cmd/password"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/crypto"

	"github.com/elastos/Elastos.ELA.Utility/http/jsonrpc"
	"github.com/elastos/Elastos.ELA.Utility/http/util"
	"github.com/urfave/cli"
)

func signTransaction(context *cli.Context, client *account.ClientImpl) error {
	content, err := getTransactionContent(context)
	if err != nil {
		return err
	}
	rawData, err := common.HexStringToBytes(content)
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

func SendTransaction(context *cli.Context) error {
	content, err := getTransactionContent(context)
	if err != nil {
		return err
	}

	result, err := jsonrpc.CallParams(clicom.LocalServer(), "sendrawtransaction", util.Params{
		"data": content,
	})
	if err != nil {
		return err
	}
	fmt.Println(result.(string))
	return nil
}

func output(haveSign, needSign int, txn *types.Transaction) error {
	// Serialise transaction content
	buf := new(bytes.Buffer)
	err := txn.Serialize(buf)
	if err != nil {
		fmt.Println("serialize error", err)
	}
	content := common.BytesToHexString(buf.Bytes())

	// Print transaction hex string content to console
	fmt.Println(content)

	// Output to file
	fileName := "to_be_signed" // Create transaction file name

	if haveSign == 0 {
		//	Transaction created do nothing
	} else if needSign > haveSign {
		fileName = fmt.Sprint(fileName, "_", haveSign, "_of_", needSign)
	} else if needSign == haveSign {
		fileName = "ready_to_send"
	}
	fileName = fileName + ".txn"

	file, err := os.OpenFile(fileName, os.O_CREATE|os.O_WRONLY|os.O_TRUNC, 0666)
	if err != nil {
		return err
	}

	_, err = file.Write([]byte(content))
	if err != nil {
		return err
	}

	var tx types.Transaction
	txBytes, _ := hex.DecodeString(content)
	tx.Deserialize(bytes.NewReader(txBytes))

	// Print output file to console
	fmt.Println("File: ", fileName)

	return nil
}

func NewTransactionCommand() *cli.Command {
	return &cli.Command{
		Name:      "transaction",
		Aliases:   []string{"t"},
		Usage:     "user transaction operation",
		ArgsUsage: "[args]",
		Subcommands: []cli.Command{
			{
				Name:        "create",
				Usage:       "create a transaction",
				Description: "use --to --amount --fee [--lock], or --file --fee [--lock] to create a transaction",
				Flags: []cli.Flag{
					cli.StringFlag{
						Name:  "from",
						Usage: "the spend address of the transaction",
					},
					cli.StringFlag{
						Name:  "to",
						Usage: "the receive address of the transaction",
					},
					cli.StringFlag{
						Name:  "amount",
						Usage: "the transfer amount of the transaction",
					},
					cli.StringFlag{
						Name:  "fee",
						Usage: "the transfer fee of the transaction",
					},
					cli.StringFlag{
						Name:  "lock",
						Usage: "the lock time to specify when the received asset can be spent",
					},
					cli.StringFlag{
						Name:  "name, n",
						Usage: "wallet name",
						Value: account.KeystoreFileName,
					},
				},
				Action: func(c *cli.Context) error {
					if c.NumFlags() == 0 {
						cli.ShowSubcommandHelp(c)
						return nil
					}
					if err := createTransaction(c); err != nil {
						fmt.Println("error:", err)
						os.Exit(1)
					}
					return nil
				},
			},
			{
				Name:        "sign",
				Usage:       "sign a transaction",
				Description: "use --file or --hex to specify the transaction file path or content",
				Flags: []cli.Flag{
					cli.StringFlag{
						Name:  "hex",
						Usage: "the transaction content in hex string format to be sign or send",
					},
					cli.StringFlag{
						Name:  "file, f",
						Usage: "the file path to specify a transaction file path with the hex string content to be sign",
					},
					cli.StringFlag{
						Name:  "name, n",
						Usage: "wallet name",
						Value: account.KeystoreFileName,
					},
				},
				Action: func(c *cli.Context) error {
					if c.NumFlags() == 0 {
						cli.ShowSubcommandHelp(c)
						return nil
					}
					name := c.String("name")
					password, err := password.GetPassword()
					if err != nil {
						return err
					}
					client, err := account.Open(name, password)
					if err != nil {
						return err
					}
					if err := signTransaction(c, client); err != nil {
						fmt.Println("error:", err)
						os.Exit(1)
					}
					return nil
				},
			},
			{
				Name:        "send",
				Usage:       "send a transaction",
				Description: "use --file or --hex to specify the transaction file path or content",
				Flags: []cli.Flag{
					cli.StringFlag{
						Name:  "hex",
						Usage: "the transaction content in hex string format to be sign or send",
					},
					cli.StringFlag{
						Name:  "file, f",
						Usage: "the file path to specify a transaction file path with the hex string content to be send",
					},
				},
				Action: func(c *cli.Context) error {
					if c.NumFlags() == 0 {
						cli.ShowSubcommandHelp(c)
						return nil
					}
					if err := SendTransaction(c); err != nil {
						fmt.Println("error:", err)
						os.Exit(1)
					}
					return nil
				},
			},
		},
		OnUsageError: func(c *cli.Context, err error, isSubcommand bool) error {
			clicom.PrintError(c, err, "transaction")
			return cli.NewExitError("", 1)
		},
	}
}
