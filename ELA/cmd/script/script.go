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
	lua "github.com/yuin/gopher-lua"
)

func registerParams(c *cli.Context, L *lua.LState) {
	code := c.String("code")
	publicKey := c.String("publickey")
	depositAddr := c.String("depositaddr")
	nickname := c.String("nickname")
	url := c.String("url")
	location := c.Int64("location")
	depositAmount := c.Float64("depositamount")
	amount := c.Float64("amount")
	fee := c.Float64("fee")
	votes := c.Float64("votes")
	toAddr := c.String("to")
	ownPubkey := c.String("ownerpublickey")
	nodePubkey := c.String("nodepublickey")
	host := c.String("host")
	candidates := c.StringSlice("candidates")
	candidateVotes := c.StringSlice("candidateVotes")

	getDepositAddr := func(L *lua.LState) int {
		L.Push(lua.LString(depositAddr))
		return 1
	}
	getPublicKey := func(L *lua.LState) int {
		L.Push(lua.LString(publicKey))
		return 1
	}
	getCode := func(L *lua.LState) int {
		L.Push(lua.LString(code))
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
	getVotes := func(L *lua.LState) int {
		L.Push(lua.LNumber(votes))
		return 1
	}
	getToAddr := func(L *lua.LState) int {
		L.Push(lua.LString(toAddr))
		return 1
	}
	getOwnerPublicKey := func(L *lua.LState) int {
		L.Push(lua.LString(ownPubkey))
		return 1
	}
	getNodePublicKey := func(L *lua.LState) int {
		L.Push(lua.LString(nodePubkey))
		return 1
	}
	getHostAddr := func(L *lua.LState) int {
		L.Push(lua.LString(host))
		return 1
	}
	getCandidates := func(L *lua.LState) int {
		table := L.NewTable()
		L.SetMetatable(table, L.GetTypeMetatable("candidates"))
		for _, c := range candidates {
			table.Append(lua.LString(c))
		}
		L.Push(table)
		return 1
	}
	getCandidateVotes := func(L *lua.LState) int {
		table := L.NewTable()
		L.SetMetatable(table, L.GetTypeMetatable("candidateVotes"))
		for _, cv := range candidateVotes {
			table.Append(lua.LString(cv))
		}
		L.Push(table)
		return 1
	}
	L.Register("getDepositAddr", getDepositAddr)
	L.Register("getPublicKey", getPublicKey)
	L.Register("getCode", getCode)
	L.Register("getNickName", getNickName)
	L.Register("getUrl", getUrl)
	L.Register("getLocation", getLocation)
	L.Register("getDepositAmount", getDepositAmount)
	L.Register("getAmount", getAmount)
	L.Register("getFee", getFee)
	L.Register("getVotes", getVotes)
	L.Register("getToAddr", getToAddr)
	L.Register("getOwnerPublicKey", getOwnerPublicKey)
	L.Register("getNodePublicKey", getNodePublicKey)
	L.Register("getHostAddr", getHostAddr)
	L.Register("getCandidates", getCandidates)
	L.Register("getCandidateVotes", getCandidateVotes)
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
				Name:  "publickey, pk",
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
			cli.StringFlag{
				Name:  "code, c",
				Usage: "set the code",
			},
			cli.Float64Flag{
				Name:  "votes, v",
				Usage: "set the votes",
			},
			cli.StringFlag{
				Name:  "to",
				Usage: "set the output address",
			},
			cli.StringFlag{
				Name:  "ownerpublickey, opk",
				Usage: "set the node public key",
			},
			cli.StringFlag{
				Name:  "nodepublickey, npk",
				Usage: "set the owner public key",
			},
			cli.StringFlag{
				Name:  "host",
				Usage: "set the host address",
			},
			cli.StringSliceFlag{
				Name:  "candidates, cds",
				Usage: "set the candidates public key",
			},
			cli.StringSliceFlag{
				Name:  "candidateVotes, cvs",
				Usage: "set the candidateVotes values",
			},
		},
		Action: scriptAction,
		OnUsageError: func(c *cli.Context, err error, isSubcommand bool) error {
			common.PrintError(c, err, "script")
			return cli.NewExitError("", 1)
		},
	}
}
