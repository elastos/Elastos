package client

import (
	"bufio"
	"bytes"
	"errors"
	"fmt"
	"os"
	"strconv"
	"strings"

	"github.com/elastos/Elastos.ELA.SPV/wallet/sutil"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/howeyc/gopass"
	"github.com/urfave/cli"
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

		if !bytes.Equal(password, confirm) {
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

	keyStore, err := OpenKeystore(password)
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
		fmt.Printf("%5d %-34s %-66s %6s\n", i+1, account.Address(), common.BytesToHexString(publicKeyBytes), accountType)
		// print divider line
		fmt.Println("-----", strings.Repeat("-", 34), strings.Repeat("-", 66), "------")
	}

	return nil
}

func SelectAccount(wallet *Wallet) (string, error) {
	addrs, err := wallet.GetAddrs()
	if err != nil || len(addrs) == 0 {
		return "", errors.New("fail to load wallet addresses")
	}

	// only one address return it
	if len(addrs) == 1 {
		return addrs[0].String(), nil
	}

	// show accounts
	err = ShowAccounts(addrs, nil, wallet)
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

func ShowAccounts(addrs []*sutil.Addr, newAddr *common.Uint168, wallet *Wallet) error {
	// print header
	fmt.Printf("%5s %34s %-20s%22s %6s\n", "INDEX", "ADDRESS", "BALANCE", "(LOCKED)", "TYPE")
	fmt.Println("-----", strings.Repeat("-", 34), strings.Repeat("-", 42), "------")

	currentHeight := wallet.BestHeight()
	for i, addr := range addrs {
		available := common.Fixed64(0)
		locked := common.Fixed64(0)
		UTXOs, err := wallet.GetAddressUTXOs(addr.Hash())
		if err != nil {
			return fmt.Errorf("get %s UTXOs failed, %s", addr, err)
		}
		for _, utxo := range UTXOs {
			if utxo.LockTime >= currentHeight || utxo.AtHeight == 0 {
				locked += utxo.Value
			} else {
				available += utxo.Value
			}
		}
		var format = "%5d %34s %-20s%22s %6s\n"
		if newAddr != nil && newAddr.IsEqual(*addr.Hash()) {
			format = "\033[0;32m" + format + "\033[m"
		}

		fmt.Printf(format, i+1, addr.String(), available.String(), "("+locked.String()+")", addr.TypeName())
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
