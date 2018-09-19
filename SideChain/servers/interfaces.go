package servers

import (
	"bytes"
	"encoding/binary"
	"encoding/json"
	"errors"
	"fmt"
	"time"

	chain "github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/config"
	. "github.com/elastos/Elastos.ELA.SideChain/core"
	. "github.com/elastos/Elastos.ELA.SideChain/errors"
	"github.com/elastos/Elastos.ELA.SideChain/log"
	"github.com/elastos/Elastos.ELA.SideChain/pow"
	. "github.com/elastos/Elastos.ELA.SideChain/protocol"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

const (
	AUXBLOCK_GENERATED_INTERVAL_SECONDS = 5
	DESTROY_ADDRESS                     = "0000000000000000000000000000000000"
)

var HttpServers *HttpServersBase

var NodeForServers Noder
var LocalPow *pow.PowService
var PreChainHeight uint64
var PreTime int64
var PreTransactionCount int

type HttpServersBase struct {
	GetTransactionInfo               func(header *Header, tx *Transaction) *TransactionInfo
	GetTransaction                   func(txInfo *TransactionInfo) (*Transaction, error)
	GetRawTransaction                func(param Params) map[string]interface{}
	GetNeighbors                     func(param Params) map[string]interface{}
	GetNodeState                     func(param Params) map[string]interface{}
	SetLogLevel                      func(param Params) map[string]interface{}
	SubmitSideAuxBlock               func(param Params) map[string]interface{}
	GenerateAuxBlock                 func(addr string) (*Block, string, bool)
	CreateAuxBlock                   func(param Params) map[string]interface{}
	GetInfo                          func(param Params) map[string]interface{}
	AuxHelp                          func(param Params) map[string]interface{}
	ToggleMining                     func(param Params) map[string]interface{}
	DiscreteMining                   func(param Params) map[string]interface{}
	GetConnectionCount               func(param Params) map[string]interface{}
	GetTransactionPool               func(param Params) map[string]interface{}
	GetBlockInfo                     func(block *Block, verbose bool) BlockInfo
	GetBlock                         func(hash Uint256, format uint32) (interface{}, ErrCode)
	GetBlockByHash                   func(param Params) map[string]interface{}
	SendTransactionInfo              func(param Params) map[string]interface{}
	SendRawTransaction               func(param Params) map[string]interface{}
	GetBlockHeight                   func(param Params) map[string]interface{}
	GetBestBlockHash                 func(param Params) map[string]interface{}
	GetBlockCount                    func(param Params) map[string]interface{}
	GetBlockHash                     func(param Params) map[string]interface{}
	GetBlockTransactions             func(block *Block) interface{}
	GetTransactionsByHeight          func(param Params) map[string]interface{}
	GetBlockByHeight                 func(param Params) map[string]interface{}
	GetAssetByHash                   func(param Params) map[string]interface{}
	GetBalanceByAddr                 func(param Params) map[string]interface{}
	GetBalanceByAsset                func(param Params) map[string]interface{}
	GetUnspends                      func(param Params) map[string]interface{}
	GetUnspendOutput                 func(param Params) map[string]interface{}
	GetTransactionByHash             func(param Params) map[string]interface{}
	GetExistDepositTransactions      func(param Params) map[string]interface{}
	GetBlockTransactionsDetail       func(block *Block, filter func(*Transaction) bool) interface{}
	GetDestroyedTransactionsByHeight func(param Params) map[string]interface{}
	GetPayload                       func(pInfo PayloadInfo) (Payload, error)
	GetPayloadInfo                   func(p Payload) PayloadInfo
	GetTransactionInfoFromBytes      func(txInfoBytes []byte) (*TransactionInfo, error)
	VerifyAndSendTx                  func(txn *Transaction) ErrCode
}

func InitHttpServers() {
	HttpServers = &HttpServersBase{}
	HttpServers.Init()
}

func (s *HttpServersBase) Init() {
	s.GetTransactionInfo = s.GetTransactionInfoImpl
	s.GetTransaction = s.GetTransactionImpl
	s.GetRawTransaction = s.GetRawTransactionImpl
	s.GetNeighbors = s.GetNeighborsImpl
	s.GetNodeState = s.GetNodeStateImpl
	s.SetLogLevel = s.SetLogLevelImpl
	s.SubmitSideAuxBlock = s.SubmitSideAuxBlockImpl
	s.GenerateAuxBlock = s.GenerateAuxBlockImpl
	s.CreateAuxBlock = s.CreateAuxBlockImpl
	s.GetInfo = s.GetInfoImpl
	s.AuxHelp = s.AuxHelpImpl
	s.ToggleMining = s.ToggleMiningImpl
	s.DiscreteMining = s.DiscreteMiningImpl
	s.GetConnectionCount = s.GetConnectionCountImpl
	s.GetTransactionPool = s.GetTransactionPoolImpl
	s.GetBlockInfo = s.GetBlockInfoImpl
	s.GetBlock = s.GetBlockImpl
	s.GetBlockByHash = s.GetBlockByHashImpl
	s.SendTransactionInfo = s.SendTransactionInfoImpl
	s.SendRawTransaction = s.SendRawTransactionImpl
	s.GetBlockHeight = s.GetBlockHeightImpl
	s.GetBestBlockHash = s.GetBestBlockHashImpl
	s.GetBlockCount = s.GetBlockCountImpl
	s.GetBlockHash = s.GetBlockHashImpl
	s.GetBlockTransactions = s.GetBlockTransactionsImpl
	s.GetTransactionsByHeight = s.GetTransactionsByHeightImpl
	s.GetBlockByHeight = s.GetBlockByHeightImpl
	s.GetAssetByHash = s.GetAssetByHashImpl
	s.GetBalanceByAddr = s.GetBalanceByAddrImpl
	s.GetBalanceByAsset = s.GetBalanceByAssetImpl
	s.GetUnspends = s.GetUnspendsImpl
	s.GetUnspendOutput = s.GetUnspendOutputImpl
	s.GetTransactionByHash = s.GetTransactionByHashImpl
	s.GetExistDepositTransactions = s.GetExistDepositTransactionsImpl
	s.GetBlockTransactionsDetail = s.GetBlockTransactionsDetailImpl
	s.GetDestroyedTransactionsByHeight = s.GetDestroyedTransactionsByHeightImpl
	s.GetPayload = s.GetPayloadImpl
	s.GetPayloadInfo = s.GetPayloadInfoImpl
	s.GetTransactionInfoFromBytes = s.GetTransactionInfoFromBytesImpl
	s.VerifyAndSendTx = s.VerifyAndSendTxImpl
}

func ToReversedString(hash Uint256) string {
	return BytesToHexString(BytesReverse(hash[:]))
}

func FromReversedString(reversed string) ([]byte, error) {
	bytes, err := HexStringToBytes(reversed)
	return BytesReverse(bytes), err
}

func (s *HttpServersBase) GetTransactionInfoImpl(header *Header, tx *Transaction) *TransactionInfo {
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
		Payload:        s.GetPayloadInfo(tx.Payload),
		Attributes:     attributes,
		Programs:       programs,
	}
}

func (s *HttpServersBase) GetTransactionImpl(txInfo *TransactionInfo) (*Transaction, error) {
	txPaload, err := s.GetPayload(txInfo.Payload)
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
func (s *HttpServersBase) GetRawTransactionImpl(param Params) map[string]interface{} {
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
	tx, height, err := chain.DefaultLedger.Store.GetTransaction(hash)
	if err != nil {
		return ResponsePack(UnknownTransaction, "")
	}
	bHash, err := chain.DefaultLedger.Store.GetBlockHash(height)
	if err != nil {
		return ResponsePack(UnknownTransaction, "")
	}
	header, err := chain.DefaultLedger.Store.GetHeader(bHash)
	if err != nil {
		return ResponsePack(UnknownTransaction, "")
	}

	verbose, ok := param.Bool("verbose")
	if verbose {
		return ResponsePack(Success, s.GetTransactionInfo(header, tx))
	} else {
		buf := new(bytes.Buffer)
		tx.Serialize(buf)
		return ResponsePack(Success, BytesToHexString(buf.Bytes()))
	}
}

func (s *HttpServersBase) GetNeighborsImpl(param Params) map[string]interface{} {
	return ResponsePack(Success, NodeForServers.GetNeighborAddrs())
}

func (s *HttpServersBase) GetNodeStateImpl(param Params) map[string]interface{} {
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

func (s *HttpServersBase) SetLogLevelImpl(param Params) map[string]interface{} {
	level, ok := param["level"].(float64)
	if !ok || level < 0 {
		return ResponsePack(InvalidParams, "level must be an integer in 0-6")
	}

	if err := log.Log.SetPrintLevel(int(level)); err != nil {
		return ResponsePack(InvalidParams, err.Error())
	}
	return ResponsePack(Success, fmt.Sprint("log level has been set to ", level))
}

func (s *HttpServersBase) SubmitSideAuxBlockImpl(param Params) map[string]interface{} {
	blockHash, ok := param.String("blockhash")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}
	if _, ok := LocalPow.MsgBlock.BlockData[blockHash]; !ok {
		log.Trace("[json-rpc:SubmitSideAuxBlock] receive invalid block hash value:", blockHash)
		return ResponsePack(InvalidParams, "")
	}

	sideAuxPow, ok := param.String("sideauxpow")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}

	buf, _ := HexStringToBytes(sideAuxPow)
	err := LocalPow.MsgBlock.BlockData[blockHash].Header.SideAuxPow.Deserialize(bytes.NewReader(buf))
	if err != nil {
		log.Trace(err)
		return ResponsePack(InternalError, "[json-rpc:SubmitSideAuxBlock] deserialize side aux pow failed")
	}

	inMainChain, isOrphan, err := chain.DefaultLedger.Blockchain.AddBlock(LocalPow.MsgBlock.BlockData[blockHash])
	if err != nil {
		log.Trace(err)
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
	log.Trace("AddBlock called finished and LocalPow.MsgBlock.BlockData has been deleted completely")

	log.Info(sideAuxPow, blockHash)
	return ResponsePack(Success, blockHash)
}

func (s *HttpServersBase) GenerateAuxBlockImpl(addr string) (*Block, string, bool) {
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

func (s *HttpServersBase) CreateAuxBlockImpl(param Params) map[string]interface{} {
	addr, ok := param.String("paytoaddress")
	if !ok {
		addr = config.Parameters.PowConfiguration.PayToAddr
	}

	msgBlock, curHashStr, _ := s.GenerateAuxBlock(addr)
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

	genesisHash, err := chain.DefaultLedger.Store.GetBlockHash(uint32(0))
	if err != nil {
		return ResponsePack(Error, "Get genesis hash failed")
	}
	genesisHashStr := BytesToHexString(genesisHash.Bytes())

	preHash := chain.DefaultLedger.Blockchain.CurrentBlockHash()
	preHashStr := BytesToHexString(preHash.Bytes())

	SendToAux := SideAuxBlock{
		GenesisHash:       genesisHashStr,
		Height:            chain.DefaultLedger.Store.GetHeight(),
		Bits:              fmt.Sprintf("%x", msgBlock.Bits), //difficulty
		Hash:              curHashStr,
		PreviousBlockHash: preHashStr,
	}
	return ResponsePack(Success, &SendToAux)
}

func (s *HttpServersBase) GetInfoImpl(param Params) map[string]interface{} {
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

func (s *HttpServersBase) AuxHelpImpl(param Params) map[string]interface{} {

	//TODO  and description for this rpc-interface
	return ResponsePack(Success, "createauxblock==submitsideauxblock")
}

func (s *HttpServersBase) ToggleMiningImpl(param Params) map[string]interface{} {
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

func (s *HttpServersBase) DiscreteMiningImpl(param Params) map[string]interface{} {
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

func (s *HttpServersBase) GetConnectionCountImpl(param Params) map[string]interface{} {
	return ResponsePack(Success, NodeForServers.GetConnectionCnt())
}

func (s *HttpServersBase) GetTransactionPoolImpl(param Params) map[string]interface{} {
	txs := make([]*TransactionInfo, 0)
	for _, t := range NodeForServers.GetTxsInPool() {
		txs = append(txs, s.GetTransactionInfo(nil, t))
	}
	return ResponsePack(Success, txs)
}

func (s *HttpServersBase) GetBlockInfoImpl(block *Block, verbose bool) BlockInfo {
	var txs []interface{}
	if verbose {
		for _, tx := range block.Transactions {
			txs = append(txs, s.GetTransactionInfo(&block.Header, tx))
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
	block.Header.SideAuxPow.Serialize(auxPow)

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

func (s *HttpServersBase) GetBlockImpl(hash Uint256, format uint32) (interface{}, ErrCode) {
	block, err := chain.DefaultLedger.Store.GetBlock(hash)
	if err != nil {
		return "", UnknownBlock
	}
	switch format {
	case 0:
		w := new(bytes.Buffer)
		block.Serialize(w)
		return BytesToHexString(w.Bytes()), Success
	case 2:
		return s.GetBlockInfo(block, true), Success
	}
	return s.GetBlockInfo(block, false), Success
}

func (s *HttpServersBase) GetBlockByHashImpl(param Params) map[string]interface{} {
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

	result, error := s.GetBlock(hash, verbosity)

	return ResponsePack(error, result)
}

func (s *HttpServersBase) SendTransactionInfoImpl(param Params) map[string]interface{} {

	infoStr, ok := param.String("Info")
	if !ok {
		return ResponsePack(InvalidParams, "Info not found")
	}

	infoBytes, err := HexStringToBytes(infoStr)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}

	txInfo, err := s.GetTransactionInfoFromBytes(infoBytes)

	if err != nil {
		return ResponsePack(InvalidParams, "")
	}

	txn, err := s.GetTransaction(txInfo)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}
	var hash Uint256
	hash = txn.Hash()
	if errCode := s.VerifyAndSendTx(txn); errCode != Success {
		return ResponsePack(errCode, "")
	}
	return ResponsePack(Success, hash.String())
}

func (s *HttpServersBase) SendRawTransactionImpl(param Params) map[string]interface{} {
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

	if errCode := s.VerifyAndSendTx(&txn); errCode != Success {
		return ResponsePack(errCode, errCode.Message())
	}

	return ResponsePack(Success, ToReversedString(txn.Hash()))
}

func (s *HttpServersBase) GetBlockHeightImpl(param Params) map[string]interface{} {
	return ResponsePack(Success, chain.DefaultLedger.Blockchain.BlockHeight)
}

func (s *HttpServersBase) GetBestBlockHashImpl(param Params) map[string]interface{} {
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

func (s *HttpServersBase) GetBlockCountImpl(param Params) map[string]interface{} {
	return ResponsePack(Success, chain.DefaultLedger.Blockchain.BlockHeight+1)
}

func (s *HttpServersBase) GetBlockHashImpl(param Params) map[string]interface{} {
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

func (s *HttpServersBase) GetBlockTransactionsImpl(block *Block) interface{} {
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

func (s *HttpServersBase) GetTransactionsByHeightImpl(param Params) map[string]interface{} {
	height, ok := param.Uint("index")
	if !ok {
		return ResponsePack(InvalidParams, "index parameter should be a positive integer")
	}

	hash, err := chain.DefaultLedger.Store.GetBlockHash(uint32(height))
	if err != nil {
		return ResponsePack(UnknownBlock, "")

	}
	block, err := chain.DefaultLedger.Store.GetBlock(hash)
	if err != nil {
		return ResponsePack(UnknownBlock, "")
	}
	return ResponsePack(Success, s.GetBlockTransactions(block))
}

func (s *HttpServersBase) GetBlockByHeightImpl(param Params) map[string]interface{} {
	height, ok := param.Uint("height")
	if !ok {
		return ResponsePack(InvalidParams, "height parameter should be a positive integer")
	}

	hash, err := chain.DefaultLedger.Store.GetBlockHash(uint32(height))
	if err != nil {
		return ResponsePack(UnknownBlock, "")
	}

	result, errCode := s.GetBlock(hash, 2)

	return ResponsePack(errCode, result)
}

//Asset
func (s *HttpServersBase) GetAssetByHashImpl(param Params) map[string]interface{} {
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

func (s *HttpServersBase) GetBalanceByAddrImpl(param Params) map[string]interface{} {
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

func (s *HttpServersBase) GetBalanceByAssetImpl(param Params) map[string]interface{} {
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

func (s *HttpServersBase) GetUnspendsImpl(param Params) map[string]interface{} {
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

func (s *HttpServersBase) GetUnspendOutputImpl(param Params) map[string]interface{} {
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
func (s *HttpServersBase) GetTransactionByHashImpl(param Params) map[string]interface{} {
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
	bHash, err := chain.DefaultLedger.Store.GetBlockHash(height)
	if err != nil {
		return ResponsePack(UnknownBlock, "")
	}
	header, err := chain.DefaultLedger.Store.GetHeader(bHash)
	if err != nil {
		return ResponsePack(UnknownBlock, "")
	}

	return ResponsePack(Success, s.GetTransactionInfo(header, txn))
}

func (s *HttpServersBase) GetExistDepositTransactionsImpl(param Params) map[string]interface{} {
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
		inStore := chain.DefaultLedger.Store.IsMainchainTxHashDuplicate(*hash)
		inTxPool := NodeForServers.IsDuplicateMainchainTx(*hash)
		if inTxPool || inStore {
			resultTxHashes = append(resultTxHashes, txHash)
		}
	}

	return ResponsePack(Success, resultTxHashes)
}

func (s *HttpServersBase) GetBlockTransactionsDetailImpl(block *Block, filter func(*Transaction) bool) interface{} {
	var trans []*TransactionInfo
	for _, tx := range block.Transactions {
		if !filter(tx) {
			continue
		}

		trans = append(trans, s.GetTransactionInfo(&block.Header, tx))
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

func (s *HttpServersBase) GetDestroyedTransactionsByHeightImpl(param Params) map[string]interface{} {
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

	destroyHash := Uint168{}
	return ResponsePack(Success, s.GetBlockTransactionsDetail(block, func(tran *Transaction) bool {
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

func (s *HttpServersBase) GetPayloadImpl(pInfo PayloadInfo) (Payload, error) {

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

func (s *HttpServersBase) GetPayloadInfoImpl(p Payload) PayloadInfo {
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

func (s *HttpServersBase) GetTransactionInfoFromBytesImpl(txInfoBytes []byte) (*TransactionInfo, error) {
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
		return nil, errors.New("GetBlockTransactions: Unknown payload type")
	}
	err = Unmarshal(&txInfo.Payload, assetInfo)
	if err == nil {
		txInfo.Payload = assetInfo
	}

	return &txInfo, nil
}

func (s *HttpServersBase) VerifyAndSendTxImpl(txn *Transaction) ErrCode {
	// if transaction is verified unsucessfully then will not put it into transaction pool
	if errCode := NodeForServers.AppendToTxnPool(txn); errCode != Success {
		log.Warn("Can NOT add the transaction to TxnPool")
		log.Info("[httpjsonrpc] VerifyTransaction failed when AppendToTxnPool.")
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
