// Copyright 2017 The Elastos.ELA.SideChain.ETH Authors
// This file is part of Elastos.ELA.SideChain.ETH.
//
// Elastos.ELA.SideChain.ETH is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Elastos.ELA.SideChain.ETH is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Elastos.ELA.SideChain.ETH. If not, see <http://www.gnu.org/licenses/>.

package main

import (
	"errors"
	"fmt"
	"io/ioutil"

	"github.com/elastos/Elastos.ELA.SideChain.ETH/cmd/evm/internal/compiler"

	cli "gopkg.in/urfave/cli.v1"
)

var compileCommand = cli.Command{
	Action:    compileCmd,
	Name:      "compile",
	Usage:     "compiles easm source to evm binary",
	ArgsUsage: "<file>",
}

func compileCmd(ctx *cli.Context) error {
	debug := ctx.GlobalBool(DebugFlag.Name)

	if len(ctx.Args().First()) == 0 {
		return errors.New("filename required")
	}

	fn := ctx.Args().First()
	src, err := ioutil.ReadFile(fn)
	if err != nil {
		return err
	}

	bin, err := compiler.Compile(fn, src, debug)
	if err != nil {
		return err
	}
	fmt.Println(bin)
	return nil
}
