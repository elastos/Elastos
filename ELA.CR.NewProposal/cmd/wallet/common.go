// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package wallet

import (
	"bytes"
	"encoding/csv"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"os"
	"strings"

	"github.com/elastos/Elastos.ELA/account"
	cmdcom "github.com/elastos/Elastos.ELA/cmd/common"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/servers"
	"github.com/elastos/Elastos.ELA/utils/http"
)

const (
	// maxPrintLen is the maximum print length
	maxPrintLen = 2000
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

func ShowAccountInfo(client *account.Client) error {
	fmt.Printf("%-34s %-66s\n", "ADDRESS", "PUBLIC KEY")
	fmt.Println(strings.Repeat("-", 34), strings.Repeat("-", 66))

	for _, acc := range client.GetAccounts() {
		var publicKey []byte
		if acc.PublicKey != nil {
			var err error
			publicKey, err = acc.PublicKey.EncodePoint(true)
			if err != nil {
				return err
			}
		}
		addr, err := acc.ProgramHash.ToAddress()
		if err != nil {
			return err
		}
		prefixType := contract.GetPrefixType(acc.ProgramHash)
		if prefixType == contract.PrefixStandard {
			fmt.Printf("%-34s %-66s\n", addr, hex.EncodeToString(publicKey))
		} else if prefixType == contract.PrefixMultiSig {
			publicKeys, err := crypto.ParseMultisigScript(acc.RedeemScript)
			if err != nil {
				return err
			}
			if len(publicKeys) > 0 {
				fmt.Printf("%-34s %-66s\n", addr,
					hex.EncodeToString(publicKeys[0][1:]))
			}
			for _, publicKey := range publicKeys[1:] {
				fmt.Printf("%-34s %-66s\n", "",
					hex.EncodeToString(publicKey[1:]))
			}
		}
		fmt.Println(strings.Repeat("-", 34), strings.Repeat("-", 66))
	}

	return nil
}

func ShowAccountBalance(walletPath string) error {
	// print header
	fmt.Printf("%5s %34s %-20s%22s \n", "INDEX", "ADDRESS", "BALANCE", "(LOCKED)")
	fmt.Println("-----", strings.Repeat("-", 34), strings.Repeat("-", 42))

	storeAccounts, err := account.GetWalletAccountData(walletPath)
	if err != nil {
		return err
	}

	for i, a := range storeAccounts {
		available, locked, err := getAddressBalance(a.Address)
		if err != nil {
			return err
		}
		fmt.Printf("%5d %34s %-20s%22s \n", i, a.Address, available.String(), "("+locked.String()+")")
		fmt.Println("-----", strings.Repeat("-", 34), strings.Repeat("-", 42))
	}

	return nil
}

func getUTXOsByAmount(address string, amount common.Fixed64) ([]servers.UTXOInfo, error) {
	result, err := cmdcom.RPCCall("getutxosbyamount", http.Params{
		"address": address,
		"amount":  amount.String(),
	})
	if err != nil {
		return nil, err
	}
	data, err := json.Marshal(result)
	if err != nil {
		return nil, err
	}
	var UTXOs []servers.UTXOInfo
	err = json.Unmarshal(data, &UTXOs)

	return UTXOs, nil
}

func getAddressUTXOs(address string) ([]servers.UTXOInfo, []servers.UTXOInfo, error) {
	result, err := cmdcom.RPCCall("listunspent", http.Params{
		"addresses": []string{address},
	})
	if err != nil {
		return nil, nil, err
	}
	data, err := json.Marshal(result)
	if err != nil {
		return nil, nil, err
	}
	var UTXOs []servers.UTXOInfo
	err = json.Unmarshal(data, &UTXOs)

	var availableUTXOs []servers.UTXOInfo
	var lockedUTXOs []servers.UTXOInfo
	for _, utxo := range UTXOs {
		if types.TxType(utxo.TxType) == types.CoinBase && utxo.Confirmations < 101 {
			lockedUTXOs = append(lockedUTXOs, utxo)
			continue
		}
		availableUTXOs = append(availableUTXOs, utxo)
	}

	return availableUTXOs, lockedUTXOs, nil
}

func getAddressBalance(address string) (common.Fixed64, common.Fixed64, error) {
	availableUTXOs, lockedUTXOs, err := getAddressUTXOs(address)
	if err != nil {
		return 0, 0, err
	}
	availableAmount := common.Fixed64(0)
	lockedAmount := common.Fixed64(0)

	for _, utxo := range availableUTXOs {
		amount, err := common.StringToFixed64(utxo.Amount)
		if err != nil {
			return 0, 0, err
		}
		availableAmount += *amount
	}

	for _, utxo := range lockedUTXOs {
		amount, err := common.StringToFixed64(utxo.Amount)
		if err != nil {
			return 0, 0, err
		}
		lockedAmount += *amount
	}

	return availableAmount, lockedAmount, nil
}

func OutputTx(haveSign, needSign int, txn *types.Transaction) error {
	// Serialise transaction content
	buf := new(bytes.Buffer)
	err := txn.Serialize(buf)
	if err != nil {
		fmt.Println("serialize error", err)
	}
	content := common.BytesToHexString(buf.Bytes())

	// Print transaction hex string
	if len(content) > maxPrintLen {
		fmt.Println("Hex: ", content[:maxPrintLen], "... ...")
	} else {
		fmt.Println("Hex: ", content)
	}

	// Output to file
	fileName := "to_be_signed" // Create transaction file name

	if haveSign == 0 && needSign > 0 {
		//	Transaction created do nothing
	} else if needSign > haveSign {
		fileName = fmt.Sprint(fileName, "_", haveSign, "_of_", needSign)
	} else if needSign == haveSign {
		fileName = "ready_to_send"
	}
	fileName = fileName + ".txn"

	file, err := os.OpenFile(fileName, os.O_CREATE|os.O_WRONLY|os.O_TRUNC, 0600)
	if err != nil {
		return err
	}

	_, err = file.Write([]byte(content))
	if err != nil {
		return err
	}

	var tx types.Transaction
	txBytes, _ := hex.DecodeString(content)
	if err := tx.Deserialize(bytes.NewReader(txBytes)); err != nil {
		return err
	}

	// Print output file to console
	fmt.Println("File: ", fileName)

	return nil
}

func parseMultiOutput(path string) ([]*OutputInfo, error) {
	if _, err := os.Stat(path); err != nil {
		return nil, errors.New("invalid multi output file path")
	}
	file, err := os.OpenFile(path, os.O_RDONLY, 0400)
	if err != nil {
		return nil, errors.New("open multi output file failed")
	}

	var multiOutput []*OutputInfo
	r := csv.NewReader(file)
	for {
		record, err := r.Read()
		if err == io.EOF {
			break
		}
		if err != nil {
			return nil, errors.New(fmt.Sprint("invalid multi output data:", err.Error()))
		}

		amountStr := strings.TrimSpace(record[1])
		amount, err := common.StringToFixed64(amountStr)
		if err != nil {
			return nil, errors.New("invalid multi output transaction amount: " + amountStr)
		}
		address := strings.TrimSpace(record[0])
		multiOutput = append(multiOutput, &OutputInfo{address, amount})
		fmt.Println("Multi output address:", address, ", amount:", amountStr)
	}

	return multiOutput, nil
}

func parseCandidates(path string) ([]string, error) {
	if _, err := os.Stat(path); err != nil {
		return nil, errors.New("invalid candidates file path")
	}
	file, err := os.OpenFile(path, os.O_RDONLY, 0400)
	if err != nil {
		return nil, errors.New("open candidates file failed")
	}

	var candidates []string
	r := csv.NewReader(file)
	for {
		record, err := r.Read()
		if err == io.EOF {
			break
		}
		if err != nil {
			return nil, errors.New(fmt.Sprint("invalid candidate data:", err.Error()))
		}

		candidate := strings.TrimSpace(record[0])
		candidates = append(candidates, candidate)
		fmt.Println("candidate:", candidate)
	}

	return candidates, nil
}
