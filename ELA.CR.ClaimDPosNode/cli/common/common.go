package common

import (
	"bytes"
	"errors"
	"fmt"
	"strconv"

	"github.com/elastos/Elastos.ELA/config"

	"github.com/elaio/gopass"
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

func GetPassword(password []byte, confirmed bool) ([]byte, error) {
	if len(password) > 0 {
		return []byte(password), nil
	}

	fmt.Print("INPUT PASSWORD:")

	password, err := gopass.GetPasswd()
	if err != nil {
		return nil, err
	}

	if !confirmed {
		return password, nil
	} else {

		fmt.Print("CONFIRM PASSWORD:")

		confirm, err := gopass.GetPasswd()
		if err != nil {
			return nil, err
		}

		if !bytes.Equal(password, confirm) {
			return nil, errors.New("input password unmatched")
		}
	}

	return password, nil
}
