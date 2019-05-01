package common

import (
	"errors"
	"fmt"
	"os"

	"github.com/elastos/Elastos.ELA/utils"
	"github.com/elastos/Elastos.ELA/utils/http"
	"github.com/elastos/Elastos.ELA/utils/http/jsonrpc"

	"github.com/urfave/cli"
)

const (
	defaultConfigPath = "./config.json"
	defaultDataDir    = "elastos"
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

// MoveRPCFlags finds the rpc argument and moves it to the front
// of the argument array.
func MoveRPCFlags(args []string) ([]string, error) {
	newArgs := args[:1]
	cacheArgs := make([]string, 0)

	for i := 1; i < len(args); i++ {
		switch args[i] {
		case "--rpcport":
			fallthrough
		case "--rpcuser":
			fallthrough
		case "--rpcpassword":
			newArgs = append(newArgs, args[i])
			if i == len(args)-1 {
				return nil, errors.New("invalid flag " + args[i])
			}
			newArgs = append(newArgs, args[i+1])
			i++
		default:
			cacheArgs = append(cacheArgs, args[i])
		}
	}

	newArgs = append(newArgs, cacheArgs...)
	return newArgs, nil
}

// GetFlagPassword gets node's wallet password from command line or user input
func GetFlagPassword(c *cli.Context) ([]byte, error) {
	flagPassword := c.String("password")
	password := []byte(flagPassword)
	if flagPassword == "" {
		return utils.GetPassword()
	}

	return password, nil
}
