package common

import (
	"os"
	"fmt"
	"bufio"
	"errors"
	"strings"
	"strconv"

	. "github.com/elastos/Elastos.ELA.SPV/core"
	"github.com/elastos/Elastos.ELA.SPV/db"
	walt "github.com/elastos/Elastos.ELA.SPV/wallet"

	"github.com/urfave/cli"
	"github.com/AlexpanXX/gopass"
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

func ShowAccountInfo(password []byte) error {
	var err error
	password, err = GetPassword(password, false)
	if err != nil {
		return err
	}

	keyStore, err := walt.OpenKeystore(password)
	if err != nil {
		return err
	}

	// print header
	fmt.Printf("%5s %34s %66s %6s\n", "INDEX", "ADDRESS", "PUBLIC KEY", "TYPE")
	fmt.Println("-----", strings.Repeat("-", 34), strings.Repeat("-", 66), "------")

	// print accounts
	for i, account := range keyStore.GetAccounts() {
		accountType := "SUB"
		if i == 0 {
			accountType = "MASTER"
		}
		// print content
		publicKey := account.PublicKey()
		publicKeyBytes, _ := publicKey.EncodePoint(true)
		fmt.Printf("%5d %-34s %-66s %6s\n", i+1, account.Address(), BytesToHexString(publicKeyBytes), accountType)
		// print divider line
		fmt.Println("-----", strings.Repeat("-", 34), strings.Repeat("-", 66), "------")
	}

	return nil
}

func SelectAccount(wallet walt.Wallet) (string, error) {
	addrs, err := wallet.GetAddrs()
	if err != nil || len(addrs) == 0 {
		return "", errors.New("fail to load wallet addresses")
	}

	// only one address return it
	if len(addrs) == 1 {
		return addrs[0].String(), nil
	}

	// show accounts
	err = ShowAccount(addrs, wallet)
	if err != nil {
		return "", err
	}

	// select address by index input
	fmt.Println("Please input the address INDEX you want to use and press enter")

	index := -1
	for index == -1 {
		index = getInput(len(addrs))
	}

	return addrs[index].String(), nil
}

func ShowAccount(addrs []*db.Addr, wallet walt.Wallet) error {
	// print header
	fmt.Printf("%5s %34s %-20s%22s %6s\n", "INDEX", "ADDRESS", "BALANCE", "(LOCKED)", "TYPE")
	fmt.Println("-----", strings.Repeat("-", 34), strings.Repeat("-", 42), "------")

	currentHeight := wallet.ChainHeight()
	for i, addr := range addrs {
		available := Fixed64(0)
		locked := Fixed64(0)
		UTXOs, err := wallet.GetAddressUTXOs(addr.Hash())
		if err != nil {
			return errors.New("get " + addr.String() + " UTXOs failed")
		}
		for _, utxo := range UTXOs {
			if utxo.LockTime <= currentHeight {
				available += utxo.Value
			} else {
				locked += utxo.Value
			}
		}

		fmt.Printf("%5d %34s %-20s%22s %6s\n", i+1, addr.String(), available.String(), "("+locked.String()+")", addr.TypeName())
		fmt.Println("-----", strings.Repeat("-", 34), strings.Repeat("-", 42), "------")
	}

	return nil
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
