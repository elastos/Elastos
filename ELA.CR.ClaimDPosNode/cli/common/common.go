package common

import (
	"fmt"
	"strconv"

	"github.com/elastos/Elastos.ELA/config"

	"github.com/urfave/cli"
)

func LocalServer() string {
	return "http://localhost" + ":" + strconv.Itoa(config.Parameters.HttpJsonPort)
}

func PrintError(c *cli.Context, err error, cmd string) {
	fmt.Println("Incorrect Usage:", err)
	fmt.Println("")
	cli.ShowCommandHelp(c, cmd)
}
