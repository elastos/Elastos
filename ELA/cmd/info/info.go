package info

import (
	"bytes"
	"encoding/json"
	"fmt"
	"strconv"

	cmdcom "github.com/elastos/Elastos.ELA/cmd/common"

	"github.com/elastos/Elastos.ELA.Utility/http/jsonrpc"
	"github.com/elastos/Elastos.ELA.Utility/http/util"
	"github.com/urfave/cli"
)

func infoAction(c *cli.Context) error {
	if c.NumFlags() == 0 {
		cli.ShowSubcommandHelp(c)
		return nil
	}

	if c.Bool("connections") {
		result, err := jsonrpc.CallParams(cmdcom.LocalServer(), "getconnectioncount", util.Params{})
		if err != nil {
			fmt.Println("error: get node connections failed,", err)
			return err
		}
		printFormat(result)
		return nil
	}

	if c.Bool("neighbor") {
		result, err := jsonrpc.CallParams(cmdcom.LocalServer(), "getneighbors", util.Params{})
		if err != nil {
			fmt.Println("error: get node neighbors info failed,", err)
			return err
		}
		printFormat(result)
		return nil
	}

	if c.Bool("state") {
		result, err := jsonrpc.CallParams(cmdcom.LocalServer(), "getnodestate", util.Params{})
		if err != nil {
			fmt.Println("error: get node state info failed,", err)
			return err
		}
		printFormat(result)
		return nil
	}

	if c.Bool("currentheight") {
		result, err := jsonrpc.CallParams(cmdcom.LocalServer(), "getcurrentheight", util.Params{})
		if err != nil {
			fmt.Println("error: get block count failed,", err)
			return err
		}
		printFormat(result)
		return nil
	}

	if c.Bool("getbestblockhash") {
		result, err := jsonrpc.CallParams(cmdcom.LocalServer(), "getbestblockhash", util.Params{})
		if err != nil {
			fmt.Println("error: get best block hash failed,", err)
			return err
		}
		printFormat(result)
		return nil
	}

	if index := c.Int64("getblockhash"); index >= 0 {
		result, err := jsonrpc.CallParams(cmdcom.LocalServer(), "getblockhash", util.Params{"height": index})
		if err != nil {
			fmt.Println("error: get block hash failed,", err)
			return err
		}
		printFormat(result)
		return nil
	}

	if param := c.String("getblock"); param != "" {
		index, err := strconv.ParseInt(param, 10, 64)
		if err == nil {
			result, err := jsonrpc.CallParams(cmdcom.LocalServer(), "getblockhash", util.Params{"height": index})
			if err != nil {
				fmt.Println("error: get block failed,", err)
				return err
			}
			param = result.(string)
		}
		result, err := jsonrpc.CallParams(cmdcom.LocalServer(), "getblock", util.Params{"blockhash": param, "verbosity": 2})
		if err != nil {
			fmt.Println("error: get block failed,", err)
			return err
		}
		printFormat(result)
		return nil
	}

	if param := c.String("gettransaction"); param != "" {
		result, err := jsonrpc.CallParams(cmdcom.LocalServer(), "getrawtransaction", util.Params{"txid": param})
		if err != nil {
			fmt.Println("error: get transaction failed,", err)
			return err
		}
		printFormat(result)
		return nil
	}

	if c.Bool("showtxpool") {
		result, err := jsonrpc.CallParams(cmdcom.LocalServer(), "getrawmempool", util.Params{})
		if err != nil {
			fmt.Println("error: get transaction pool failed,", err)
			return err
		}
		printFormat(result)
		return nil
	}

	return nil
}

func printFormat(data interface{}) {
	dataBytes, err := json.Marshal(data)
	if err != nil {
		fmt.Println(err)
		return
	}

	buf := new(bytes.Buffer)
	json.Indent(buf, dataBytes, "", "    ")
	fmt.Println(string(buf.Bytes()))
}

func NewCommand() *cli.Command {
	return &cli.Command{
		Name:        "info",
		Usage:       "show node information",
		Description: "With ela-cli info, you could look up node status, query blocks, transactions, etc.",
		ArgsUsage:   "[args]",
		Flags: []cli.Flag{
			cli.BoolFlag{
				Name:  "connections",
				Usage: "see how many peers are connected with current node",
			},
			cli.BoolFlag{
				Name:  "neighbor, nbr",
				Usage: "show neighbor nodes information",
			},
			cli.BoolFlag{
				Name:  "state",
				Usage: "show current node status",
			},
			cli.BoolFlag{
				Name:  "currentheight, height",
				Usage: "show blockchain height on current node",
			},
			cli.BoolFlag{
				Name:  "getbestblockhash",
				Usage: "show best block hash",
			},
			cli.Int64Flag{
				Name:  "getblockhash, blockh",
				Usage: "query a block's hash with it's height",
				Value: -1,
			},
			cli.StringFlag{
				Name:  "getblock, block",
				Usage: "query a block with height or it's hash",
			},
			cli.StringFlag{
				Name:  "gettransaction, tx",
				Usage: "query a transaction with it's hash",
			},
			cli.BoolFlag{
				Name:  "showtxpool, txpool",
				Usage: "show transactions in node's transaction pool",
			},
		},
		Action: infoAction,
		OnUsageError: func(c *cli.Context, err error, isSubcommand bool) error {
			return cli.NewExitError(err, 1)
		},
	}
}
