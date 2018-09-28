package servers

import (
	"bytes"
	"encoding/binary"
	"encoding/json"
	"errors"
	"fmt"
	"time"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/logger"
	"github.com/elastos/Elastos.ELA.SideChain/pow"

)

const (
	Api_Getconnectioncount  = "/api/v1/node/connectioncount"
	Api_GetblockTxsByHeight = "/api/v1/block/transactions/height/:height"
	Api_Getblockbyheight    = "/api/v1/block/details/height/:height"
	Api_Getblockbyhash      = "/api/v1/block/details/hash/:hash"
	Api_Getblockheight      = "/api/v1/block/height"
	Api_Getblockhash        = "/api/v1/block/hash/:height"
	Api_GetTotalIssued      = "/api/v1/totalissued/:assetid"
	Api_Gettransaction      = "/api/v1/transaction/:hash"
	Api_Getasset            = "/api/v1/asset/:hash"
	Api_GetBalanceByAddr    = "/api/v1/asset/balances/:addr"
	Api_GetBalancebyAsset   = "/api/v1/asset/balance/:addr/:assetid"
	Api_GetUTXObyAsset      = "/api/v1/asset/utxo/:addr/:assetid"
	Api_GetUTXObyAddr       = "/api/v1/asset/utxos/:addr"
	Api_SendRawTransaction  = "/api/v1/transaction"
	Api_GetTransactionPool  = "/api/v1/transactionpool"
	Api_Restart             = "/api/v1/restart"

	AUXBLOCK_GENERATED_INTERVAL_SECONDS = 5
	DESTROY_ADDRESS                     = "0000000000000000000000000000000000"
)

var HttpServers *HttpServers

var NodeForServers Noder
var LocalPow *pow.Service
var PreChainHeight uint64
var PreTime int64
var PreTransactionCount int

type Action struct {
	Name    string
	Handler func(Params) map[string]interface{}
}

type HttpServers struct {
	RpcFunctions         []Action
	RestFulGetFunctions  map[string]Action
	RestFulPostFunctions map[string]Action
}

func InitHttpServers() {
	HttpServers = &HttpServers{}
	HttpServers.Init()
}

func (s *HttpServers) Init() {
	s.RpcFunctions = make([]Action, 0)
	s.RestFulGetFunctions = make(map[string]Action, 0)
	s.RestFulPostFunctions = make(map[string]Action, 0)

	s.RegisterRpcFunctions("setloglevel", s.setLogLevel)
	s.RegisterRpcFunctions("getinfo", s.getInfo)
	s.RegisterRpcFunctions("getblock", s.getBlockByHash)
	s.RegisterRpcFunctions("getcurrentheight", s.getBlockHeight)
	s.RegisterRpcFunctions("getblockhash", s.getBlockHash)
	s.RegisterRpcFunctions("getconnectioncount", s.getConnectionCount)
	s.RegisterRpcFunctions("getrawmempool", s.getTransactionPool)
	s.RegisterRpcFunctions("getrawtransaction", s.getRawTransaction)
	s.RegisterRpcFunctions("getneighbors", s.getNeighbors)
	s.RegisterRpcFunctions("getnodestate", s.getNodeState)
	s.RegisterRpcFunctions("sendtransactioninfo", s.sendTransactionInfo)
	s.RegisterRpcFunctions("sendrawtransaction", s.sendRawTransaction)
	s.RegisterRpcFunctions("getbestblockhash", s.getBestBlockHash)
	s.RegisterRpcFunctions("getblockcount", s.getBlockCount)
	s.RegisterRpcFunctions("getblockbyheight", s.getBlockByHeight)
	s.RegisterRpcFunctions("getdestroyedtransactions", s.getDestroyedTransactionsByHeight)
	s.RegisterRpcFunctions("getexistdeposittransactions", s.getExistDepositTransactions)
	s.RegisterRpcFunctions("help", s.auxHelp)
	s.RegisterRpcFunctions("submitsideauxblock", s.submitSideAuxBlock)
	s.RegisterRpcFunctions("createauxblock", s.createAuxBlock)
	s.RegisterRpcFunctions("togglemining", s.toggleMining)
	s.RegisterRpcFunctions("discretemining", s.discreteMining)

	s.RegisterRestfulGetFunctions("getconnectioncount", Api_Getconnectioncount, s.getConnectionCount)
	s.RegisterRestfulGetFunctions("getblocktransactionsbyheight", Api_GetblockTxsByHeight, s.getTransactionsByHeight)
	s.RegisterRestfulGetFunctions("getblockbyheight", Api_Getblockbyheight, s.getBlockByHeight)
	s.RegisterRestfulGetFunctions("getblockbyhash", Api_Getblockbyhash, s.getBlockByHash)
	s.RegisterRestfulGetFunctions("getblockheight", Api_Getblockheight, s.getBlockHeight)
	s.RegisterRestfulGetFunctions("getblockhash", Api_Getblockhash, s.getBlockHash)
	s.RegisterRestfulGetFunctions("gettransactionpool", Api_GetTransactionPool, s.getTransactionPool)
	s.RegisterRestfulGetFunctions("gettransaction", Api_Gettransaction, s.getTransactionByHash)
	s.RegisterRestfulGetFunctions("getasset", Api_Getasset, s.getAssetByHash)
	s.RegisterRestfulGetFunctions("getutxobyaddr", Api_GetUTXObyAddr, s.getUnspends)
	s.RegisterRestfulGetFunctions("getutxobyasset", Api_GetUTXObyAsset, s.getUnspendOutput)
	s.RegisterRestfulGetFunctions("getbalancebyaddr", Api_GetBalanceByAddr, s.getBalanceByAddr)
	s.RegisterRestfulGetFunctions("getbalancebyasset", Api_GetBalancebyAsset, s.getBalanceByAsset)

	s.RegisterRestfulPostFunctions("sendrawtransaction", Api_SendRawTransaction, s.sendRawTransaction)
}

func (s *HttpServers) RegisterRpcFunctions(funcName string, function func(param Params) map[string]interface{}) {
	s.RpcFunctions = append(s.RpcFunctions, Action{Name: funcName, Handler: function})
}

func (s *HttpServers) RegisterRestfulGetFunctions(funcName string, apiPath string, function func(param Params) map[string]interface{}) {
	s.RestFulGetFunctions[apiPath] = Action{Name: funcName, Handler: function}
}

func (s *HttpServers) RegisterRestfulPostFunctions(funcName string, apiPath string, function func(param Params) map[string]interface{}) {
	s.RestFulPostFunctions[apiPath] = Action{Name: funcName, Handler: function}
}

func ToReversedString(hash Uint256) string {
	return BytesToHexString(BytesReverse(hash[:]))
}

func FromReversedString(reversed string) ([]byte, error) {
	bytes, err := HexStringToBytes(reversed)
	return BytesReverse(bytes), err
}

func (s *HttpServers) getTransactionInfo(header *Header, tx *Transaction) *TransactionInfo {
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
		var address string
		destroyHash := Uint168{}
		if v.ProgramHash == destroyHash {
			address = DESTROY_ADDRESS
		} else {
			address, _ = v.ProgramHash.ToAddress()
		}
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
		confirmations = chain.DefaultChain.GetBestHeight() - header.Height + 1
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
		Payload:        s.getPayloadInfo(tx.Payload),
		Attributes:     attributes,
		Programs:       programs,
	}
}

func (s *HttpServers) getTransaction(txInfo *TransactionInfo) (*Transaction, error) {
	txPaload, err := s.getPayload(txInfo.Payload)
	if err != nil {
		return nil, err
	}

	var txAttribute []*Attribute
	for _, att := range txInfo.Attributes {
		var attData []byte
		if att.Usage == Nonce {
			attData = []byte(att.Data)
		} else {
			attData, err = HexStringToBytes(att.Data)
			if err != nil {
				return nil, err
			}
		}
		txAttr := &Attribute{
			Usage: att.Usage,
			Data:  attData,
			Size:  0,
		}
		txAttribute = append(txAttribute, txAttr)
	}

	var txUTXOTxInput []*Input
	for _, input := range txInfo.Inputs {
		txID, err := FromReversedString(input.TxID)
		if err != nil {
			return nil, err
		}
		referID, err := Uint256FromBytes(txID)
		if err != nil {
			return nil, err
		}
		utxoInput := &Input{
			Previous: OutPoint{
				TxID:  *referID,
				Index: input.VOut,
			},
			Sequence: input.Sequence,
		}
		txUTXOTxInput = append(txUTXOTxInput, utxoInput)
	}

	var txOutputs []*Output
	for _, output := range txInfo.Outputs {
		assetIdBytes, err := FromReversedString(output.AssetID)
		if err != nil {
			return nil, err
		}
		assetId, err := Uint256FromBytes(assetIdBytes)
		if err != nil {
			return nil, err
		}
		value, err := StringToFixed64(output.Value)
		if err != nil {
			return nil, err
		}
		programHash, err := Uint168FromAddress(output.Address)
		if err != nil {
			return nil, err
		}
		output := &Output{
			AssetID:     *assetId,
			Value:       *value,
			OutputLock:  output.OutputLock,
			ProgramHash: *programHash,
		}
		txOutputs = append(txOutputs, output)
	}

	var txPrograms []*Program
	for _, pgrm := range txInfo.Programs {
		code, err := HexStringToBytes(pgrm.Code)
		if err != nil {
			return nil, err
		}
		parameter, err := HexStringToBytes(pgrm.Parameter)
		if err != nil {
			return nil, err
		}
		txProgram := &Program{
			Code:      code,
			Parameter: parameter,
		}
		txPrograms = append(txPrograms, txProgram)
	}

	txTransaction := &Transaction{
		TxType:         txInfo.TxType,
		PayloadVersion: txInfo.PayloadVersion,
		Payload:        txPaload,
		Attributes:     txAttribute,
		Inputs:         txUTXOTxInput,
		Outputs:        txOutputs,
		Programs:       txPrograms,
	}
	return txTransaction, nil
}

// Input JSON string examples for getblock method as following:
func (s *HttpServers) getRawTransaction(param Params) map[string]interface{} {
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
	tx, height, err := chain.DefaultChain.GetTransaction(hash)
	if err != nil {
		return ResponsePack(UnknownTransaction, "")
	}
	bHash, err := chain.DefaultChain.GetBlockHash(height)
	if err != nil {
		return ResponsePack(UnknownTransaction, "")
	}
	header, err := chain.DefaultChain.GetHeader(bHash)
	if err != nil {
		return ResponsePack(UnknownTransaction, "")
	}

	verbose, ok := param.Bool("verbose")
	if verbose {
		return ResponsePack(Success, s.getTransactionInfo(header, tx))
	} else {
		buf := new(bytes.Buffer)
		tx.Serialize(buf)
		return ResponsePack(Success, BytesToHexString(buf.Bytes()))
	}
}

func (s *HttpServers) getNeighbors(param Params) map[string]interface{} {
	return ResponsePack(Success, NodeForServers.GetNeighborAddrs())
}

func (s *HttpServers) getNodeState(param Params) map[string]interface{} {
	n := NodeInfo{
		State:    uint(NodeForServers.State()),
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

func (s *HttpServers) setLogLevel(param Params) map[string]interface{} {
	level, ok := param["level"].(float64)
	if !ok || level < 0 {
		return ResponsePack(InvalidParams, "level must be an integer in 0-6")
	}

	if err := logger.Log.SetPrintLevel(int(level)); err != nil {
		return ResponsePack(InvalidParams, err.Error())
	}
	return ResponsePack(Success, fmt.Sprint("log level has been set to ", level))
}

func (s *HttpServers) submitSideAuxBlock(param Params) map[string]interface{} {
	blockHash, ok := param.String("blockhash")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}
	if _, ok := LocalPow.MsgBlock.BlockData[blockHash]; !ok {
		logger.Trace("[json-rpc:SubmitSideAuxBlock] receive invalid block hash value:", blockHash)
		return ResponsePack(InvalidParams, "")
	}

	sideAuxPow, ok := param.String("sideauxpow")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}

	buf, _ := HexStringToBytes(sideAuxPow)
	err := LocalPow.MsgBlock.BlockData[blockHash].Header.SideAuxPow.Deserialize(bytes.NewReader(buf))
	if err != nil {
		logger.Trace(err)
		return ResponsePack(InternalError, "[json-rpc:SubmitSideAuxBlock] deserialize side aux pow failed")
	}

	inMainChain, isOrphan, err := chain.DefaultChain.AddBlock(LocalPow.MsgBlock.BlockData[blockHash])
	if err != nil {
		logger.Trace(err)
		return ResponsePack(InternalError, "")
	}

	if isOrphan || !inMainChain {
		return ResponsePack(InternalError, "")
	}
	LocalPow.BroadcastBlock(LocalPow.MsgBlock.BlockData[blockHash])

	LocalPow.MsgBlock.Mutex.Lock()
	for key := range LocalPow.MsgBlock.BlockData {
		delete(LocalPow.MsgBlock.BlockData, key)
	}
	LocalPow.MsgBlock.Mutex.Unlock()
	logger.Trace("AddBlock called finished and LocalPow.MsgBlock.BlockData has been deleted completely")

	logger.Info(sideAuxPow, blockHash)
	return ResponsePack(Success, blockHash)
}

func (s *HttpServers) generateAuxBlock(addr string) (*Block, string, bool) {
	msgBlock := &Block{}
	if NodeForServers.Height() == 0 || PreChainHeight != NodeForServers.Height() ||
		time.Now().Unix()-PreTime > AUXBLOCK_GENERATED_INTERVAL_SECONDS {
		if PreChainHeight != NodeForServers.Height() {
			PreChainHeight = NodeForServers.Height()
			PreTime = time.Now().Unix()
			PreTransactionCount = LocalPow.GetTransactionCount()
		}

		currentTxsCount := LocalPow.CollectTransactions(msgBlock)
		if 0 == currentTxsCount {
			// return nil, "currentTxs is nil", false
		}

		msgBlock, err := LocalPow.GenerateBlock(addr)
		if nil != err {
			return nil, "msgBlock generate err", false
		}

		curHash := msgBlock.Hash()
		curHashStr := BytesToHexString(curHash.Bytes())

		LocalPow.MsgBlock.Mutex.Lock()
		LocalPow.MsgBlock.BlockData[curHashStr] = msgBlock
		LocalPow.MsgBlock.Mutex.Unlock()

		PreChainHeight = NodeForServers.Height()
		PreTime = time.Now().Unix()
		PreTransactionCount = currentTxsCount // Don't Call GetTransactionCount()

		return msgBlock, curHashStr, true
	}
	return nil, "", false
}

func (s *HttpServers) createAuxBlock(param Params) map[string]interface{} {
	addr, ok := param.String("paytoaddress")
	if !ok {
		addr = config.Parameters.PowConfiguration.PayToAddr
	}

	msgBlock, curHashStr, _ := s.generateAuxBlock(addr)
	if nil == msgBlock {
		return ResponsePack(UnknownBlock, "")
	}

	type SideAuxBlock struct {
		GenesisHash       string `json:"genesishash"`
		Height            uint32 `json:"height"`
		Bits              string `json:"bits"`
		Hash              string `json:"hash"`
		PreviousBlockHash string `json:"previousblockhash"`
	}

	LocalPow.PayToAddr = addr

	genesisHash, err := chain.DefaultChain.GetBlockHash(uint32(0))
	if err != nil {
		return ResponsePack(Error, "Get genesis hash failed")
	}
	genesisHashStr := BytesToHexString(genesisHash.Bytes())

	preHash := chain.DefaultChain.CurrentBlockHash()
	preHashStr := BytesToHexString(preHash.Bytes())

	SendToAux := SideAuxBlock{
		GenesisHash:       genesisHashStr,
		Height:            chain.DefaultChain.GetBestHeight(),
		Bits:              fmt.Sprintf("%x", msgBlock.Bits), //difficulty
		Hash:              curHashStr,
		PreviousBlockHash: preHashStr,
	}
	return ResponsePack(Success, &SendToAux)
}

func (s *HttpServers) getInfo(param Params) map[string]interface{} {
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
		Connections:    NodeForServers.GetConnectionCnt(),
		Testnet:        config.Parameters.PowConfiguration.TestNet,
		Keypoololdest:  0,
		Keypoolsize:    0,
		Unlocked_until: 0,
		Paytxfee:       0,
		Relayfee:       0,
		Errors:         "Tobe written"}
	return ResponsePack(Success, &RetVal)
}

func (s *HttpServers) auxHelp(param Params) map[string]interface{} {

	//TODO  and description for this rpc-interface
	return ResponsePack(Success, "createauxblock==submitsideauxblock")
}

func (s *HttpServers) toggleMining(param Params) map[string]interface{} {
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

func (s *HttpServers) discreteMining(param Params) map[string]interface{} {
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

func (s *HttpServers) getConnectionCount(param Params) map[string]interface{} {
	return ResponsePack(Success, NodeForServers.GetConnectionCnt())
}

func (s *HttpServers) getTransactionPool(param Params) map[string]interface{} {
	txs := make([]*TransactionInfo, 0)
	for _, t := range NodeForServers.GetTxsInPool() {
		txs = append(txs, s.getTransactionInfo(nil, t))
	}
	return ResponsePack(Success, txs)
}

func (s *HttpServers) getBlockInfo(block *Block, verbose bool) BlockInfo {
	var txs []interface{}
	if verbose {
		for _, tx := range block.Transactions {
			txs = append(txs, s.getTransactionInfo(&block.Header, tx))
		}
	} else {
		for _, tx := range block.Transactions {
			txs = append(txs, ToReversedString(tx.Hash()))
		}
	}
	var versionBytes [4]byte
	binary.BigEndian.PutUint32(versionBytes[:], block.Header.Version)

	var chainWork [4]byte
	binary.BigEndian.PutUint32(chainWork[:], chain.DefaultChain.GetBestHeight()-block.Header.Height)

	nextBlockHash, _ := chain.DefaultChain.GetBlockHash(block.Header.Height + 1)

	auxPow := new(bytes.Buffer)
	block.Header.SideAuxPow.Serialize(auxPow)

	return BlockInfo{
		Hash:              ToReversedString(block.Hash()),
		Confirmations:     chain.DefaultChain.GetBestHeight() - block.Header.Height + 1,
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

func (s *HttpServers) getBlock(hash Uint256, format uint32) (interface{}, ErrCode) {
	block, err := chain.DefaultChain.GetBlockWithHash(hash)

	if err != nil {
		return "", UnknownBlock
	}
	switch format {
	case 0:
		w := new(bytes.Buffer)
		block.Serialize(w)
		return BytesToHexString(w.Bytes()), Success
	case 2:
		return s.getBlockInfo(block, true), Success
	}
	return s.getBlockInfo(block, false), Success
}

func (s *HttpServers) getBlockByHash(param Params) map[string]interface{} {
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

	result, error := s.getBlock(hash, verbosity)

	return ResponsePack(error, result)
}

func (s *HttpServers) sendTransactionInfo(param Params) map[string]interface{} {

	infoStr, ok := param.String("Info")
	if !ok {
		return ResponsePack(InvalidParams, "Info not found")
	}

	infoBytes, err := HexStringToBytes(infoStr)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}

	txInfo, err := s.getTransactionInfoFromBytes(infoBytes)

	if err != nil {
		return ResponsePack(InvalidParams, "")
	}

	txn, err := s.getTransaction(txInfo)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}
	var hash Uint256
	hash = txn.Hash()
	if errCode := s.verifyAndSendTx(txn); errCode != Success {
		return ResponsePack(errCode, "")
	}
	return ResponsePack(Success, hash.String())
}

func (s *HttpServers) sendRawTransaction(param Params) map[string]interface{} {
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

	if errCode := s.verifyAndSendTx(&txn); errCode != Success {
		return ResponsePack(errCode, errCode.Message())
	}

	return ResponsePack(Success, ToReversedString(txn.Hash()))
}

func (s *HttpServers) getBlockHeight(param Params) map[string]interface{} {
	return ResponsePack(Success, chain.DefaultChain.GetBestHeight())
}

func (s *HttpServers) getBestBlockHash(param Params) map[string]interface{} {
	height, ok := param.Uint("height")
	if !ok {
		return ResponsePack(InvalidParams, "height parameter should be a positive integer")
	}

	hash, err := chain.DefaultChain.GetBlockHash(height)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}
	return ResponsePack(Success, ToReversedString(hash))
}

func (s *HttpServers) getBlockCount(param Params) map[string]interface{} {
	return ResponsePack(Success, chain.DefaultChain.GetBestHeight()+1)
}

func (s *HttpServers) getBlockHash(param Params) map[string]interface{} {
	height, ok := param.Uint("height")
	if !ok {
		return ResponsePack(InvalidParams, "height parameter should be a positive integer")
	}

	hash, err := chain.DefaultChain.GetBlockHash(height)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}
	return ResponsePack(Success, ToReversedString(hash))
}

func (s *HttpServers) getBlockTransactions(block *Block) interface{} {
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

func (s *HttpServers) getTransactionsByHeight(param Params) map[string]interface{} {
	height, ok := param.Uint("index")
	if !ok {
		return ResponsePack(InvalidParams, "index parameter should be a positive integer")
	}

	hash, err := chain.DefaultChain.GetBlockHash(uint32(height))
	if err != nil {
		return ResponsePack(UnknownBlock, "")

	}
	block, err := chain.DefaultChain.GetBlockByHash(hash)
	if err != nil {
		return ResponsePack(UnknownBlock, "")
	}
	return ResponsePack(Success, s.getBlockTransactions(block))
}

func (s *HttpServers) getBlockByHeight(param Params) map[string]interface{} {
	height, ok := param.Uint("height")
	if !ok {
		return ResponsePack(InvalidParams, "height parameter should be a positive integer")
	}

	hash, err := chain.DefaultChain.GetBlockHash(uint32(height))
	if err != nil {
		return ResponsePack(UnknownBlock, "")
	}

	result, errCode := s.getBlock(hash, 2)

	return ResponsePack(errCode, result)
}

//Asset
func (s *HttpServers) getAssetByHash(param Params) map[string]interface{} {
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
	asset, err := chain.DefaultChain.GetAsset(hash)
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

func (s *HttpServers) getBalanceByAddr(param Params) map[string]interface{} {
	str, ok := param.String("addr")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}

	programHash, err := Uint168FromAddress(str)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}
	unspends, err := chain.DefaultChain.GetUnspents(*programHash)
	var balance Fixed64 = 0
	for _, u := range unspends {
		for _, v := range u {
			balance = balance + v.Value
		}
	}
	return ResponsePack(Success, balance.String())
}

func (s *HttpServers) getBalanceByAsset(param Params) map[string]interface{} {
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

	unspents, err := chain.DefaultChain.GetUnspents(*programHash)
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

func (s *HttpServers) getUnspends(param Params) map[string]interface{} {
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
	unspends, err := chain.DefaultChain.GetUnspents(*programHash)

	for k, u := range unspends {
		asset, err := chain.DefaultChain.GetAsset(k)
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

func (s *HttpServers) getUnspendOutput(param Params) map[string]interface{} {
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
	infos, err := chain.DefaultChain.GetAssetUnspents(*programHash, assetHash)
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
func (s *HttpServers) getTransactionByHash(param Params) map[string]interface{} {
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
	txn, height, err := chain.DefaultChain.GetTransaction(hash)
	if err != nil {
		return ResponsePack(UnknownTransaction, "")
	}
	bHash, err := chain.DefaultChain.GetBlockHash(height)
	if err != nil {
		return ResponsePack(UnknownBlock, "")
	}
	header, err := chain.DefaultChain.GetHeader(bHash)
	if err != nil {
		return ResponsePack(UnknownBlock, "")
	}

	return ResponsePack(Success, s.getTransactionInfo(header, txn))
}

func (s *HttpServers) getExistDepositTransactions(param Params) map[string]interface{} {
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
		inStore := chain.DefaultChain.IsDuplicateMainchainTx(*hash)
		inTxPool := NodeForServers.IsDuplicateMainchainTx(*hash)
		if inTxPool || inStore {
			resultTxHashes = append(resultTxHashes, txHash)
		}
	}

	return ResponsePack(Success, resultTxHashes)
}

func (s *HttpServers) getBlockTransactionsDetail(block *Block, filter func(*Transaction) bool) interface{} {
	var trans []*TransactionInfo
	for _, tx := range block.Transactions {
		if !filter(tx) {
			continue
		}

		trans = append(trans, s.getTransactionInfo(&block.Header, tx))
	}
	hash := block.Hash()
	type BlockTransactions struct {
		Hash         string
		Height       uint32
		Transactions []*TransactionInfo
	}
	b := BlockTransactions{
		Hash:         hash.String(),
		Height:       block.Height,
		Transactions: trans,
	}
	return b
}

func (s *HttpServers) getDestroyedTransactionsByHeight(param Params) map[string]interface{} {
	height, ok := param.Uint("height")
	if !ok {
		return ResponsePack(InvalidParams, "height parameter should be a positive integer")
	}

	hash, err := chain.DefaultChain.GetBlockHash(uint32(height))
	if err != nil {
		return ResponsePack(UnknownBlock, "")

	}
	block, err := chain.DefaultChain.GetBlockByHash(hash)
	if err != nil {
		return ResponsePack(UnknownBlock, "")
	}

	destroyHash := Uint168{}
	return ResponsePack(Success, s.getBlockTransactionsDetail(block, func(tran *Transaction) bool {
		_, ok := tran.Payload.(*PayloadTransferCrossChainAsset)
		if !ok {
			return false
		}
		for _, output := range tran.Outputs {
			if output.ProgramHash == destroyHash {
				return true
			}
		}
		return false
	}))
}

func (s *HttpServers) getPayload(pInfo PayloadInfo) (Payload, error) {

	switch object := pInfo.(type) {
	case *RegisterAssetInfo:
		obj := new(PayloadRegisterAsset)
		obj.Asset = object.Asset
		amount, err := StringToFixed64(object.Amount)
		if err != nil {
			return nil, err
		}
		obj.Amount = *amount
		bytes, err := HexStringToBytes(object.Controller)
		if err != nil {
			return nil, err
		}
		controller, err := Uint168FromBytes(BytesReverse(bytes))
		obj.Controller = *controller
		return obj, nil
	case *RechargeToSideChainInfo:
		obj := new(PayloadRechargeToSideChain)
		proofBytes, err := HexStringToBytes(object.Proof)
		if err != nil {
			return nil, err
		}
		obj.MerkleProof = proofBytes
		transactionBytes, err := HexStringToBytes(object.MainChainTransaction)
		if err != nil {
			return nil, err
		}
		obj.MainChainTransaction = transactionBytes
		return obj, nil
	case *TransferCrossChainAssetInfo:
		obj := new(PayloadTransferCrossChainAsset)
		obj.CrossChainAddresses = object.CrossChainAddresses
		obj.OutputIndexes = object.OutputIndexes
		obj.CrossChainAmounts = object.CrossChainAmounts
		return obj, nil
	}

	return nil, errors.New("Invalid payload type.")
}

func (s *HttpServers) getPayloadInfo(p Payload) PayloadInfo {
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
	case *PayloadTransferCrossChainAsset:
		obj := new(TransferCrossChainAssetInfo)
		obj.CrossChainAddresses = object.CrossChainAddresses
		obj.OutputIndexes = object.OutputIndexes
		obj.CrossChainAmounts = object.CrossChainAmounts
		return obj
	case *PayloadTransferAsset:
	case *PayloadRecord:
	case *PayloadRechargeToSideChain:
		obj := new(RechargeToSideChainInfo)
		obj.MainChainTransaction = BytesToHexString(object.MainChainTransaction)
		obj.Proof = BytesToHexString(object.MerkleProof)
		return obj
	}
	return nil
}

func Unmarshal(result interface{}, target interface{}) error {
	data, err := json.Marshal(result)
	if err != nil {
		return err
	}
	err = json.Unmarshal(data, target)
	if err != nil {
		return err
	}
	return nil
}

func (s *HttpServers) getTransactionInfoFromBytes(txInfoBytes []byte) (*TransactionInfo, error) {
	var txInfo TransactionInfo
	err := json.Unmarshal(txInfoBytes, &txInfo)
	if err != nil {
		return nil, errors.New("InvalidParameter")
	}

	var assetInfo PayloadInfo
	switch txInfo.TxType {
	case CoinBase:
		assetInfo = &CoinbaseInfo{}
	case RegisterAsset:
		assetInfo = &RegisterAssetInfo{}
	case SideChainPow:
		assetInfo = &SideChainPowInfo{}
	case RechargeToSideChain:
		assetInfo = &RechargeToSideChainInfo{}
	case TransferCrossChainAsset:
		assetInfo = &TransferCrossChainAssetInfo{}
	default:
		return nil, errors.New("getBlockTransactions: Unknown payload type")
	}
	err = Unmarshal(&txInfo.Payload, assetInfo)
	if err == nil {
		txInfo.Payload = assetInfo
	}

	return &txInfo, nil
}

func (s *HttpServers) verifyAndSendTx(txn *Transaction) ErrCode {
	// if transaction is verified unsucessfully then will not put it into transaction pool
	if errCode := NodeForServers.AppendToTxnPool(txn); errCode != Success {
		logger.Warn("Can NOT add the transaction to TxnPool")
		logger.Info("[httpjsonrpc] VerifyTransaction failed when AppendToTxnPool.")
		return errCode
	}
	if err := NodeForServers.Relay(nil, txn); err != nil {
		logger.Error("Xmit Tx Error:Relay transaction failed.", err)
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
