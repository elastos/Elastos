package common

import (
	"fmt"
	"os"

	"github.com/elastos/Elastos.ELA/utils/http"
	"github.com/elastos/Elastos.ELA/utils/http/jsonrpc"
	"github.com/urfave/cli"
)

const (
	DefaultConfigPath = "./config.json"
	DefaultDataDir    = "./elastos"
)

var (
	rpcPort     = "20336"
	rpcUser     = ""
	rpcPassword = ""
)

func SetRpcConfig(c *cli.Context) {
	port := c.String("rpcport")
	if port != "" {
		rpcPort = port
	}
	user := c.String("rpcuser")
	if user != "" {
		rpcUser = user
	}
	password := c.String("rpcpassword")
	if password != "" {
		rpcPassword = password
	}
}

func localServer() string {
	return "http://localhost:" + rpcPort
}

func RPCCall(method string, params http.Params) (interface{}, error) {
	req := jsonrpc.Request{
		Method: method,
		Params: params,
	}
	return jsonrpc.Call(localServer(), req, rpcUser, rpcPassword)
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
