package common

import (
	"errors"

	"github.com/elastos/Elastos.ELA/account"
	"github.com/elastos/Elastos.ELA/utils"

	"github.com/urfave/cli"
)

var (
	// Account flags
	AccountWalletFlag = cli.StringFlag{
		Name:  "wallet, w",
		Usage: "wallet `<file>` path",
		Value: account.KeystoreFileName,
	}
	AccountPasswordFlag = cli.StringFlag{
		Name:  "password, p",
		Usage: "wallet password",
	}
	AccountMultiMFlag = cli.IntFlag{
		Name:  "m",
		Usage: "min signature `<number>` of multi signature address",
	}
	AccountMultiPubKeyFlag = cli.StringFlag{
		Name:  "pubkeys, pks",
		Usage: "public key list of multi signature address, separate public keys with comma `,`",
	}

	// Transaction flags
	TransactionFromFlag = cli.StringFlag{
		Name:  "from",
		Usage: "the sender `<address>` of the transaction",
	}
	TransactionToFlag = cli.StringFlag{
		Name:  "to",
		Usage: "the recipient `<address>` of the transaction",
	}
	TransactionToManyFlag = cli.StringFlag{
		Name:  "tomany",
		Usage: "the `<file>` path that contains multi-recipients and amount",
	}
	TransactionAmountFlag = cli.StringFlag{
		Name:  "amount",
		Usage: "the transfer `<amount>` of the transaction",
	}
	TransactionFeeFlag = cli.StringFlag{
		Name:  "fee",
		Usage: "the transfer `<fee>` of the transaction",
	}
	TransactionLockFlag = cli.StringFlag{
		Name:  "lock",
		Usage: "the `<lock time>` to specify when the received asset can be spent",
	}
	TransactionHexFlag = cli.StringFlag{
		Name:  "hex",
		Usage: "the transaction content in hex string format to be sign or send",
	}
	TransactionFileFlag = cli.StringFlag{
		Name:  "file, f",
		Usage: "the file path to specify a transaction file path with the hex string content to be sign",
	}
	TransactionNodePublicKeyFlag = cli.StringFlag{
		Name:  "nodepublickey",
		Usage: "the node public key of an arbitrator which have been inactivated",
	}
	TransactionForFlag = cli.StringFlag{
		Name:  "for",
		Usage: "the `<file>` path that holds the list of candidates",
	}

	// RPC flags
	RPCUserFlag = cli.StringFlag{
		Name:  "rpcuser",
		Usage: "username for JSON-RPC connections",
	}
	RPCPasswordFlag = cli.StringFlag{
		Name:  "rpcpassword",
		Usage: "password for JSON-RPC connections",
	}
	RPCPortFlag = cli.StringFlag{
		Name:  "rpcport",
		Usage: "JSON-RPC server listening port `<number>`",
	}

	// Info flags
	InfoStartFlag = cli.IntFlag{
		Name:  "start",
		Usage: "the start index of producers",
		Value: 0,
	}
	InfoLimitFlag = cli.Int64Flag{
		Name:  "limit",
		Usage: "the limit count of producers",
		Value: -1,
	}
	InfoProducerStateFlag = cli.StringFlag{
		Name:  "state",
		Usage: "the producer state you want",
	}

	// Config flags
	ConfigFileFlag = cli.StringFlag{
		Name:  "conf",
		Usage: "config `<file>` path, ",
		Value: defaultConfigPath,
	}
	DataDirFlag = cli.StringFlag{
		Name:  "datadir",
		Usage: "block data and logs storage `<path>`",
		Value: defaultDataDir,
	}
)

// MoveRPCFlags finds the rpc argument and moves it to the front
// of the argument array.
func MoveRPCFlags(args []string) ([]string, error) {
	newArgs := args[:1]
	cacheArgs := make([]string, 0)

	for i := 1; i < len(args); i++ {
		switch args[i] {
		case "--rpcport":
			fallthrough
		case "--rpcuser":
			fallthrough
		case "--rpcpassword":
			newArgs = append(newArgs, args[i])
			if i == len(args)-1 {
				return nil, errors.New("invalid flag " + args[i])
			}
			newArgs = append(newArgs, args[i+1])
			i++
		default:
			cacheArgs = append(cacheArgs, args[i])
		}
	}

	newArgs = append(newArgs, cacheArgs...)
	return newArgs, nil
}

// GetFlagPassword gets node's wallet password from command line or user input
func GetFlagPassword(c *cli.Context) ([]byte, error) {
	flagPassword := c.String("password")
	password := []byte(flagPassword)
	if flagPassword == "" {
		return utils.GetPassword()
	}

	return password, nil
}
