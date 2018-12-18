package servers

import (
	"bytes"
	"encoding/binary"
	"encoding/json"
	"fmt"
	"math"
	"time"

	aux "github.com/elastos/Elastos.ELA/auxpow"
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/blockchain/interfaces"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/contract"
	. "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	. "github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/elanet"
	. "github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/mempool"
	"github.com/elastos/Elastos.ELA/p2p/msg"
	"github.com/elastos/Elastos.ELA/p2p/peer"
	"github.com/elastos/Elastos.ELA/pow"
)

const (
	AUXBLOCK_GENERATED_INTERVAL_SECONDS = 5
)

var (
	Chain     *blockchain.BlockChain
	Store     blockchain.IChainStore
	TxMemPool *mempool.TxPool
	Pow       *pow.PowService
	Server    elanet.Server
	Arbiters  interfaces.Arbitrators
	Versions  interfaces.HeightVersions
)

var preChainHeight uint32
var preTime int64
var currentAuxBlock *Block

func ToReversedString(hash common.Uint256) string {
	return common.BytesToHexString(common.BytesReverse(hash[:]))
}

func FromReversedString(reversed string) ([]byte, error) {
	bytes, err := common.HexStringToBytes(reversed)
	return common.BytesReverse(bytes), err
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
		outputs[i].OutputType = uint32(v.OutputType)
		outputs[i].OutputPayload = getOutputPayloadInfo(v.OutputPayload)
	}

	attributes := make([]AttributeInfo, len(tx.Attributes))
	for i, v := range tx.Attributes {
		attributes[i].Usage = v.Usage
		attributes[i].Data = common.BytesToHexString(v.Data)
	}

	programs := make([]ProgramInfo, len(tx.Programs))
	for i, v := range tx.Programs {
		programs[i].Code = common.BytesToHexString(v.Code)
		programs[i].Parameter = common.BytesToHexString(v.Parameter)
	}

	var txHash = tx.Hash()
	var txHashStr = ToReversedString(txHash)
	var size = uint32(tx.GetSize())
	var blockHash string
	var confirmations uint32
	var time uint32
	var blockTime uint32
	if header != nil {
		confirmations = Store.GetHeight() - header.Height + 1
		blockHash = ToReversedString(header.Hash())
		time = header.Timestamp
		blockTime = header.Timestamp
	}

	return &TransactionInfo{
		TxID:           txHashStr,
		Hash:           txHashStr,
		Size:           size,
		VSize:          size,
		Version:        tx.Version,
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
	var hash common.Uint256
	err = hash.Deserialize(bytes.NewReader(hex))
	if err != nil {
		return ResponsePack(InvalidTransaction, "")
	}

	var header *Header
	tx, height, err := Store.GetTransaction(hash)
	if err != nil {
		//try to find transaction in transaction pool.
		tx = TxMemPool.GetTransaction(hash)
		if tx == nil {
			return ResponsePack(UnknownTransaction,
				"cannot find transaction in blockchain and transactionpool")
		}
	} else {
		hash, err := Store.GetBlockHash(height)
		if err != nil {
			return ResponsePack(UnknownTransaction, "")
		}
		header, err = Chain.GetHeader(hash)
		if err != nil {
			return ResponsePack(UnknownTransaction, "")
		}
	}

	verbose, _ := param.Bool("verbose")
	if verbose {
		return ResponsePack(Success, GetTransactionInfo(header, tx))
	} else {
		buf := new(bytes.Buffer)
		tx.Serialize(buf)
		return ResponsePack(Success, common.BytesToHexString(buf.Bytes()))
	}
}

func GetNeighbors(param Params) map[string]interface{} {
	peers := Server.ConnectedPeers()
	neighborAddrs := make([]string, 0, len(peers))
	for _, peer := range peers {
		neighborAddrs = append(neighborAddrs, peer.ToPeer().String())
	}
	return ResponsePack(Success, neighborAddrs)
}

func GetNodeState(param Params) map[string]interface{} {
	peers := Server.ConnectedPeers()
	states := make([]*peer.StatsSnap, 0, len(peers))
	for _, peer := range peers {
		states = append(states, peer.ToPeer().StatsSnapshot())
	}
	return ResponsePack(Success, states)
}

func SetLogLevel(param Params) map[string]interface{} {
	level, ok := param.Int("level")
	if !ok || level < 0 {
		return ResponsePack(InvalidParams, "level must be an integer in 0-6")
	}

	log.SetPrintLevel(uint8(level))
	return ResponsePack(Success, fmt.Sprint("log level has been set to ", level))
}

func SubmitAuxBlock(param Params) map[string]interface{} {
	blockHashHex, ok := param.String("blockhash")
	if !ok {
		return ResponsePack(InvalidParams, "parameter blockhash not found")
	}
	auxPow, ok := param.String("auxpow")
	if !ok {
		return ResponsePack(InvalidParams, "parameter auxpow not found")
	}

	blockHash, err := common.Uint256FromHexString(blockHashHex)
	if err != nil {
		return ResponsePack(InvalidParams, "bad blockhash")
	}
	var msgAuxBlock *Block
	if msgAuxBlock, ok = Pow.AuxBlockPool.GetBlock(*blockHash); !ok {
		log.Debug("[json-rpc:SubmitAuxBlock] block hash unknown", blockHash)
		return ResponsePack(InternalError, "block hash unknown")
	}

	var aux aux.AuxPow
	buf, _ := common.HexStringToBytes(auxPow)
	if err := aux.Deserialize(bytes.NewReader(buf)); err != nil {
		log.Debug("[json-rpc:SubmitAuxBlock] auxpow deserialization failed", auxPow)
		return ResponsePack(InternalError, "auxpow deserialization failed")
	}

	msgAuxBlock.Header.AuxPow = aux
	_, _, err = Versions.AddDposBlock(&DposBlock{
		BlockFlag: true,
		Block:     msgAuxBlock,
	})
	if err != nil {
		log.Debug(err)
		return ResponsePack(InternalError, "adding block failed")
	}

	log.Debug("AddBlock called finished and Pow.MsgBlock.MapNewBlock has been deleted completely")
	log.Info(auxPow, blockHash)
	return ResponsePack(Success, true)
}

func CreateAuxBlock(param Params) map[string]interface{} {
	var ok bool
	Pow.PayToAddr, ok = param.String("paytoaddress")
	if !ok {
		return ResponsePack(InvalidParams, "parameter paytoaddress not found")
	}

	if Store.GetHeight() == 0 || preChainHeight != Store.GetHeight() ||
		time.Now().Unix()-preTime > AUXBLOCK_GENERATED_INTERVAL_SECONDS {

		if preChainHeight != Store.GetHeight() {
			// Clear old blocks since they're obsolete now.
			currentAuxBlock = nil
			Pow.AuxBlockPool.ClearBlock()
		}

		// Create new block with nonce = 0
		auxBlock, err := Pow.GenerateBlock(config.Parameters.PowConfiguration.
			PayToAddr, Chain.BestChain)
		if nil != err {
			return ResponsePack(InternalError, "generate block failed")
		}

		// Update state only when CreateNewBlock succeeded
		preChainHeight = Store.GetHeight()
		preTime = time.Now().Unix()

		// Save
		currentAuxBlock = auxBlock
		Pow.AuxBlockPool.AppendBlock(auxBlock)
	}

	// At this point, currentAuxBlock is always initialised: If we make it here without creating
	// a new block above, it means that, in particular, preChainHeight == ServerNode.Height().
	// But for that to happen, we must already have created a currentAuxBlock in a previous call,
	// as preChainHeight is initialised only when currentAuxBlock is.
	if currentAuxBlock == nil {
		return ResponsePack(InternalError, "no block cached")
	}

	type AuxBlock struct {
		ChainID           int            `json:"chainid"`
		Height            uint32         `json:"height"`
		CoinBaseValue     common.Fixed64 `json:"coinbasevalue"`
		Bits              string         `json:"bits"`
		Hash              string         `json:"hash"`
		PreviousBlockHash string         `json:"previousblockhash"`
	}

	SendToAux := AuxBlock{
		ChainID:           aux.AuxPowChainID,
		Height:            Store.GetHeight(),
		CoinBaseValue:     currentAuxBlock.Transactions[0].Outputs[1].Value,
		Bits:              fmt.Sprintf("%x", currentAuxBlock.Header.Bits),
		Hash:              currentAuxBlock.Hash().String(),
		PreviousBlockHash: Chain.CurrentBlockHash().String(),
	}
	return ResponsePack(Success, &SendToAux)
}

func GetInfo(param Params) map[string]interface{} {
	RetVal := struct {
		Version       int    `json:"version"`
		Balance       int    `json:"balance"`
		Blocks        uint32 `json:"blocks"`
		Timeoffset    int    `json:"timeoffset"`
		Connections   int32  `json:"connections"`
		Testnet       bool   `json:"testnet"`
		Keypoololdest int    `json:"keypoololdest"`
		Keypoolsize   int    `json:"keypoolsize"`
		UnlockedUntil int    `json:"unlocked_until"`
		Paytxfee      int    `json:"paytxfee"`
		Relayfee      int    `json:"relayfee"`
		Errors        string `json:"errors"`
	}{
		Version:       config.Parameters.Version,
		Balance:       0,
		Blocks:        Store.GetHeight(),
		Timeoffset:    0,
		Connections:   Server.ConnectedCount(),
		Keypoololdest: 0,
		Keypoolsize:   0,
		UnlockedUntil: 0,
		Paytxfee:      0,
		Relayfee:      0,
		Errors:        "Tobe written"}
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
		go Pow.Start()
		message = "mining started"
	} else {
		go Pow.Halt()
		message = "mining stopped"
	}

	return ResponsePack(Success, message)
}

func DiscreteMining(param Params) map[string]interface{} {
	if Pow == nil {
		return ResponsePack(PowServiceNotStarted, "")
	}
	count, ok := param.Uint("count")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}

	ret := make([]string, 0)

	blockHashes, err := Pow.DiscreteMining(uint32(count))
	if err != nil {
		return ResponsePack(Error, err)
	}

	for _, hash := range blockHashes {
		retStr := ToReversedString(*hash)
		ret = append(ret, retStr)
	}

	return ResponsePack(Success, ret)
}

func GetConnectionCount(param Params) map[string]interface{} {
	return ResponsePack(Success, Server.ConnectedCount())
}

func GetTransactionPool(param Params) map[string]interface{} {
	txs := make([]*TransactionInfo, 0)
	for _, tx := range TxMemPool.GetTxsInPool() {
		txs = append(txs, GetTransactionInfo(nil, tx))
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
	binary.BigEndian.PutUint32(chainWork[:], Store.GetHeight()-block.Header.Height)

	nextBlockHash, _ := Store.GetBlockHash(block.Header.Height + 1)

	auxPow := new(bytes.Buffer)
	block.Header.AuxPow.Serialize(auxPow)

	return BlockInfo{
		Hash:              ToReversedString(block.Hash()),
		Confirmations:     Store.GetHeight() - block.Header.Height + 1,
		StrippedSize:      uint32(block.GetSize()),
		Size:              uint32(block.GetSize()),
		Weight:            uint32(block.GetSize() * 4),
		Height:            block.Header.Height,
		Version:           block.Header.Version,
		VersionHex:        common.BytesToHexString(versionBytes[:]),
		MerkleRoot:        ToReversedString(block.Header.MerkleRoot),
		Tx:                txs,
		Time:              block.Header.Timestamp,
		MedianTime:        block.Header.Timestamp,
		Nonce:             block.Header.Nonce,
		Bits:              block.Header.Bits,
		Difficulty:        Chain.CalcCurrentDifficulty(block.Header.Bits),
		ChainWork:         common.BytesToHexString(chainWork[:]),
		PreviousBlockHash: ToReversedString(block.Header.Previous),
		NextBlockHash:     ToReversedString(nextBlockHash),
		AuxPow:            common.BytesToHexString(auxPow.Bytes()),
		MinerInfo:         string(block.Transactions[0].Payload.(*PayloadCoinBase).CoinbaseData[:]),
	}
}

func getBlock(hash common.Uint256, verbose uint32) (interface{}, ErrCode) {
	block, err := Store.GetBlock(hash)
	if err != nil {
		return "", UnknownBlock
	}
	switch verbose {
	case 0:
		w := new(bytes.Buffer)
		block.Serialize(w)
		return common.BytesToHexString(w.Bytes()), Success
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

	var hash common.Uint256
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

	bys, err := common.HexStringToBytes(str)
	if err != nil {
		return ResponsePack(InvalidParams, "hex string to bytes error")
	}
	var txn Transaction
	if err := txn.Deserialize(bytes.NewReader(bys)); err != nil {
		return ResponsePack(InvalidTransaction, err.Error())
	}

	if errCode := VerifyAndSendTx(&txn); errCode != Success {
		return ResponsePack(errCode, errCode.Message())
	}

	return ResponsePack(Success, ToReversedString(txn.Hash()))
}

func GetBlockHeight(param Params) map[string]interface{} {
	return ResponsePack(Success, Store.GetHeight())
}

func GetBestBlockHash(param Params) map[string]interface{} {
	hash, err := Store.GetBlockHash(Store.GetHeight())
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}
	return ResponsePack(Success, ToReversedString(hash))
}

func GetBlockCount(param Params) map[string]interface{} {
	return ResponsePack(Success, Store.GetHeight()+1)
}

func GetBlockHash(param Params) map[string]interface{} {
	height, ok := param.Uint("height")
	if !ok {
		return ResponsePack(InvalidParams, "height parameter should be a positive integer")
	}

	hash, err := Store.GetBlockHash(height)
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

	hash, err := Store.GetBlockHash(height)
	if err != nil {
		return ResponsePack(UnknownBlock, "")

	}
	block, err := Store.GetBlock(hash)
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

	hash, err := Store.GetBlockHash(height)
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

	hash, err := Store.GetBlockHash(height)
	if err != nil {
		return ResponsePack(UnknownBlock, "")
	}

	block, err := Store.GetBlock(hash)
	if err != nil {
		return ResponsePack(InternalError, "")
	}

	arbitratorsBytes := Arbiters.GetArbitrators()
	index := int(block.Header.Height) % len(arbitratorsBytes)

	var arbitrators []string
	for _, data := range arbitratorsBytes {
		arbitrators = append(arbitrators, common.BytesToHexString(data))
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
	var hash common.Uint256
	err = hash.Deserialize(bytes.NewReader(hashBytes))
	if err != nil {
		return ResponsePack(InvalidAsset, "")
	}
	asset, err := Store.GetAsset(hash)
	if err != nil {
		return ResponsePack(UnknownAsset, "")
	}
	if false {
		w := new(bytes.Buffer)
		asset.Serialize(w)
		return ResponsePack(Success, common.BytesToHexString(w.Bytes()))
	}
	return ResponsePack(Success, asset)
}

func GetBalanceByAddr(param Params) map[string]interface{} {
	str, ok := param.String("addr")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}

	programHash, err := common.Uint168FromAddress(str)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}
	unspends, err := Store.GetUnspentsFromProgramHash(*programHash)
	var balance common.Fixed64 = 0
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

	programHash, err := common.Uint168FromAddress(addr)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}

	assetIDStr, ok := param.String("assetid")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}
	assetIDBytes, err := FromReversedString(assetIDStr)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}
	assetID, err := common.Uint256FromBytes(assetIDBytes)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}

	unspents, err := Store.GetUnspentsFromProgramHash(*programHash)
	var balance common.Fixed64 = 0
	for k, u := range unspents {
		for _, v := range u {
			if assetID.IsEqual(k) {
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
	programHash, err := common.Uint168FromAddress(address)
	if err != nil {
		return ResponsePack(InvalidParams, "Invalid address: "+address)
	}
	UTXOsWithAssetID, err := Store.GetUnspentsFromProgramHash(*programHash)
	if err != nil {
		return ResponsePack(InvalidParams, err)
	}
	UTXOs := UTXOsWithAssetID[config.ELAAssetID]
	var totalValue common.Fixed64
	for _, unspent := range UTXOs {
		totalValue += unspent.Value
	}

	return ResponsePack(Success, totalValue.String())
}

func ListUnspent(param Params) map[string]interface{} {
	bestHeight := Store.GetHeight()

	var result []UTXOInfo
	addresses, ok := param.ArrayString("addresses")
	if !ok {
		return ResponsePack(InvalidParams, "need addresses in an array!")
	}
	for _, address := range addresses {
		programHash, err := common.Uint168FromAddress(address)
		if err != nil {
			return ResponsePack(InvalidParams, "Invalid address: "+address)
		}
		unspents, err := Store.GetUnspentsFromProgramHash(*programHash)
		if err != nil {
			return ResponsePack(InvalidParams, "cannot get asset with program")
		}

		for _, unspent := range unspents[config.ELAAssetID] {
			tx, height, err := Store.GetTransaction(unspent.TxID)
			if err != nil {
				return ResponsePack(InternalError,
					"unknown transaction "+unspent.TxID.String()+" from persisted utxo")
			}
			result = append(result, UTXOInfo{
				TxType:        byte(tx.TxType),
				TxID:          ToReversedString(unspent.TxID),
				AssetID:       ToReversedString(config.ELAAssetID),
				VOut:          unspent.Index,
				Amount:        unspent.Value.String(),
				Address:       address,
				OutputLock:    tx.Outputs[unspent.Index].OutputLock,
				Confirmations: bestHeight - height + 1,
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

	programHash, err := common.Uint168FromAddress(addr)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}
	type UTXOUnspentInfo struct {
		TxID  string `json:"Txid"`
		Index uint32 `json:"Index"`
		Value string `json:"Value"`
	}
	type Result struct {
		AssetID   string            `json:"AssetId"`
		AssetName string            `json:"AssetName"`
		Utxo      []UTXOUnspentInfo `json:"Utxo"`
	}
	var results []Result
	unspends, err := Store.GetUnspentsFromProgramHash(*programHash)

	for k, u := range unspends {
		asset, err := Store.GetAsset(k)
		if err != nil {
			return ResponsePack(InternalError, "")
		}
		var unspendsInfo []UTXOUnspentInfo
		for _, v := range u {
			unspendsInfo = append(unspendsInfo, UTXOUnspentInfo{ToReversedString(v.TxID), v.Index, v.Value.String()})
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
	programHash, err := common.Uint168FromAddress(addr)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}
	assetID, ok := param.String("assetid")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}
	bys, err := FromReversedString(assetID)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}

	var assetHash common.Uint256
	if err := assetHash.Deserialize(bytes.NewReader(bys)); err != nil {
		return ResponsePack(InvalidParams, "")
	}
	type UTXOUnspentInfo struct {
		TxID  string `json:"Txid"`
		Index uint32 `json:"Index"`
		Value string `json:"Value"`
	}
	infos, err := Store.GetUnspentFromProgramHash(*programHash, assetHash)
	if err != nil {
		return ResponsePack(InvalidParams, "")

	}
	var UTXOoutputs []UTXOUnspentInfo
	for _, v := range infos {
		UTXOoutputs = append(UTXOoutputs, UTXOUnspentInfo{TxID: ToReversedString(v.TxID), Index: v.Index, Value: v.Value.String()})
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

	var hash common.Uint256
	err = hash.Deserialize(bytes.NewReader(bys))
	if err != nil {
		return ResponsePack(InvalidTransaction, "")
	}
	txn, height, err := Store.GetTransaction(hash)
	if err != nil {
		return ResponsePack(UnknownTransaction, "")
	}
	if false {
		w := new(bytes.Buffer)
		txn.Serialize(w)
		return ResponsePack(Success, common.BytesToHexString(w.Bytes()))
	}
	bHash, err := Store.GetBlockHash(height)
	if err != nil {
		return ResponsePack(UnknownBlock, "")
	}
	header, err := Chain.GetHeader(bHash)
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

	txsBytes, err := common.HexStringToBytes(txsStr)
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
		txHashBytes, err := common.HexStringToBytes(txHash)
		if err != nil {
			return ResponsePack(InvalidParams, "")
		}
		hash, err := common.Uint256FromBytes(txHashBytes)
		if err != nil {
			return ResponsePack(InvalidParams, "")
		}
		inStore := Store.IsSidechainTxHashDuplicate(*hash)
		inTxPool := TxMemPool.IsDuplicateSidechainTx(*hash)
		if inTxPool || inStore {
			resultTxHashes = append(resultTxHashes, txHash)
		}
	}

	return ResponsePack(Success, resultTxHashes)
}

type Producer struct {
	Address  string `json:"address"`
	Nickname string `json:"nickname"`
	Url      string `json:"url"`
	Location uint64 `json:"location"`
	Active   bool   `json:"active"`
	Votes    string `json:"votes"`
	IP       string `json:"ip"`
}

type Producers struct {
	Producers  []Producer `json:"producers"`
	TotalVotes string     `json:"total_votes"`
}

func ListProducers(param Params) map[string]interface{} {
	start, _ := param.Int("start")
	limit, ok := param.Int("limit")
	if !ok {
		limit = math.MaxInt64
	}

	producers, err := Store.GetRegisteredProducersSorted()
	if err != nil {
		return ResponsePack(Error, "not found producer")
	}
	var ps []Producer
	for _, p := range producers {
		programHash, err := contract.PublicKeyToDepositProgramHash(p.PublicKey)
		if err != nil {
			return ResponsePack(Error, "invalid public key")
		}
		addr, err := programHash.ToAddress()
		if err != nil {
			return ResponsePack(Error, "invalid program hash")
		}
		var active bool
		state := Store.GetProducerStatus(addr)
		if state == blockchain.ProducerRegistered {
			active = true
		}
		vote := Store.GetProducerVote(p.PublicKey)
		producer := Producer{
			Address:  addr,
			Nickname: p.NickName,
			Url:      p.Url,
			Location: p.Location,
			Active:   active,
			Votes:    vote.String(),
			IP:       p.Address,
		}

		ps = append(ps, producer)
	}

	var resultPs []Producer
	var totalVotes common.Fixed64
	for i := start; i < limit && i < int64(len(ps)); i++ {
		resultPs = append(resultPs, ps[i])
		v, _ := common.StringToFixed64(ps[i].Votes)
		totalVotes += *v
	}

	result := &Producers{
		Producers:  resultPs,
		TotalVotes: totalVotes.String(),
	}

	return ResponsePack(Success, result)
}

func ProducerStatus(param Params) map[string]interface{} {
	address, ok := param.String("address")
	if !ok {
		return ResponsePack(InvalidParams, "address not found.")
	}
	return ResponsePack(Success, Store.GetProducerStatus(address))
}

func VoteStatus(param Params) map[string]interface{} {
	address, ok := param.String("address")
	if !ok {
		return ResponsePack(InvalidParams, "address not found.")
	}

	programHash, err := common.Uint168FromAddress(address)
	if err != nil {
		return ResponsePack(InvalidParams, "Invalid address: "+address)
	}
	unspents, err := Store.GetUnspentsFromProgramHash(*programHash)
	if err != nil {
		return ResponsePack(InvalidParams, "cannot get asset with program")
	}

	var total common.Fixed64
	var voting common.Fixed64
	status := true
	for _, unspent := range unspents[config.ELAAssetID] {
		if unspent.Index == 0 {
			tx, height, err := Store.GetTransaction(unspent.TxID)
			if err != nil {
				return ResponsePack(InternalError, "unknown transaction "+unspent.TxID.String()+" from persisted utxo")
			}
			if tx.Outputs[unspent.Index].OutputType == VoteOutput {
				voting += unspent.Value
			}
			bHash, err := Store.GetBlockHash(height)
			if err != nil {
				return ResponsePack(UnknownTransaction, "")
			}
			header, err := Store.GetHeader(bHash)
			if err != nil {
				return ResponsePack(UnknownTransaction, "")
			}

			if Store.GetHeight()-header.Height < 6 {
				status = false
			}
		}
		total += unspent.Value
	}

	type voteInfo struct {
		Total   string `json:"total"`
		Voting  string `json:"voting"`
		Pending bool   `json:"pending"`
	}
	return ResponsePack(Success, &voteInfo{
		Total:   total.String(),
		Voting:  voting.String(),
		Pending: status,
	})
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
		obj.Controller = common.BytesToHexString(common.BytesReverse(object.Controller.Bytes()))
		return obj
	case *PayloadSideChainPow:
		obj := new(SideChainPowInfo)
		obj.BlockHeight = object.BlockHeight
		obj.SideBlockHash = object.SideBlockHash.String()
		obj.SideGenesisHash = object.SideGenesisHash.String()
		obj.SignedData = common.BytesToHexString(object.SignedData)
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
	case *PayloadRegisterProducer:
		obj := new(RegisterProducerInfo)
		obj.PublicKey = common.BytesToHexString(object.PublicKey)
		obj.NickName = object.NickName
		obj.Url = object.Url
		obj.Location = object.Location
		obj.Address = object.Address
		return obj
	case *PayloadCancelProducer:
		obj := new(CancelProducerInfo)
		obj.PublicKey = common.BytesToHexString(object.PublicKey)
		return obj
	case *PayloadUpdateProducer:
		obj := &UpdateProducerInfo{
			new(RegisterProducerInfo),
		}
		obj.PublicKey = common.BytesToHexString(object.PublicKey)
		obj.NickName = object.NickName
		obj.Url = object.Url
		obj.Location = object.Location
		obj.Address = object.Address
		return obj
	}
	return nil
}

func getOutputPayloadInfo(op OutputPayload) OutputPayloadInfo {
	switch object := op.(type) {
	case *outputpayload.DefaultOutput:
		obj := new(DefaultOutputInfo)
		return obj
	case *outputpayload.VoteOutput:
		obj := new(VoteOutputInfo)
		obj.Version = object.Version
		for _, content := range object.Contents {
			var contentInfo VoteContentInfo
			contentInfo.VoteType = content.VoteType
			for _, candidate := range content.Candidates {
				contentInfo.CandidatesInfo = append(contentInfo.CandidatesInfo, common.BytesToHexString(candidate))
			}
			obj.Contents = append(obj.Contents, contentInfo)
		}
		return obj
	}

	return nil
}

func VerifyAndSendTx(tx *Transaction) ErrCode {
	// if transaction is verified unsuccessfully then will not put it into transaction pool
	if errCode := TxMemPool.AppendToTxnPool(tx); errCode != Success {
		log.Info("[httpjsonrpc] VerifyTransaction failed when AppendToTxnPool. Errcode:", errCode)
		return errCode
	}

	// Broadcast to peer-to-peer network.
	Server.BroadcastMessage(msg.NewTx(tx))

	return Success
}

func ResponsePack(errCode ErrCode, result interface{}) map[string]interface{} {
	if errCode != 0 && (result == "" || result == nil) {
		result = ErrMap[errCode]
	}
	return map[string]interface{}{"Result": result, "Error": errCode}
}
