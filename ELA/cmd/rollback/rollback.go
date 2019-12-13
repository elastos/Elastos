// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package rollback

import (
	"errors"
	"fmt"
	"strconv"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/database"

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

	log.NewDefault("logs/node", 0, 0, 0)
	fdb, err := blockchain.NewChainStoreFFLDB("elastos/data")
	if err != nil {
		return err
	}
	defer fdb.Close()
	nodes := getBlockNodes(fdb)

	store, err := blockchain.NewLevelDB("elastos/data/chain")
	if err != nil {
		fmt.Println("connect leveldb failed! Please check whether there "+
			"is already a ela process running.", err)
	}
	defer store.Close()

	chain := blockchain.ChainStore{IStore: store}
	currentHeight := len(nodes) - 1
	if targetHeight >= int(currentHeight) {
		errorStr := fmt.Sprintf("Current height of blockchain is %d,"+
			" you can't do this, man.", currentHeight)
		fmt.Println(errorStr)
		return errors.New(errorStr)
	}

	for i := currentHeight; int(i) > targetHeight; i-- {
		fmt.Println("current height is", i)
		block := new(types.Block)
		block, _ = fdb.GetBlock(*nodes[i].Hash)
		if block != nil {
			rollBackFFLDBBlock(fdb, &block.Header)
		}
		fmt.Println("blockhash before rollback:", block.Hash())
		chain.NewBatch()
		chain.RollbackTrimmedBlock(block)
		chain.RollbackBlockHash(block)
		chain.RollbackTransactions(block)
		chain.RollbackUnspendUTXOs(block)
		chain.RollbackUnspend(block)
		chain.RollbackCurrentBlock(block)
		chain.RollbackConfirm(block)
		chain.BatchCommit()

		blockHashAfter := *nodes[i-1].Hash
		fmt.Println("blockhash after rollback:", blockHashAfter)
	}

	return nil
}

func rollBackFFLDBBlock(fflDB blockchain.IFFLDBChainStore, header *types.Header) error {
	err := fflDB.Update(func(dbTx database.Tx) error {
		err := blockchain.DBRemoveBlockNode(dbTx, header)
		if err != nil {
			return err
		}

		// Remove the block hash and height from the block index which
		// tracks the main chain.
		blockHash := header.Hash()
		err = blockchain.DBRemoveBlockIndex(dbTx, &blockHash, header.Height)
		if err != nil {
			return err
		}

		return nil
	})
	return err
}

func getBlockNodes(fdb blockchain.IFFLDBChainStore) []*blockchain.BlockNode {
	blockNodes := make([]*blockchain.BlockNode, 0)
	err := fdb.View(func(dbTx database.Tx) error {
		// Load all of the headers from the data for the known best
		// chain and construct the block index accordingly.  Since the
		// number of nodes are already known, perform a single alloc
		// for them versus a whole bunch of little ones to reduce
		// pressure on the GC.
		log.Infof("Loading block index...")

		blockIndexBucket := dbTx.Metadata().Bucket([]byte("blockheaderidx"))

		// Determine how many blocks will be loaded into the index so we can
		// allocate the right amount.
		var blockCount int32
		cursor := blockIndexBucket.Cursor()
		for ok := cursor.First(); ok; ok = cursor.Next() {
			blockCount++
		}
		log.Info("block count:", blockCount)

		var i int32
		cursor = blockIndexBucket.Cursor()
		for ok := cursor.First(); ok; ok = cursor.Next() {
			header, status, err := blockchain.DeserializeBlockRow(cursor.Value())
			if err != nil {
				return err
			}

			curHash := header.Hash()
			node := blockchain.NewBlockNode(header, &curHash)
			node.Status = status
			blockNodes = append(blockNodes, node)
			i++
		}

		return nil
	})
	if err != nil {
		return nil
	}
	return blockNodes
}
