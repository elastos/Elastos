package wallet

import (
	"bytes"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"os"
	"strings"

	"github.com/elastos/Elastos.ELA/account"
	clicom "github.com/elastos/Elastos.ELA/cli/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/servers"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA.Utility/http/jsonrpc"
	"github.com/elastos/Elastos.ELA.Utility/http/util"
)

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

func ShowAccountInfo(client *account.ClientImpl) error {
	// print header
	fmt.Printf("%-34s %-66s\n", "ADDRESS", "PUBLIC KEY")
	fmt.Println(strings.Repeat("-", 34), strings.Repeat("-", 66))

	mainAccount, err := client.GetDefaultAccount()
	if err != nil {
		return err
	}
	publicKey, err := mainAccount.PublicKey.EncodePoint(true)
	if err != nil {
		return err
	}
	addr, err := mainAccount.ProgramHash.ToAddress()
	fmt.Printf("%-34s %-66s\n", addr, hex.EncodeToString(publicKey))
	// print divider line
	fmt.Println(strings.Repeat("-", 34), strings.Repeat("-", 66))

	return nil
}

func ShowAccountBalance(name string) error {
	// print header
	fmt.Printf("%5s %34s %-20s%22s \n", "INDEX", "ADDRESS", "BALANCE", "(LOCKED)")
	fmt.Println("-----", strings.Repeat("-", 34), strings.Repeat("-", 42))

	var fileStore account.FileStore
	fileStore.SetPath(name)
	storeAddresses, err := fileStore.LoadAccountData()
	if err != nil {
		return err
	}

	for _, a := range storeAddresses {
		result, err := jsonrpc.CallParams(clicom.LocalServer(), "listunspent", util.Params{
			"addresses": []string{a.Address},
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

			if types.TransactionType(utxo.TxType) == types.CoinBase && utxo.Confirmations < 100 {
				lockedAmount += *amount
				continue
			}
			availableAmount += *amount
		}

		fmt.Printf("%5d %34s %-20s%22s \n", 0, a.Address, availableAmount.String(), "("+lockedAmount.String()+")")
		fmt.Println("-----", strings.Repeat("-", 34), strings.Repeat("-", 42))
	}

	return nil
}
