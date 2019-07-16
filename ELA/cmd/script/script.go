// Copyright (c) 2017-2019 Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package script

import (
	"fmt"
	"os"

	"github.com/elastos/Elastos.ELA/cmd/common"
	"github.com/elastos/Elastos.ELA/cmd/script/api"

	"github.com/urfave/cli"
	"github.com/yuin/gopher-lua"
)

func registerParams(c *cli.Context, L *lua.LState) {
	publicKey := c.String("pubkey")
	depositAddr := c.String("depositaddr")
	nickname := c.String("nickname")
	url := c.String("url")
	location := c.Int64("location")
	depositAmount := c.Int64("depositamount")
	amount := c.Int64("amount")
	fee := c.Int64("fee")

	getDepositAddr := func(L *lua.LState) int {
		L.Push(lua.LString(depositAddr))
		return 1
	}
	getPublicKey := func(L *lua.LState) int {
		L.Push(lua.LString(publicKey))
		return 1
	}
	getNickName := func(L *lua.LState) int {
		L.Push(lua.LString(nickname))
		return 1
	}
	getUrl := func(L *lua.LState) int {
		L.Push(lua.LString(url))
		return 1
	}
	getLocation := func(L *lua.LState) int {
		L.Push(lua.LNumber(location))
		return 1
	}
	getDepositAmount := func(L *lua.LState) int {
		L.Push(lua.LNumber(depositAmount))
		return 1
	}
	getAmount := func(L *lua.LState) int {
		L.Push(lua.LNumber(amount))
		return 1
	}
	getFee := func(L *lua.LState) int {
		L.Push(lua.LNumber(fee))
		return 1
	}
	L.Register("getDepositAddr", getDepositAddr)
	L.Register("getPublicKey", getPublicKey)
	L.Register("getNickName", getNickName)
	L.Register("getUrl", getUrl)
	L.Register("getLocation", getLocation)
	L.Register("getDepositAmount", getDepositAmount)
	L.Register("getAmount", getAmount)
	L.Register("getFee", getFee)
}

func scriptAction(c *cli.Context) error {
	if c.NumFlags() == 0 {
		cli.ShowSubcommandHelp(c)
		return nil
	}

	fileContent := c.String("file")
	strContent := c.String("str")
	testContent := c.String("test")

	L := lua.NewState()
	defer L.Close()
	L.PreloadModule("api", api.Loader)
	api.RegisterDataType(L)

	if strContent != "" {
		if err := L.DoString(strContent); err != nil {
			panic(err)
		}
	}

	if fileContent != "" {
		registerParams(c, L)
		if err := L.DoFile(fileContent); err != nil {
			panic(err)
		}
	}

	if testContent != "" {
		fmt.Println("begin white box")
		if err := L.DoFile(testContent); err != nil {
			println(err.Error())
			os.Exit(1)
		} else {
			os.Exit(0)
		}
	}

	return nil
}

func NewCommand() *cli.Command {
	return &cli.Command{
		Name:        "script",
		Usage:       "Test the blockchain via lua script",
		Description: "With ela-cli test, you could test blockchain.",
		ArgsUsage:   "[args]",
		Flags: []cli.Flag{
			cli.StringFlag{
				Name:  "file, f",
				Usage: "test file",
			},
			cli.StringFlag{
				Name:  "str, s",
				Usage: "test string",
			},
			cli.StringFlag{
				Name:  "test, t",
				Usage: "white box test",
			},
			cli.StringFlag{
				Name:  "pubkey, pk",
				Usage: "set the public key",
			},
			cli.StringFlag{
				Name:  "depositaddr, daddr",
				Usage: "set the deposit addr",
			},
			cli.Float64Flag{
				Name:  "depositamount, damount",
				Usage: "set the amount",
			},
			cli.StringFlag{
				Name:  "nickname, nn",
				Usage: "set the nick name",
			},
			cli.StringFlag{
				Name:  "url, u",
				Usage: "set the url",
			},
			cli.Int64Flag{
				Name:  "location, l",
				Usage: "set the location",
			},
			cli.Float64Flag{
				Name:  "amount",
				Usage: "set the amount",
			},
			cli.Float64Flag{
				Name:  "fee",
				Usage: "set the fee",
			},
		},
		Action: scriptAction,
		OnUsageError: func(c *cli.Context, err error, isSubcommand bool) error {
			common.PrintError(c, err, "script")
			return cli.NewExitError("", 1)
		},
	}
}
