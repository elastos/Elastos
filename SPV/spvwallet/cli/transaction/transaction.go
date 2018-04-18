package transaction

import (
	"os"
	"fmt"
	"bytes"
	"bufio"
	"errors"
	"strings"
	"strconv"
	"io/ioutil"

	"github.com/elastos/Elastos.ELA.SPV/log"
	. "github.com/elastos/Elastos.ELA.SPV/spvwallet/cli"
	walt "github.com/elastos/Elastos.ELA.SPV/spvwallet"

	. "github.com/elastos/Elastos.ELA.Utility/core"
	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/crypto"
	"github.com/urfave/cli"
)

func CreateTransaction(c *cli.Context, wallet walt.Wallet) error {
	txn, err := createTransaction(c, wallet)
	if err != nil {
		return err
	}
	return output(txn)
}

func createTransaction(c *cli.Context, wallet walt.Wallet) (*Transaction, error) {
	feeStr := c.String("fee")
	if feeStr == "" {
		return nil, errors.New("use --fee to specify transfer fee")
	}

	fee, err := StringToFixed64(feeStr)
	if err != nil {
		return nil, errors.New("invalid transaction fee")
	}

	from := c.String("from")
	if from == "" {
		from, err = SelectAccount(wallet)
		if err != nil {
			return nil, err
		}
	}

	var txn *Transaction

	multiOutput := c.String("file")
	if multiOutput != "" {
		txn, err = createMultiOutputTransaction(c, wallet, multiOutput, from, fee)
		if err != nil {
			return nil, err
		}
		return txn, nil
	}

	to := c.String("to")
	if to == "" {
		return nil, errors.New("use --to to specify receiver address")
	}

	amountStr := c.String("amount")
	if amountStr == "" {
		return nil, errors.New("use --amount to specify transfer amount")
	}

	amount, err := StringToFixed64(amountStr)
	if err != nil {
		return nil, errors.New("invalid transaction amount")
	}

	lockStr := c.String("lock")
	if lockStr == "" {
		txn, err = wallet.CreateTransaction(from, to, amount, fee)
		if err != nil {
			return nil, errors.New("create transaction failed: " + err.Error())
		}
	} else {
		lock, err := strconv.ParseUint(lockStr, 10, 32)
		if err != nil {
			return nil, errors.New("invalid lock height")
		}
		txn, err = wallet.CreateLockedTransaction(from, to, amount, fee, uint32(lock))
		if err != nil {
			return nil, errors.New("create transaction failed: " + err.Error())
		}
	}

	return txn, nil
}

func createMultiOutputTransaction(c *cli.Context, wallet walt.Wallet, path, from string, fee *Fixed64) (*Transaction, error) {
	if _, err := os.Stat(path); err != nil {
		return nil, errors.New("invalid multi output file path")
	}
	file, err := os.OpenFile(path, os.O_RDONLY, 0666)
	if err != nil {
		return nil, errors.New("open multi output file failed")
	}

	scanner := bufio.NewScanner(file)
	var multiOutput []*walt.Transfer
	for scanner.Scan() {
		columns := strings.Split(scanner.Text(), ",")
		if len(columns) < 2 {
			return nil, errors.New(fmt.Sprint("invalid multi output line:", columns))
		}
		amountStr := strings.TrimSpace(columns[1])
		amount, err := StringToFixed64(amountStr)
		if err != nil {
			return nil, errors.New("invalid multi output transaction amount: " + amountStr)
		}
		address := strings.TrimSpace(columns[0])
		multiOutput = append(multiOutput, &walt.Transfer{address, amount})
		log.Trace("Multi output address:", address, ", amount:", amountStr)
	}

	lockStr := c.String("lock")
	var txn *Transaction
	if lockStr == "" {
		txn, err = wallet.CreateMultiOutputTransaction(from, fee, multiOutput...)
		if err != nil {
			return nil, errors.New("create multi output transaction failed: " + err.Error())
		}
	} else {
		lock, err := strconv.ParseUint(lockStr, 10, 32)
		if err != nil {
			return nil, errors.New("invalid lock height")
		}
		txn, err = wallet.CreateLockedMultiOutputTransaction(from, fee, uint32(lock), multiOutput...)
		if err != nil {
			return nil, errors.New("create multi output transaction failed: " + err.Error())
		}
	}

	return txn, nil
}

func SignTransaction(password []byte, context *cli.Context, wallet walt.Wallet) error {
	txn, err := getTransaction(context)
	if err != nil {
		return err
	}

	txn, err = signTransaction(password, wallet, txn)
	if err != nil {
		return err
	}

	return output(txn)
}

func signTransaction(password []byte, wallet walt.Wallet, txn *Transaction) (*Transaction, error) {
	haveSign, needSign, err := crypto.GetSignStatus(txn.Programs[0].Code, txn.Programs[0].Parameter)
	if haveSign == needSign {
		return nil, errors.New("transaction was fully signed, no need more sign")
	}

	password, err = GetPassword(password, false)
	if err != nil {
		return nil, err
	}

	_, err = wallet.Sign(password, txn)
	if err != nil {
		return nil, err
	}

	return txn, nil
}

func SendTransaction(password []byte, context *cli.Context, wallet walt.Wallet) error {
	content, err := getContent(context)

	var txn *Transaction
	if content == nil {
		// Create transaction with command line arguments
		txn, err = createTransaction(context, wallet)
		if err != nil {
			return err
		}
		// Sign transaction
		txn, err = signTransaction(password, wallet, txn)
		if err != nil {
			return err
		}
	}

	err = wallet.SendTransaction(txn)
	if err != nil {
		return err
	}

	// Return reversed hex string
	fmt.Println(BytesToHexString(BytesReverse(txn.Hash().Bytes())))
	return nil
}

func getContent(context *cli.Context) (*string, error) {
	var content string
	// If parameter with file path is not empty, read content from file
	if filePath := strings.TrimSpace(context.String("file")); filePath != "" {

		if _, err := os.Stat(filePath); err != nil {
			return nil, errors.New("invalid transaction file path")
		}
		file, err := os.OpenFile(filePath, os.O_RDONLY, 0666)
		if err != nil {
			return nil, errors.New("open transaction file failed")
		}
		rawData, err := ioutil.ReadAll(file)
		if err != nil {
			return nil, errors.New("read transaction file failed")
		}

		content = strings.TrimSpace(string(rawData))
		// File content can not by empty
		if content == "" {
			return nil, errors.New("transaction file is empty")
		}
	} else {
		content = strings.TrimSpace(context.String("hex"))
		// Hex string content can not be empty
		if content == "" {
			return nil, errors.New("transaction hex string is empty")
		}
	}
	return &content, nil
}

func getTransaction(context *cli.Context) (*Transaction, error) {
	content, err := getContent(context)
	if err != nil {
		return nil, err
	}

	rawData, err := HexStringToBytes(*content)
	if err != nil {
		return nil, errors.New("decode transaction content failed")
	}

	var txn Transaction
	err = txn.Deserialize(bytes.NewReader(rawData))
	if err != nil {
		return nil, errors.New("deserialize transaction failed")
	}

	return &txn, nil
}

func output(txn *Transaction) error {
	// Serialise transaction content
	buf := new(bytes.Buffer)
	txn.Serialize(buf)
	content := BytesToHexString(buf.Bytes())

	// Print transaction hex string content to console
	fmt.Println(content)

	// Output to file
	fileName := "to_be_signed" // Create transaction file name

	haveSign, needSign, _ := crypto.GetSignStatus(txn.Programs[0].Code, txn.Programs[0].Parameter)

	if needSign > haveSign {
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

	// Print output message to console
	fmt.Println("[", haveSign, "/", needSign, "] Transaction successfully signed, file:", fileName)

	return nil
}

func transactionAction(context *cli.Context) {
	if context.NumFlags() == 0 {
		cli.ShowSubcommandHelp(context)
		os.Exit(0)
	}
	pass := context.String("password")

	wallet, err := walt.Open()
	if err != nil {
		fmt.Println("error: open wallet failed, ", err)
		os.Exit(2)
	}

	// create transaction
	if context.Bool("create") {
		if err := CreateTransaction(context, wallet); err != nil {
			fmt.Println("error:", err)
			cli.ShowCommandHelpAndExit(context, "create", 701)
		}
	}

	// sign transaction
	if context.Bool("sign") {
		if err := SignTransaction([]byte(pass), context, wallet); err != nil {
			fmt.Println("error:", err)
			cli.ShowCommandHelpAndExit(context, "sign", 702)
		}
	}

	// send transaction
	if context.Bool("send") {
		if err := SendTransaction([]byte(pass), context, wallet); err != nil {
			fmt.Println("error:", err)
			cli.ShowCommandHelpAndExit(context, "send", 703)

		}
	}
}

func NewCommand() cli.Command {
	return cli.Command{
		Name:        "transaction",
		ShortName:   "tx",
		Usage:       "use [--create, --sign, --send], to create, sign or send a transaction",
		Description: "create, sign or send transaction",
		ArgsUsage:   "[args]",
		Flags: append(CommonFlags,
			cli.BoolFlag{
				Name: "create",
				Usage: "use [--from] --to --amount --fee [--lock], or [--from] --file --fee [--lock]\n" +
					"\tto create a standard transaction, or multi output transaction",
			},
			cli.BoolFlag{
				Name:  "sign",
				Usage: "use --hex or --file to pass the transaction content or transaction file path",
			},
			cli.BoolFlag{
				Name: "send",
				Usage: "use --hex or --file to pass the transaction content or transaction file path\n" +
					"\tor use [--from] --to --amount --fee [--lock], or [--from] --file --fee [--lock]\n" +
					"\tto create a standard transaction, or multi output transaction and send it",
			},
			cli.StringFlag{
				Name: "from",
				Usage: "the spend address of the transaction\n" +
					"\tby default this argument is not necessary and the from address will be set to the main account address\n" +
					"\tif you have added mulitsig account, this argument can be used to specify the multisig address you want to use",
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
				Name:  "hex",
				Usage: "the transaction content in hex string format to be signed or sent",
			},
			cli.StringFlag{
				Name: "file",
				Usage: "the file path to specify a CSV format file path with [address,amount] as multi output content\n" +
					"\tor the transaction file path with the hex string content to be signed or sent",
			},
		),
		Action: transactionAction,
		OnUsageError: func(c *cli.Context, err error, subCommand bool) error {
			return cli.NewExitError(err, 1)
		},
	}
}
