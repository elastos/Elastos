// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package wallet

import (
	"github.com/urfave/cli"
)

func NewCommand() *cli.Command {
	var subCommands []cli.Command
	subCommands = append(subCommands, txCommand...)
	subCommands = append(subCommands, accountCommand...)

	return &cli.Command{
		Name:        "wallet",
		Usage:       "Wallet operations",
		Description: "With ela-cli wallet, you could control your asset.",
		ArgsUsage:   "[args]",
		Subcommands: subCommands,
	}
}
