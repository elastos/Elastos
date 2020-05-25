// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package main

import (
	"math/rand"
	"os"
	"time"

	cmdcom "github.com/elastos/Elastos.ELA/cmd/common"
	"github.com/elastos/Elastos.ELA/cmd/info"
	"github.com/elastos/Elastos.ELA/cmd/mine"
	"github.com/elastos/Elastos.ELA/cmd/rollback"
	"github.com/elastos/Elastos.ELA/cmd/script"
	"github.com/elastos/Elastos.ELA/cmd/wallet"

	"github.com/urfave/cli"
)

var Version string

func main() {
	app := cli.NewApp()
	app.Name = "ela-cli"
	app.Version = Version
	app.HelpName = "ela-cli"
	app.Usage = "command line tool for ELA blockchain"
	app.UsageText = "ela-cli [global options] command [command options] [args]"
	app.HideHelp = false
	app.HideVersion = false
	app.Flags = []cli.Flag{
		cmdcom.RPCUserFlag,
		cmdcom.RPCPasswordFlag,
		cmdcom.RPCPortFlag,
	}
	app.Before = func(c *cli.Context) error {
		//seed transaction nonce
		rand.Seed(time.Now().UnixNano())

		cmdcom.SetRpcConfig(c)
		return nil
	}
	//commands
	app.Commands = []cli.Command{
		*wallet.NewCommand(),
		*info.NewCommand(),
		*mine.NewCommand(),
		*script.NewCommand(),
		*rollback.NewCommand(),
	}

	//sort.Sort(cli.CommandsByName(app.Commands))
	//sort.Sort(cli.FlagsByName(app.Flags))
	newArgs, err := cmdcom.MoveRPCFlags(os.Args)
	if err != nil {
		cmdcom.PrintErrorMsg(err.Error())
		os.Exit(1)
	}

	if err := app.Run(newArgs); err != nil {
		cmdcom.PrintErrorMsg(err.Error())
		os.Exit(1)
	}
}
