// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package rollback

import (
	"errors"
	"fmt"
	"strconv"

	"github.com/elastos/Elastos.ELA/blockchain"
	cmdcom "github.com/elastos/Elastos.ELA/cmd/common"
	"github.com/elastos/Elastos.ELA/common/config/settings"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/database"

	"github.com/urfave/cli"
)

var (
	appSettings = settings.NewSettings()
	dataDir     = "elastos/data"
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
			cmdcom.ConfigFileFlag,
			cmdcom.DataDirFlag,
			cmdcom.TestNetFlag,
			cmdcom.RegTestFlag,
			cmdcom.InstantBlockFlag,
		},
		Action: rollbackAction,
	}
}

func rollbackAction(c *cli.Context) error {
	appSettings.SetContext(c)
	appSettings.SetupConfig()
	appSettings.InitParamsValue()

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
	if targetHeight < 0 {
		fmt.Println("get height error: height must be positive")
		return nil
	}

	log.NewDefault("logs/node", 0, 0, 0)
	chainStore, err := blockchain.NewChainStore(dataDir, appSettings.Params())
	if err != nil {
		fmt.Println("create chain store failed, ", err)
		return err
	}
	defer chainStore.Close()

	chain, err := blockchain.New(chainStore, appSettings.Params(), nil, nil)
	if err != nil {
		fmt.Println("create blockchain failed, ", err)
		return err
	}
	nodes := chain.Nodes

	currentHeight := len(nodes) - 1
	if targetHeight >= currentHeight {
		errorStr := fmt.Sprintf("Current height of blockchain is %d,"+
			" you can't do this, man.", currentHeight)
		fmt.Println(errorStr)
		return errors.New(errorStr)
	}

	for i := currentHeight; i > targetHeight; i-- {
		fmt.Println("current height is", i)
		block, err := chainStore.GetFFLDB().GetBlock(*nodes[i].Hash)
		if err != nil {
			return err
		}
		if err = removeBlockNode(chainStore.GetFFLDB(), &block.Header); err != nil {
			return err
		}
		fmt.Println("block hash before rollback:", block.Hash())
		err = chainStore.RollbackBlock(block.Block, nodes[i], nil, blockchain.CalcPastMedianTime(nodes[i-1]))
		if err != nil {
			fmt.Println("rollback block failed, ", block.Height, err)
			return err
		}

		blockHashAfter := *nodes[i-1].Hash
		fmt.Println("block hash after rollback:", blockHashAfter)
	}

	return nil
}

func removeBlockNode(fflDB blockchain.IFFLDBChainStore, header *types.Header) error {
	return fflDB.Update(func(dbTx database.Tx) error {
		return blockchain.DBRemoveBlockNode(dbTx, header)
	})
}
