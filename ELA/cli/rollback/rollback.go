package rollback

import (
	"bytes"
	"errors"
	"fmt"
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/urfave/cli"
	"path/filepath"
	"strconv"

	cliCommon "github.com/elastos/Elastos.ELA/cli/common"
	"github.com/elastos/Elastos.ELA/common/config"
)

func NewCommand() *cli.Command {
	return &cli.Command{
		Name:        "rollback",
		Usage:       "rollback blockchain data",
		Description: "With ela-cli rollback command, you could rollback blockchain data.",
		ArgsUsage:   "[args]",
		Flags: []cli.Flag{
			cli.IntFlag{
				Name:  "height",
				Usage: "the final height after rollback",
			},
		},
		Action: rollbackBlockchain,
		OnUsageError: func(c *cli.Context, err error, isSubcommand bool) error {
			cliCommon.PrintError(c, err, "rollback")
			return cli.NewExitError("", 1)
		},
	}
}

func rollbackBlockchain(context *cli.Context) error {
	targetHeight, err := strconv.Atoi(context.Args().Get(0))
	if err != nil {
		fmt.Println("get height error:", err)
		return err
	}
	store, err := blockchain.NewLevelDB(filepath.Join(config.DataPath, config.DataDir, config.ChainDir))
	if err != nil {
		fmt.Println("connect leveldb failed! Please check wether there is already a ela process running.", err)
	}

	chain := blockchain.ChainStore{IStore: store}
	data, _ := store.Get([]byte{byte(blockchain.SYSCurrentBlock)})
	currentHeight, _ := common.ReadUint32(bytes.NewReader(data[32:]))
	if targetHeight >= int(currentHeight) {
		errorStr := fmt.Sprintf("Current height of blockchain is %d, you can't do this, man.", currentHeight)
		fmt.Println(errorStr)
		return errors.New(errorStr)
	}

	for i := currentHeight; int(i) > targetHeight; i-- {
		fmt.Println("current height is", i)
		blockHash, _ := chain.GetBlockHash(i)
		block, _ := chain.GetBlock(blockHash)
		fmt.Println("blockhash before rollback:", blockHash.String())

		chain.NewBatch()
		chain.RollbackTrimmedBlock(block)
		chain.RollbackBlockHash(block)
		chain.RollbackTransactions(block)
		chain.RollbackUnspendUTXOs(block)
		chain.RollbackUnspend(block)
		chain.RollbackCurrentBlock(block)
		chain.RollbackConfirm(block)
		chain.BatchCommit()

		blockHashAfter, _ := chain.GetBlockHash(i)
		fmt.Println("blockhash after rollback:", blockHashAfter.String())
	}

	return nil
}
