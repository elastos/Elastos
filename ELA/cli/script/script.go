package script

import (
	"fmt"

	"github.com/elastos/Elastos.ELA/cli/common"
	"github.com/elastos/Elastos.ELA/cli/script/api"

	"github.com/urfave/cli"
	"github.com/yuin/gopher-lua"
)

func scriptAction(c *cli.Context) error {
	if c.NumFlags() == 0 {
		cli.ShowSubcommandHelp(c)
		return nil
	}

	fileContent := c.String("file")
	strContent := c.String("str")

	L := lua.NewState()
	defer L.Close()
	L.PreloadModule("api", api.Loader)
	api.RegisterDataType(L)

	if strContent != "" {
		fmt.Println("str test")
		if err := L.DoString(strContent); err != nil {
			panic(err)
		}
	}

	if fileContent != "" {
		fmt.Println("file test")
		if err := L.DoFile(fileContent); err != nil {
			panic(err)
		}
	}

	return nil
}

func NewCommand() *cli.Command {
	return &cli.Command{
		Name:        "script",
		Usage:       "blockchain test via lua script.",
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
		},
		Action: scriptAction,
		OnUsageError: func(c *cli.Context, err error, isSubcommand bool) error {
			common.PrintError(c, err, "script")
			return cli.NewExitError("", 1)
		},
	}
}
