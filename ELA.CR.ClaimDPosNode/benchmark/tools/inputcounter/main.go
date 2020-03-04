// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package main

import (
	"errors"
	"fmt"
	"log"
	"os"

	bencli "github.com/elastos/Elastos.ELA/benchmark/common/cli"
	"github.com/elastos/Elastos.ELA/benchmark/tools/inputcounter/calculator"

	"github.com/urfave/cli"
)

var (
	Version        string
	essentialFlags []cli.Flag
)

func main() {
	app := cli.NewApp()
	app.Name = "ela-inputcounter"
	app.Version = Version
	app.HelpName = "ela-inputcounter"
	app.Usage = "command line tools for ELA input counter"
	app.UsageText = "ela-inputcounter [global options] command [command options] [args]"
	app.HideHelp = false
	app.HideVersion = false
	essentialFlags = []cli.Flag{
		bencli.BlockSizeFlag,
		bencli.TxFormatFlag,
	}
	app.Flags = essentialFlags
	app.Action = func(c *cli.Context) error {
		return calculate(c)
	}

	if err := app.Run(os.Args); err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
}

func calculate(c *cli.Context) error {
	if !c.IsSet(bencli.BlockSizeFlag.Name) {
		return errors.New("block size is missing")
	}
	if !c.IsSet(bencli.TxFormatFlag.Name) {
		return errors.New("tx format is missing")
	}

	count, err := calculator.Calculate(c.Uint(bencli.BlockSizeFlag.Name),
		c.Uint(bencli.TxFormatFlag.Name))
	if err != nil {
		log.Fatal("calculate error: ", err)
	} else {
		log.Println("result count is: ", count)
	}

	return nil
}
