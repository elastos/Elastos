package wallet

import (
	"bytes"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"strings"

	"github.com/elastos/Elastos.ELA/account"
	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/servers"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/http/jsonrpc"
	"github.com/elastos/Elastos.ELA.Utility/http/util"
	"github.com/howeyc/gopass"
	"github.com/urfave/cli"
)

func PrintError(c *cli.Context, err error, cmd string) {
	fmt.Println("Incorrect Usage:", err)
	fmt.Println("")
	cli.ShowCommandHelp(c, cmd)
}

func FormatOutput(o []byte) error {
	var out bytes.Buffer
	err := json.Indent(&out, o, "", "\t")
	if err != nil {
		return err
	}
	out.Write([]byte("\n"))
	_, err = out.WriteTo(os.Stdout)

	return err
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

// GetConfirmedPassword gets double confirmed password from user input
func GetConfirmedPassword() ([]byte, error) {
	fmt.Printf("Password:")
	first, err := gopass.GetPasswd()
	if err != nil {
		return nil, err
	}
	fmt.Printf("Re-enter Password:")
	second, err := gopass.GetPasswd()
	if err != nil {
		return nil, err
	}
	if len(first) != len(second) {
		fmt.Println("Unmatched Password")
		os.Exit(1)
	}
	for i, v := range first {
		if v != second[i] {
			fmt.Println("Unmatched Password")
			os.Exit(1)
		}
	}
	return first, nil
}

func FileExisted(filename string) bool {
	_, err := os.Stat(filename)
	return err == nil || os.IsExist(err)
}

func ShowAccountInfo(name string, password []byte) error {
	var err error
	password, err = GetPassword(password, false)
	if err != nil {
		return err
	}

	keyStore, err := account.OpenKeystore(name, password)
	if err != nil {
		return err
	}

	// print header
	fmt.Printf("%-34s %-66s\n", "ADDRESS", "PUBLIC KEY")
	fmt.Println(strings.Repeat("-", 34), strings.Repeat("-", 66))

	// print account
	publicKey := keyStore.GetPublicKey()
	publicKeyBytes, _ := publicKey.EncodePoint(true)
	fmt.Printf("%-34s %-66s\n", keyStore.Address(), hex.EncodeToString(publicKeyBytes))
	// print divider line
	fmt.Println(strings.Repeat("-", 34), strings.Repeat("-", 66))

	return nil
}

func ShowAccountBalance(name string, password []byte) error {
	// print header
	fmt.Printf("%5s %34s %-20s%22s \n", "INDEX", "ADDRESS", "BALANCE", "(LOCKED)")
	fmt.Println("-----", strings.Repeat("-", 34), strings.Repeat("-", 42))

	var err error
	password, err = GetPassword(password, false)
	if err != nil {
		return err
	}

	keyStore, err := account.OpenKeystore(name, password)
	if err != nil {
		return err
	}

	result, err := jsonrpc.CallParams(account.ElaServer(), "listunspent", util.Params{
		"addresses": []string{keyStore.Address()},
	})
	if err != nil {
		return err
	}
	data, err := json.Marshal(result)
	if err != nil {
		return err
	}
	var utxos []servers.UTXOInfo
	err = json.Unmarshal(data, &utxos)

	//var availabelUtxos []servers.UTXOInfo
	availableAmount := common.Fixed64(0)
	lockedAmount := common.Fixed64(0)
	for _, utxo := range utxos {
		amount, err := common.StringToFixed64(utxo.Amount)
		if err != nil {
			return err
		}

		if core.TransactionType(utxo.TxType) == core.CoinBase && utxo.Confirmations < 100 {
			lockedAmount += *amount
			continue
		}
		availableAmount += *amount
	}

	fmt.Printf("%5d %34s %-20s%22s \n", 0, keyStore.Address(), availableAmount.String(), "("+lockedAmount.String()+")")
	fmt.Println("-----", strings.Repeat("-", 34), strings.Repeat("-", 42))

	return nil
}
