package common

import (
	"fmt"
	"os"
	"strconv"

	"github.com/elastos/Elastos.ELA/common/config"

	"github.com/urfave/cli"
)

func LocalServer() string {
	return "http://localhost" + ":" + strconv.Itoa(config.Template.HttpJsonPort)
}

func PrintError(c *cli.Context, err error, cmd string) {
	fmt.Println("Incorrect Usage:", err)
	fmt.Println("")
	cli.ShowCommandHelp(c, cmd)
}

func FileExisted(filename string) bool {
	_, err := os.Stat(filename)
	return err == nil || os.IsExist(err)
}

func PrintErrorMsg(format string, a ...interface{}) {
	format = fmt.Sprintf("\033[31m[ERROR] %s\033[0m\n", format) //Print error msg with red color
	fmt.Printf(format, a...)
}

func PrintWarnMsg(format string, a ...interface{}) {
	format = fmt.Sprintf("\033[33m[WARN] %s\033[0m\n", format) //Print error msg with yellow color
	fmt.Printf(format, a...)
}

func PrintInfoMsg(format string, a ...interface{}) {
	fmt.Printf(format+"\n", a...)
}
