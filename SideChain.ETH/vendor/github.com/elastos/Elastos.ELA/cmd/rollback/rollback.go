package rollback

import (
	"bytes"
	"errors"
	"fmt"
	"strconv"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"

	"github.com/urfave/cli"
)

func NewCommand() *cli.Command {
	return &cli.Command{
		Name:        "rollback",
		Usage:       "Rollback blockchain data",
		Description: "With ela-cli rollback command, you could rollback blockchain data.",
		ArgsUsage:   "[args]",
		Flags: []cli.Flag{
			cli.IntFlag{
				Name:  "height",
				Usage: "the final height after rollback",
			},
		},
		Action: rollbackAction,
	}
}

func rollbackAction(c *cli.Context) error {
	if c.NumFlags() == 0 {
		cli.ShowSubcommandHelp(c)
		return nil
	}
	targetHeightStr := c.String("height")
	targetHeight, err := strconv.Atoi(targetHeightStr)
	if err != nil {
		fmt.Println("get height error:", err)
		return err
	}
	store, err := blockchain.NewLevelDB("elastos/data/chain")
	if err != nil {
		fmt.Println("connect leveldb failed! Please check whether there is already a ela process running.", err)
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
