package common

import (
	"os"
	"fmt"
	"bufio"
	"errors"
	"strings"
	"strconv"

	. "SPVWallet/core"
	tx "SPVWallet/core/transaction"
	walt "SPVWallet/wallet"

	"github.com/urfave/cli"
	"github.com/howeyc/gopass"
)

var CommonFlags = []cli.Flag{
	cli.StringFlag{
		Name:  "password, p",
		Usage: "keystore password",
	},
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

		if !IsEqualBytes(password, confirm) {
			return nil, errors.New("input password unmatched")
		}
	}

	return password, nil
}

func SelectAccount(wallet walt.Wallet) (string, error) {
	scripts, err := wallet.GetScripts()
	if err != nil || len(scripts) == 0 {
		return "", errors.New("fail to load wallet addresses")
	}

	// only one address return it
	if len(scripts) == 1 {
		programHash, _ := tx.ToProgramHash(scripts[0])
		return programHash.ToAddress()
	}

	// print out addresses in wallet
	fmt.Printf("%5s %34s %32s\n", "INDEX", "ADDRESS", "BALANCE")
	fmt.Println("-----", strings.Repeat("-", 34), strings.Repeat("-", 32))
	for i, script := range scripts {
		balance := Fixed64(0)
		programHash, err := tx.ToProgramHash(script)
		if err != nil {
			return "", errors.New("parse address script failed")
		}
		address, _ := programHash.ToAddress()
		UTXOs, err := wallet.GetAddressUTXOs(programHash)
		if err != nil {
			return "", errors.New("get " + address + " UTXOs failed")
		}
		for _, utxo := range UTXOs {
			balance += utxo.Value
		}

		fmt.Printf("%5d %34s %-32s\n", i+1, address, balance.String())
		fmt.Println("-----", strings.Repeat("-", 34), strings.Repeat("-", 32))
	}

	// select address by index input
	fmt.Println("Please input the address INDEX you want to use and press enter")

	index := -1
	for index == -1 {
		index = getInput(len(scripts))
	}

	programHash, _ := tx.ToProgramHash(scripts[index])
	return programHash.ToAddress()
}

func getInput(max int) int {
	fmt.Print("INPUT INDEX: ")
	input, err := bufio.NewReader(os.Stdin).ReadString('\n')
	if err != nil {
		fmt.Println("read input falied")
		return -1
	}

	// trim space
	input = strings.TrimSpace(input)

	index, err := strconv.ParseInt(input, 10, 32)
	if err != nil {
		fmt.Println("please input a positive integer")
		return -1
	}

	if int(index) > max {
		fmt.Println("INDEX should between 1 ~", max)
		return -1
	}

	return int(index) - 1
}
