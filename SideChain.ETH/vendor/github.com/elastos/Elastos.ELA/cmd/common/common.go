package common

import (
	"errors"
	"io/ioutil"
	"os"
	"strings"

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

func ReadFile(filePath string) (string, error) {
	if _, err := os.Stat(filePath); err != nil {
		return "", errors.New("invalid transaction file path")
	}
	file, err := os.OpenFile(filePath, os.O_RDONLY, 0666)
	if err != nil {
		return "", errors.New("open transaction file failed")
	}
	rawData, err := ioutil.ReadAll(file)
	if err != nil {
		return "", errors.New("read transaction file failed")
	}

	content := strings.TrimSpace(string(rawData))
	if content == "" {
		return "", errors.New("transaction file is empty")
	}
	return content, nil
}
