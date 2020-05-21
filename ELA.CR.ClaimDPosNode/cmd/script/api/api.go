// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package api

import (
	"bytes"
	"encoding/hex"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/elastos/Elastos.ELA/blockchain"
	cmdcom "github.com/elastos/Elastos.ELA/cmd/common"
	"github.com/elastos/Elastos.ELA/cmd/wallet"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/crypto"
	dlog "github.com/elastos/Elastos.ELA/dpos/log"
	"github.com/elastos/Elastos.ELA/dpos/state"
	"github.com/elastos/Elastos.ELA/utils/http"
	"github.com/elastos/Elastos.ELA/utils/signal"
	"github.com/elastos/Elastos.ELA/utils/test"

	"github.com/yuin/gopher-lua"
)

func Loader(L *lua.LState) int {
	// register functions to the table
	mod := L.SetFuncs(L.NewTable(), exports)
	// register other stuff
	L.SetField(mod, "version", lua.LString("0.1"))

	// returns the module
	L.Push(mod)
	return 1
}

var exports = map[string]lua.LGFunction{
	"hex_reverse":       hexReverse,
	"send_tx":           sendTx,
	"get_asset_id":      getAssetID,
	"set_arbitrators":   setArbitrators,
	"init_ledger":       initLedger,
	"close_store":       closeStore,
	"clear_store":       clearStore,
	"get_dir_all_files": getDirAllFiles,
	"get_standard_addr": getStandardAddr,
	"output_tx":         outputTx,
}

func outputTx(L *lua.LState) int {
	txn := checkTransaction(L, 1)
	if len(txn.Programs) == 0 {
		fmt.Println("no program found in transaction")
		os.Exit(1)
	}
	haveSign, needSign, _ := crypto.GetSignStatus(txn.Programs[0].Code, txn.Programs[0].Parameter)
	fmt.Println("[", haveSign, "/", needSign, "] Transaction was successfully signed")
	wallet.OutputTx(haveSign, needSign, txn)

	return 0
}

func getStandardAddr(L *lua.LState) int {
	pubKeyHex := L.ToString(1)
	pubKey, err := common.HexStringToBytes(pubKeyHex)
	if err != nil {
		fmt.Println("invalid public key hex")
		os.Exit(1)
	}
	pk, err := crypto.DecodePoint(pubKey)
	if err != nil {
		fmt.Println("invalid public key")
		os.Exit(1)
	}
	code, err := contract.CreateStandardRedeemScript(pk)
	if err != nil {
		fmt.Println(err.Error())
		os.Exit(1)
	}
	programHash, err := contract.PublicKeyToStandardProgramHash(pubKey)
	if err != nil {
		fmt.Println(err.Error())
		os.Exit(1)
	}
	addr, err := programHash.ToAddress()
	if err != nil {
		fmt.Println(err.Error())
		os.Exit(1)
	}

	L.Push(lua.LString(addr))
	L.Push(lua.LString(common.BytesToHexString(code)))

	return 2
}

func getDirAllFiles(L *lua.LState) int {
	str := L.ToString(1)

	files, err := walkDir(str, ".lua")
	if err != nil {
		fmt.Println("Read directory error: " + err.Error())
	}

	table := L.NewTable()
	L.SetMetatable(table, L.GetTypeMetatable(luaStringsTypeName))
	for _, f := range files {
		table.Append(lua.LString(f))
	}
	L.Push(table)

	return 1
}

func hexReverse(L *lua.LState) int {
	str := L.ToString(1)
	buf, _ := hex.DecodeString(str)
	retHex := hex.EncodeToString(common.BytesReverse(buf))

	L.Push(lua.LString(retHex))
	return 1
}

func sendTx(L *lua.LState) int {
	txn := checkTransaction(L, 1)

	var buffer bytes.Buffer
	err := txn.Serialize(&buffer)
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
	txHex := hex.EncodeToString(buffer.Bytes())

	result, err := cmdcom.RPCCall("sendrawtransaction", http.Params{
		"data": txHex,
	})
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}

	L.Push(lua.LString(result.(string)))

	return 1
}

func getAssetID(L *lua.LState) int {
	L.Push(lua.LString("a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0"))
	return 1
}

func setArbitrators(L *lua.LState) int {
	arbitrators := checkArbitrators(L, 1)
	blockchain.DefaultLedger.Arbitrators = arbitrators

	return 0
}

func initLedger(L *lua.LState) int {
	chainParams := &config.DefaultParams
	logLevel := uint8(L.ToInt(1))

	log.NewDefault(test.NodeLogPath, logLevel, 0, 0)
	dlog.Init("elastos", logLevel, 0, 0)

	ledger := blockchain.Ledger{}
	chainStore, err := blockchain.NewChainStore(test.DataPath, chainParams)
	if err != nil {
		fmt.Printf("Init chain store error: %s \n", err.Error())
	}

	arbiters, err := state.NewArbitrators(chainParams,
		nil, nil)
	if err != nil {
		fmt.Printf("New arbitrators error: %s \n", err.Error())
	}
	arbiters.RegisterFunction(chainStore.GetHeight,
		func(height uint32) (*types.Block, error) {
			return nil, nil
		}, nil)

	var interrupt = signal.NewInterrupt()
	chain, err := blockchain.New(chainStore, chainParams,
		state.NewState(chainParams, arbiters.GetArbitrators,
			nil), nil)
	if err != nil {
		fmt.Printf("Init block chain error: %s \n", err.Error())
	}
	err = chain.Init(nil)
	if err != nil {
		fmt.Printf("Init index manager error: %s \n", err.Error())
	}
	err = chain.MigrateOldDB(interrupt.C, nil, nil, test.DataPath, chainParams)
	if err != nil {
		fmt.Printf("Init fflDB error: %s \n", err.Error())
	}

	blockchain.FoundationAddress = chainParams.Foundation
	blockchain.DefaultLedger = &ledger // fixme
	blockchain.DefaultLedger.Blockchain = chain
	blockchain.DefaultLedger.Store = chainStore
	blockchain.DefaultLedger.Arbitrators = arbiters

	err = chain.InitCheckpoint(interrupt.C, nil, nil)
	if err != nil {
		fmt.Printf("Init producers state error: %s \n", err.Error())
	}

	return 1
}

func closeStore(L *lua.LState) int {
	blockchain.DefaultLedger.Store.Close()

	return 0
}

func clearStore(L *lua.LState) int {
	os.RemoveAll(test.DataDir)
	return 0
}

func walkDir(dirPth, suffix string) (files []string, err error) {
	files = make([]string, 0, 30)
	suffix = strings.ToUpper(suffix)
	err = filepath.Walk(dirPth, func(filename string, fi os.FileInfo, err error) error {
		if fi.IsDir() {
			return nil
		}
		if strings.HasSuffix(strings.ToUpper(fi.Name()), suffix) {
			files = append(files, filename)
		}
		return nil
	})
	return files, err
}

func RegisterDataType(L *lua.LState) int {
	RegisterClientType(L)
	RegisterAttributeType(L)
	RegisterInputType(L)
	RegisterOutputType(L)
	RegisterDefaultOutputType(L)
	RegisterVoteOutputType(L)
	RegisterVoteContentType(L)
	RegisterTransactionType(L)
	RegisterCoinBaseType(L)
	RegisterTransferAssetType(L)
	RegisterTransactionType(L)
	RegisterReturnDepositCoinType(L)
	RegisterProposalType(L)
	RegisterVoteType(L)
	RegisterConfirmType(L)
	RegisterBlockType(L)
	RegisterHeaderType(L)
	RegisterDposNetworkType(L)
	RegisterDposManagerType(L)
	RegisterArbitratorsType(L)
	RegisterRegisterProducerType(L)
	RegisterUpdateProducerType(L)
	RegisterCancelProducerType(L)
	RegisterActivateProducerType(L)
	RegisterIllegalProposalsType(L)
	RegisterIllegalVotesType(L)
	RegisterIllegalBlocksType(L)
	RegisterStringsType(L)
	RegisterSidechainPowType(L)
	RegisterProgramType(L)
	RegisterRegisterCRType(L)
	RegisterUpdateCRType(L)
	RegisterUnregisterCRType(L)
	RegisterCRCProposalType(L)
	RegisterCRChangeProposalOwnerType(L)
	RegisterCRCCloseProposalHashType(L)
	RegisterCRCProposalReviewType(L)
	RegisterCRCProposalWithdrawType(L)

	RegisterCRCProposalTrackingType(L)
	return 0
}
