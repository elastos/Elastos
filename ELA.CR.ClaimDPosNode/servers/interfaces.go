package servers

import (
	"bytes"
	"encoding/binary"
	"encoding/json"
	"fmt"
	"time"

	aux "github.com/elastos/Elastos.ELA/auxpow"
	chain "github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/config"
	. "github.com/elastos/Elastos.ELA/core"
	. "github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/log"
	"github.com/elastos/Elastos.ELA/pow"
	. "github.com/elastos/Elastos.ELA/protocol"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

const (
	AUXBLOCK_GENERATED_INTERVAL_SECONDS = 60
)

var NodeForServers Noder
var LocalPow *pow.PowService
var PreChainHeight uint64
var PreTime int64

func ToReversedString(hash Uint256) string {
	return BytesToHexString(BytesReverse(hash[:]))
}

func FromReversedString(reversed string) ([]byte, error) {
	bytes, err := HexStringToBytes(reversed)
	return BytesReverse(bytes), err
}

func GetTransactionInfo(header *Header, tx *Transaction) *TransactionInfo {
	inputs := make([]InputInfo, len(tx.Inputs))
	for i, v := range tx.Inputs {
		inputs[i].TxID = ToReversedString(v.Previous.TxID)
		inputs[i].VOut = v.Previous.Index
		inputs[i].Sequence = v.Sequence
	}

	outputs := make([]OutputInfo, len(tx.Outputs))
	for i, v := range tx.Outputs {
		outputs[i].Value = v.Value.String()
		outputs[i].Index = uint32(i)
		address, _ := v.ProgramHash.ToAddress()
		outputs[i].Address = address
		outputs[i].AssetID = ToReversedString(v.AssetID)
		outputs[i].OutputLock = v.OutputLock
	}

	attributes := make([]AttributeInfo, len(tx.Attributes))
	for i, v := range tx.Attributes {
		attributes[i].Usage = v.Usage
		attributes[i].Data = BytesToHexString(v.Data)
	}

	programs := make([]ProgramInfo, len(tx.Programs))
	for i, v := range tx.Programs {
		programs[i].Code = BytesToHexString(v.Code)
		programs[i].Parameter = BytesToHexString(v.Parameter)
	}

	var txHash = tx.Hash()
	var txHashStr = ToReversedString(txHash)
	var size = uint32(tx.GetSize())
	var blockHash string
	var confirmations uint32
	var time uint32
	var blockTime uint32
	if header != nil {
		confirmations = chain.DefaultLedger.Blockchain.GetBestHeight() - header.Height + 1
		blockHash = ToReversedString(header.Hash())
		time = header.Timestamp
		blockTime = header.Timestamp
	}

	return &TransactionInfo{
		TxId:           txHashStr,
		Hash:           txHashStr,
		Size:           size,
		VSize:          size,
		Version:        0x00,
		LockTime:       tx.LockTime,
		Inputs:         inputs,
		Outputs:        outputs,
		BlockHash:      blockHash,
		Confirmations:  confirmations,
		Time:           time,
		BlockTime:      blockTime,
		TxType:         tx.TxType,
		PayloadVersion: tx.PayloadVersion,
		Payload:        getPayloadInfo(tx.Payload),
		Attributes:     attributes,
		Programs:       programs,
	}
}

// Input JSON string examples for getblock method as following:
func GetRawTransaction(param Params) map[string]interface{} {
	str, ok := param.String("txid")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}

	hex, err := FromReversedString(str)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}
	var hash Uint256
	err = hash.Deserialize(bytes.NewReader(hex))
	if err != nil {
		return ResponsePack(InvalidTransaction, "")
	}

	var header *Header
	var targetTransaction *Transaction
	tx, height, err := chain.DefaultLedger.Store.GetTransaction(hash)
	if err != nil {
		//try to find transaction in transaction pool.
		targetTransaction, ok = NodeForServers.GetTransactionPool(false)[hash]
		if !ok {
			return ResponsePack(UnknownTransaction,
				"cannot find transaction in blockchain and transactionpool")
		}
	} else {
		targetTransaction = tx
		bHash, err := chain.DefaultLedger.Store.GetBlockHash(height)
		if err != nil {
			return ResponsePack(UnknownTransaction, "")
		}
		header, err = chain.DefaultLedger.Store.GetHeader(bHash)
		if err != nil {
			return ResponsePack(UnknownTransaction, "")
		}
	}

	verbose, _ := param.Bool("verbose")
	if verbose {
		return ResponsePack(Success, GetTransactionInfo(header, targetTransaction))
	} else {
		buf := new(bytes.Buffer)
		targetTransaction.Serialize(buf)
		return ResponsePack(Success, BytesToHexString(buf.Bytes()))
	}
}

func GetNeighbors(param Params) map[string]interface{} {
	return ResponsePack(Success, NodeForServers.GetNeighbourAddresses())
}

func GetNodeState(param Params) map[string]interface{} {
	n := NodeInfo{
		State:    NodeForServers.State(),
		Time:     NodeForServers.GetTime(),
		Port:     NodeForServers.Port(),
		ID:       NodeForServers.ID(),
		Version:  NodeForServers.Version(),
		Services: NodeForServers.Services(),
		Relay:    NodeForServers.IsRelay(),
		Height:   NodeForServers.Height(),
		TxnCnt:   NodeForServers.GetTxnCnt(),
		RxTxnCnt: NodeForServers.GetRxTxnCnt(),
	}
	return ResponsePack(Success, n)
}

func SetLogLevel(param Params) map[string]interface{} {
	level, ok := param.Int("level")
	if !ok || level < 0 {
		return ResponsePack(InvalidParams, "level must be an integer in 0-6")
	}

	if err := log.Log.SetPrintLevel(int(level)); err != nil {
		return ResponsePack(InvalidParams, err.Error())
	}
	return ResponsePack(Success, fmt.Sprint("log level has been set to ", level))
}

func SubmitAuxBlock(param Params) map[string]interface{} {
	blockHash, ok := param.String("blockhash")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}
	if _, ok := LocalPow.MsgBlock.BlockData[blockHash]; !ok {
		log.Trace("[json-rpc:SubmitAuxBlock] receive invalid block hash value:", blockHash)
		return ResponsePack(InvalidParams, "")
	}

	auxPow, ok := param.String("auxpow")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}

	var aux aux.AuxPow
	buf, _ := HexStringToBytes(auxPow)
	if err := aux.Deserialize(bytes.NewReader(buf)); err != nil {
		log.Trace("[json-rpc:SubmitAuxBlock] can not resolve auxpow parameter: ", auxPow)
		return ResponsePack(InvalidParams, "can not resolve auxpow parameter")
	}

	LocalPow.MsgBlock.BlockData[blockHash].Header.AuxPow = aux
	_, _, err := chain.DefaultLedger.Blockchain.AddBlock(LocalPow.MsgBlock.BlockData[blockHash])
	if err != nil {
		log.Trace(err)
		return ResponsePack(InternalError, "")
	}

	LocalPow.MsgBlock.Mutex.Lock()
	for key := range LocalPow.MsgBlock.BlockData {
		delete(LocalPow.MsgBlock.BlockData, key)
	}
	LocalPow.MsgBlock.Mutex.Unlock()
	log.Trace("AddBlock called finished and LocalPow.MsgBlock.BlockData has been deleted completely")

	log.Info(auxPow, blockHash)
	return ResponsePack(Success, "")
}

func GenerateAuxBlock(addr string) (*Block, string, bool) {
	msgBlock := new(Block)
	if NodeForServers.Height() == 0 || PreChainHeight != NodeForServers.Height() ||
		time.Now().Unix()-PreTime > AUXBLOCK_GENERATED_INTERVAL_SECONDS {

		if PreChainHeight != NodeForServers.Height() {
			PreChainHeight = NodeForServers.Height()
			PreTime = time.Now().Unix()
		}

		currentTxsCount := LocalPow.CollectTransactions(msgBlock)
		if 0 == currentTxsCount {
			// return nil, "currentTxs is nil", false
		}

		msgBlock, err := LocalPow.GenerateBlock(addr)
		if nil != err {
			return nil, "msgBlock generate err", false
		}

		// TODO is this hash needs to reverse ?
		curHash := msgBlock.Hash()
		curHashStr := BytesToHexString(curHash.Bytes())

		LocalPow.MsgBlock.Mutex.Lock()
		LocalPow.MsgBlock.BlockData[curHashStr] = msgBlock
		LocalPow.MsgBlock.Mutex.Unlock()

		PreChainHeight = NodeForServers.Height()
		PreTime = time.Now().Unix()

		return msgBlock, curHashStr, true
	}
	return nil, "", false
}

func CreateAuxBlock(param Params) map[string]interface{} {
	msgBlock, curHashStr, _ := GenerateAuxBlock(config.Parameters.PowConfiguration.PayToAddr)
	if nil == msgBlock {
		return ResponsePack(UnknownBlock, "")
	}

	var ok bool
	LocalPow.PayToAddr, ok = param.String("paytoaddress")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}

	type AuxBlock struct {
		ChainId           int    `json:"chainid"`
		Height            uint64 `json:"height"`
		CoinBaseValue     int    `json:"coinbasevalue"`
		Bits              string `json:"bits"`
		Hash              string `json:"hash"`
		PreviousBlockHash string `json:"previousblockhash"`
	}

	preHash := chain.DefaultLedger.Blockchain.CurrentBlockHash()
	preHashStr := BytesToHexString(preHash.Bytes())

	SendToAux := AuxBlock{
		ChainId:           aux.AuxPowChainID,
		Height:            NodeForServers.Height(),
		CoinBaseValue:     1,                                       //transaction content
		Bits:              fmt.Sprintf("%x", msgBlock.Header.Bits), //difficulty
		Hash:              curHashStr,
		PreviousBlockHash: preHashStr,
	}
	return ResponsePack(Success, &SendToAux)
}

func GetInfo(param Params) map[string]interface{} {
	RetVal := struct {
		Version        int    `json:"version"`
		Balance        int    `json:"balance"`
		Blocks         uint64 `json:"blocks"`
		Timeoffset     int    `json:"timeoffset"`
		Connections    uint   `json:"connections"`
		Testnet        bool   `json:"testnet"`
		Keypoololdest  int    `json:"keypoololdest"`
		Keypoolsize    int    `json:"keypoolsize"`
		Unlocked_until int    `json:"unlocked_until"`
		Paytxfee       int    `json:"paytxfee"`
		Relayfee       int    `json:"relayfee"`
		Errors         string `json:"errors"`
	}{
		Version:        config.Parameters.Version,
		Balance:        0,
		Blocks:         NodeForServers.Height(),
		Timeoffset:     0,
		Connections:    NodeForServers.GetConnectionCount(),
		Keypoololdest:  0,
		Keypoolsize:    0,
		Unlocked_until: 0,
		Paytxfee:       0,
		Relayfee:       0,
		Errors:         "Tobe written"}
	return ResponsePack(Success, &RetVal)
}

func AuxHelp(param Params) map[string]interface{} {

	//TODO  and description for this rpc-interface
	return ResponsePack(Success, "createauxblock==submitauxblock")
}

func ToggleMining(param Params) map[string]interface{} {
	mining, ok := param.Bool("mining")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}

	var message string
	if mining {
		go LocalPow.Start()
		message = "mining started"
	} else {
		go LocalPow.Halt()
		message = "mining stopped"
	}

	return ResponsePack(Success, message)
}

func DiscreteMining(param Params) map[string]interface{} {

	if LocalPow == nil {
		return ResponsePack(PowServiceNotStarted, "")
	}
	count, ok := param.Uint("count")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}

	ret := make([]string, count)

	blockHashes, err := LocalPow.DiscreteMining(uint32(count))
	if err != nil {
		return ResponsePack(Error, err)
	}

	for i, hash := range blockHashes {
		ret[i] = ToReversedString(*hash)
	}

	return ResponsePack(Success, ret)
}

func GetConnectionCount(param Params) map[string]interface{} {
	return ResponsePack(Success, NodeForServers.GetConnectionCount())
}

func GetTransactionPool(param Params) map[string]interface{} {
	txs := make([]*TransactionInfo, 0)
	for _, t := range NodeForServers.GetTransactionPool(false) {
		txs = append(txs, GetTransactionInfo(nil, t))
	}
	return ResponsePack(Success, txs)
}

func GetBlockInfo(block *Block, verbose bool) BlockInfo {
	var txs []interface{}
	if verbose {
		for _, tx := range block.Transactions {
			txs = append(txs, GetTransactionInfo(&block.Header, tx))
		}
	} else {
		for _, tx := range block.Transactions {
			txs = append(txs, ToReversedString(tx.Hash()))
		}
	}
	var versionBytes [4]byte
	binary.BigEndian.PutUint32(versionBytes[:], block.Header.Version)

	var chainWork [4]byte
	binary.BigEndian.PutUint32(chainWork[:], chain.DefaultLedger.Blockchain.GetBestHeight()-block.Header.Height)

	nextBlockHash, _ := chain.DefaultLedger.Store.GetBlockHash(block.Header.Height + 1)

	auxPow := new(bytes.Buffer)
	block.Header.AuxPow.Serialize(auxPow)

	return BlockInfo{
		Hash:              ToReversedString(block.Hash()),
		Confirmations:     chain.DefaultLedger.Blockchain.GetBestHeight() - block.Header.Height + 1,
		StrippedSize:      uint32(block.GetSize()),
		Size:              uint32(block.GetSize()),
		Weight:            uint32(block.GetSize() * 4),
		Height:            block.Header.Height,
		Version:           block.Header.Version,
		VersionHex:        BytesToHexString(versionBytes[:]),
		MerkleRoot:        ToReversedString(block.Header.MerkleRoot),
		Tx:                txs,
		Time:              block.Header.Timestamp,
		MedianTime:        block.Header.Timestamp,
		Nonce:             block.Header.Nonce,
		Bits:              block.Header.Bits,
		Difficulty:        chain.CalcCurrentDifficulty(block.Header.Bits),
		ChainWork:         BytesToHexString(chainWork[:]),
		PreviousBlockHash: ToReversedString(block.Header.Previous),
		NextBlockHash:     ToReversedString(nextBlockHash),
		AuxPow:            BytesToHexString(auxPow.Bytes()),
	}
}

func getBlock(hash Uint256, verbose uint32) (interface{}, ErrCode) {
	block, err := chain.DefaultLedger.Store.GetBlock(hash)
	if err != nil {
		return "", UnknownBlock
	}
	switch verbose {
	case 0:
		w := new(bytes.Buffer)
		block.Serialize(w)
		return BytesToHexString(w.Bytes()), Success
	case 2:
		return GetBlockInfo(block, true), Success
	}
	return GetBlockInfo(block, false), Success
}

func GetBlockByHash(param Params) map[string]interface{} {
	str, ok := param.String("blockhash")
	if !ok {
		return ResponsePack(InvalidParams, "block hash not found")
	}

	var hash Uint256
	hashBytes, err := FromReversedString(str)
	if err != nil {
		return ResponsePack(InvalidParams, "invalid block hash")
	}
	if err := hash.Deserialize(bytes.NewReader(hashBytes)); err != nil {
		ResponsePack(InvalidParams, "invalid block hash")
	}

	verbosity, ok := param.Uint("verbosity")
	if !ok {
		verbosity = 1
	}

	result, error := getBlock(hash, verbosity)

	return ResponsePack(error, result)
}

func SendRawTransaction(param Params) map[string]interface{} {
	str, ok := param.String("data")
	if !ok {
		return ResponsePack(InvalidParams, "need a string parameter named data")
	}

	bys, err := HexStringToBytes(str)
	if err != nil {
		return ResponsePack(InvalidParams, "hex string to bytes error")
	}
	var txn Transaction
	if err := txn.Deserialize(bytes.NewReader(bys)); err != nil {
		return ResponsePack(InvalidTransaction, "transaction deserialize error")
	}

	if errCode := VerifyAndSendTx(&txn); errCode != Success {
		return ResponsePack(errCode, errCode.Message())
	}

	return ResponsePack(Success, ToReversedString(txn.Hash()))
}

func GetBlockHeight(param Params) map[string]interface{} {
	return ResponsePack(Success, chain.DefaultLedger.Blockchain.BlockHeight)
}

func GetBestBlockHash(param Params) map[string]interface{} {
	bestHeight := chain.DefaultLedger.Blockchain.BlockHeight
	return GetBlockHash(map[string]interface{}{"height": float64(bestHeight)})
}

func GetBlockCount(param Params) map[string]interface{} {
	return ResponsePack(Success, chain.DefaultLedger.Blockchain.BlockHeight+1)
}

func GetBlockHash(param Params) map[string]interface{} {
	height, ok := param.Uint("height")
	if !ok {
		return ResponsePack(InvalidParams, "height parameter should be a positive integer")
	}

	hash, err := chain.DefaultLedger.Store.GetBlockHash(height)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}
	return ResponsePack(Success, ToReversedString(hash))
}

func GetBlockTransactions(block *Block) interface{} {
	trans := make([]string, len(block.Transactions))
	for i := 0; i < len(block.Transactions); i++ {
		trans[i] = ToReversedString(block.Transactions[i].Hash())
	}
	type BlockTransactions struct {
		Hash         string
		Height       uint32
		Transactions []string
	}
	b := BlockTransactions{
		Hash:         ToReversedString(block.Hash()),
		Height:       block.Header.Height,
		Transactions: trans,
	}
	return b
}

func GetTransactionsByHeight(param Params) map[string]interface{} {
	height, ok := param.Uint("height")
	if !ok {
		return ResponsePack(InvalidParams, "height parameter should be a positive integer")
	}

	hash, err := chain.DefaultLedger.Store.GetBlockHash(uint32(height))
	if err != nil {
		return ResponsePack(UnknownBlock, "")

	}
	block, err := chain.DefaultLedger.Store.GetBlock(hash)
	if err != nil {
		return ResponsePack(UnknownBlock, "")
	}
	return ResponsePack(Success, GetBlockTransactions(block))
}

func GetBlockByHeight(param Params) map[string]interface{} {
	height, ok := param.Uint("height")
	if !ok {
		return ResponsePack(InvalidParams, "height parameter should be a positive integer")
	}

	hash, err := chain.DefaultLedger.Store.GetBlockHash(uint32(height))
	if err != nil {
		return ResponsePack(UnknownBlock, err.Error())
	}

	result, errCode := getBlock(hash, 2)

	return ResponsePack(errCode, result)
}

func GetArbitratorGroupByHeight(param Params) map[string]interface{} {
	height, ok := param.Uint("height")
	if !ok {
		return ResponsePack(InvalidParams, "height parameter should be a positive integer")
	}

	hash, err := chain.DefaultLedger.Store.GetBlockHash(uint32(height))
	if err != nil {
		return ResponsePack(UnknownBlock, "")
	}

	block, err := chain.DefaultLedger.Store.GetBlock(hash)
	if err != nil {
		return ResponsePack(InternalError, "")
	}

	arbitratorsBytes, err := config.Parameters.GetArbitrators()
	if err != nil {
		return ResponsePack(InternalError, "")
	}

	index := int(block.Header.Height) % len(arbitratorsBytes)

	var arbitrators []string
	for _, data := range arbitratorsBytes {
		arbitrators = append(arbitrators, BytesToHexString(data))
	}

	result := ArbitratorGroupInfo{
		OnDutyArbitratorIndex: index,
		Arbitrators:           arbitrators,
	}

	return ResponsePack(Success, result)
}

//Asset
func GetAssetByHash(param Params) map[string]interface{} {
	str, ok := param.String("hash")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}
	hashBytes, err := FromReversedString(str)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}
	var hash Uint256
	err = hash.Deserialize(bytes.NewReader(hashBytes))
	if err != nil {
		return ResponsePack(InvalidAsset, "")
	}
	asset, err := chain.DefaultLedger.Store.GetAsset(hash)
	if err != nil {
		return ResponsePack(UnknownAsset, "")
	}
	if false {
		w := new(bytes.Buffer)
		asset.Serialize(w)
		return ResponsePack(Success, BytesToHexString(w.Bytes()))
	}
	return ResponsePack(Success, asset)
}

func GetBalanceByAddr(param Params) map[string]interface{} {
	str, ok := param.String("addr")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}

	programHash, err := Uint168FromAddress(str)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}
	unspends, err := chain.DefaultLedger.Store.GetUnspentsFromProgramHash(*programHash)
	var balance Fixed64 = 0
	for _, u := range unspends {
		for _, v := range u {
			balance = balance + v.Value
		}
	}
	return ResponsePack(Success, balance.String())
}

func GetBalanceByAsset(param Params) map[string]interface{} {
	addr, ok := param.String("addr")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}

	programHash, err := Uint168FromAddress(addr)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}

	assetIdStr, ok := param.String("assetid")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}
	assetIdBytes, err := FromReversedString(assetIdStr)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}
	assetId, err := Uint256FromBytes(assetIdBytes)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}

	unspents, err := chain.DefaultLedger.Store.GetUnspentsFromProgramHash(*programHash)
	var balance Fixed64 = 0
	for k, u := range unspents {
		for _, v := range u {
			if assetId.IsEqual(k) {
				balance = balance + v.Value
			}
		}
	}
	return ResponsePack(Success, balance.String())
}

func GetReceivedByAddress(param Params) map[string]interface{} {
	address, ok := param.String("address")
	if !ok {
		return ResponsePack(InvalidParams, "need a parameter named address")
	}
	programHash, err := Uint168FromAddress(address)
	if err != nil {
		return ResponsePack(InvalidParams, "Invalid address: "+address)
	}
	UTXOsWithAssetID, err := chain.DefaultLedger.Store.GetUnspentsFromProgramHash(*programHash)
	if err != nil {
		return ResponsePack(InvalidParams, err)
	}
	UTXOs := UTXOsWithAssetID[chain.DefaultLedger.Blockchain.AssetID]
	var totalValue Fixed64
	for _, unspent := range UTXOs {
		totalValue += unspent.Value
	}

	return ResponsePack(Success, totalValue.String())
}

func ListUnspent(param Params) map[string]interface{} {
	bestHeight := chain.DefaultLedger.Blockchain.GetBestHeight()

	var result []UTXOInfo
	addresses, ok := param.ArrayString("addresses")
	if !ok {
		return ResponsePack(InvalidParams, "need addresses in an array!")
	}
	for _, address := range addresses {
		programHash, err := Uint168FromAddress(address)
		if err != nil {
			return ResponsePack(InvalidParams, "Invalid address: "+address)
		}
		unspents, err := chain.DefaultLedger.Store.GetUnspentsFromProgramHash(*programHash)
		if err != nil {
			return ResponsePack(InvalidParams, "cannot get asset with program")
		}

		for _, unspent := range unspents[chain.DefaultLedger.Blockchain.AssetID] {
			tx, height, err := chain.DefaultLedger.Store.GetTransaction(unspent.TxId)
			if err != nil {
				return ResponsePack(InternalError,
					"unknown transaction "+unspent.TxId.String()+" from persisted utxo")
			}
			result = append(result, UTXOInfo{
				AssetId:       ToReversedString(chain.DefaultLedger.Blockchain.AssetID),
				Txid:          ToReversedString(unspent.TxId),
				VOut:          unspent.Index,
				Amount:        unspent.Value.String(),
				Address:       address,
				Confirmations: bestHeight - height + 1,
				OutputLock:    tx.Outputs[unspent.Index].OutputLock,
			})
		}
	}
	return ResponsePack(Success, result)
}

func GetUnspends(param Params) map[string]interface{} {
	addr, ok := param.String("addr")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}

	programHash, err := Uint168FromAddress(addr)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}
	type UTXOUnspentInfo struct {
		Txid  string
		Index uint32
		Value string
	}
	type Result struct {
		AssetId   string
		AssetName string
		Utxo      []UTXOUnspentInfo
	}
	var results []Result
	unspends, err := chain.DefaultLedger.Store.GetUnspentsFromProgramHash(*programHash)

	for k, u := range unspends {
		asset, err := chain.DefaultLedger.Store.GetAsset(k)
		if err != nil {
			return ResponsePack(InternalError, "")
		}
		var unspendsInfo []UTXOUnspentInfo
		for _, v := range u {
			unspendsInfo = append(unspendsInfo, UTXOUnspentInfo{ToReversedString(v.TxId), v.Index, v.Value.String()})
		}
		results = append(results, Result{ToReversedString(k), asset.Name, unspendsInfo})
	}
	return ResponsePack(Success, results)
}

func GetUnspendOutput(param Params) map[string]interface{} {
	addr, ok := param.String("addr")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}
	programHash, err := Uint168FromAddress(addr)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}
	assetId, ok := param.String("assetid")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}
	bys, err := FromReversedString(assetId)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}

	var assetHash Uint256
	if err := assetHash.Deserialize(bytes.NewReader(bys)); err != nil {
		return ResponsePack(InvalidParams, "")
	}
	type UTXOUnspentInfo struct {
		Txid  string
		Index uint32
		Value string
	}
	infos, err := chain.DefaultLedger.Store.GetUnspentFromProgramHash(*programHash, assetHash)
	if err != nil {
		return ResponsePack(InvalidParams, "")

	}
	var UTXOoutputs []UTXOUnspentInfo
	for _, v := range infos {
		UTXOoutputs = append(UTXOoutputs, UTXOUnspentInfo{Txid: ToReversedString(v.TxId), Index: v.Index, Value: v.Value.String()})
	}
	return ResponsePack(Success, UTXOoutputs)
}

//Transaction
func GetTransactionByHash(param Params) map[string]interface{} {
	str, ok := param.String("hash")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}

	bys, err := FromReversedString(str)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}

	var hash Uint256
	err = hash.Deserialize(bytes.NewReader(bys))
	if err != nil {
		return ResponsePack(InvalidTransaction, "")
	}
	txn, height, err := chain.DefaultLedger.Store.GetTransaction(hash)
	if err != nil {
		return ResponsePack(UnknownTransaction, "")
	}
	if false {
		w := new(bytes.Buffer)
		txn.Serialize(w)
		return ResponsePack(Success, BytesToHexString(w.Bytes()))
	}
	bHash, err := chain.DefaultLedger.Store.GetBlockHash(height)
	if err != nil {
		return ResponsePack(UnknownBlock, "")
	}
	header, err := chain.DefaultLedger.Store.GetHeader(bHash)
	if err != nil {
		return ResponsePack(UnknownBlock, "")
	}

	return ResponsePack(Success, GetTransactionInfo(header, txn))
}

func GetExistWithdrawTransactions(param Params) map[string]interface{} {
	txsStr, ok := param.String("txs")
	if !ok {
		return ResponsePack(InvalidParams, "txs not found")
	}

	txsBytes, err := HexStringToBytes(txsStr)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}

	var txHashes []string
	err = json.Unmarshal(txsBytes, &txHashes)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}

	var resultTxHashes []string
	for _, txHash := range txHashes {
		txHashBytes, err := HexStringToBytes(txHash)
		if err != nil {
			return ResponsePack(InvalidParams, "")
		}
		hash, err := Uint256FromBytes(txHashBytes)
		if err != nil {
			return ResponsePack(InvalidParams, "")
		}
		inStore := chain.DefaultLedger.Store.IsSidechainTxHashDuplicate(*hash)
		inTxPool := NodeForServers.IsDuplicateSidechainTx(*hash)
		if inTxPool || inStore {
			resultTxHashes = append(resultTxHashes, txHash)
		}
	}

	return ResponsePack(Success, resultTxHashes)
}

func getPayloadInfo(p Payload) PayloadInfo {
	switch object := p.(type) {
	case *PayloadCoinBase:
		obj := new(CoinbaseInfo)
		obj.CoinbaseData = string(object.CoinbaseData)
		return obj
	case *PayloadRegisterAsset:
		obj := new(RegisterAssetInfo)
		obj.Asset = object.Asset
		obj.Amount = object.Amount.String()
		obj.Controller = BytesToHexString(BytesReverse(object.Controller.Bytes()))
		return obj
	case *PayloadSideChainPow:
		obj := new(SideChainPowInfo)
		obj.BlockHeight = object.BlockHeight
		obj.SideBlockHash = object.SideBlockHash.String()
		obj.SideGenesisHash = object.SideGenesisHash.String()
		obj.SignedData = BytesToHexString(object.SignedData)
		return obj
	case *PayloadWithdrawFromSideChain:
		obj := new(WithdrawFromSideChainInfo)
		obj.BlockHeight = object.BlockHeight
		obj.GenesisBlockAddress = object.GenesisBlockAddress
		for _, hash := range object.SideChainTransactionHashes {
			obj.SideChainTransactionHashes = append(obj.SideChainTransactionHashes, hash.String())
		}
		return obj
	case *PayloadTransferCrossChainAsset:
		obj := new(TransferCrossChainAssetInfo)
		obj.CrossChainAddresses = object.CrossChainAddresses
		obj.OutputIndexes = object.OutputIndexes
		obj.CrossChainAmounts = object.CrossChainAmounts
		return obj
	case *PayloadTransferAsset:
	case *PayloadRecord:
	}
	return nil
}

func VerifyAndSendTx(txn *Transaction) ErrCode {
	// if transaction is verified unsucessfully then will not put it into transaction pool
	if errCode := NodeForServers.AppendToTxnPool(txn); errCode != Success {
		log.Warn("Can NOT add the transaction to TxnPool")
		log.Info("[httpjsonrpc] VerifyTransaction failed when AppendToTxnPool. Errcode:", errCode)
		return errCode
	}
	if err := NodeForServers.Relay(nil, txn); err != nil {
		log.Error("Xmit Tx Error:Relay transaction failed.", err)
		return ErrXmitFail
	}
	return Success
}

func ResponsePack(errCode ErrCode, result interface{}) map[string]interface{} {
	if errCode != 0 && (result == "" || result == nil) {
		result = ErrMap[errCode]
	}
	return map[string]interface{}{"Result": result, "Error": errCode}
}
