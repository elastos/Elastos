package servers

import (
	"bytes"
	"encoding/binary"
	"encoding/json"
	"errors"
	"fmt"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/core"
	"github.com/elastos/Elastos.ELA.SideChain/logger"
	"github.com/elastos/Elastos.ELA.SideChain/mempool"
	"github.com/elastos/Elastos.ELA.SideChain/pow"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA.Utility/p2p/peer"
	"github.com/elastos/Elastos.ELA.Utility/p2p/server"
)

const (
	AuxBlockGenerateInterval = 5
	DestroyAddress           = "0000000000000000000000000000000000"
)

type Config struct {
	Logger     *logger.Logger
	Server     server.IServer
	Chain      *blockchain.BlockChain
	TxMemPool  *mempool.TxPool
	PowService *pow.Service
}

type Handler func(Params) map[string]interface{}

type HttpService struct {
	logger    *logger.Logger
	server    server.IServer
	chain     *blockchain.BlockChain
	txMemPool *mempool.TxPool
	pow       *pow.Service

	// This params are protected by prelock
	preLock        sync.Mutex
	preChainHeight uint32
	preTime        int64
	preTxCount     int
}

func NewHttpService(cfg *Config) *HttpService {
	s := &HttpService{
		logger:               cfg.Logger,
		server:               cfg.Server,
		chain:                cfg.Chain,
		txMemPool:            cfg.TxMemPool,
		pow:                  cfg.PowService,
	}
	return s
}

func ToReversedString(hash common.Uint256) string {
	return common.BytesToHexString(common.BytesReverse(hash[:]))
}

func FromReversedString(reversed string) ([]byte, error) {
	bytes, err := common.HexStringToBytes(reversed)
	return common.BytesReverse(bytes), err
}

func (s *HttpService) getTransactionInfo(header *core.Header, tx *core.Transaction) *TransactionInfo {
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
		destroyHash := common.Uint168{}
		if v.ProgramHash == destroyHash {
			address = DestroyAddress
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
		confirmations = s.chain.GetBestHeight() - header.Height + 1
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

func (s *HttpService) getTransaction(txInfo *TransactionInfo) (*core.Transaction, error) {
	txPaload, err := s.getPayload(txInfo.Payload)
	if err != nil {
		return nil, err
	}

	var txAttribute []*core.Attribute
	for _, att := range txInfo.Attributes {
		var attData []byte
		if att.Usage == core.Nonce {
			attData = []byte(att.Data)
		} else {
			attData, err = common.HexStringToBytes(att.Data)
			if err != nil {
				return nil, err
			}
		}
		txAttr := &core.Attribute{
			Usage: att.Usage,
			Data:  attData,
		}
		txAttribute = append(txAttribute, txAttr)
	}

	var txUTXOTxInput []*core.Input
	for _, input := range txInfo.Inputs {
		txID, err := FromReversedString(input.TxID)
		if err != nil {
			return nil, err
		}
		referID, err := common.Uint256FromBytes(txID)
		if err != nil {
			return nil, err
		}
		utxoInput := &core.Input{
			Previous: core.OutPoint{
				TxID:  *referID,
				Index: input.VOut,
			},
			Sequence: input.Sequence,
		}
		txUTXOTxInput = append(txUTXOTxInput, utxoInput)
	}

	var txOutputs []*core.Output
	for _, output := range txInfo.Outputs {
		assetIdBytes, err := FromReversedString(output.AssetID)
		if err != nil {
			return nil, err
		}
		assetId, err := common.Uint256FromBytes(assetIdBytes)
		if err != nil {
			return nil, err
		}
		value, err := common.StringToFixed64(output.Value)
		if err != nil {
			return nil, err
		}
		programHash, err := common.Uint168FromAddress(output.Address)
		if err != nil {
			return nil, err
		}
		output := &core.Output{
			AssetID:     *assetId,
			Value:       *value,
			OutputLock:  output.OutputLock,
			ProgramHash: *programHash,
		}
		txOutputs = append(txOutputs, output)
	}

	var txPrograms []*core.Program
	for _, pgrm := range txInfo.Programs {
		code, err := common.HexStringToBytes(pgrm.Code)
		if err != nil {
			return nil, err
		}
		parameter, err := common.HexStringToBytes(pgrm.Parameter)
		if err != nil {
			return nil, err
		}
		txProgram := &core.Program{
			Code:      code,
			Parameter: parameter,
		}
		txPrograms = append(txPrograms, txProgram)
	}

	txTransaction := &core.Transaction{
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
func (s *HttpService) GetRawTransaction(param Params) map[string]interface{} {
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
	tx, height, err := s.chain.GetTransaction(hash)
	if err != nil {
		return ResponsePack(UnknownTransaction, "")
	}
	bHash, err := s.chain.GetBlockHash(height)
	if err != nil {
		return ResponsePack(UnknownTransaction, "")
	}
	header, err := s.chain.GetHeader(bHash)
	if err != nil {
		return ResponsePack(UnknownTransaction, "")
	}

	verbose, ok := param.Bool("verbose")
	if verbose {
		return ResponsePack(Success, s.getTransactionInfo(header, tx))
	} else {
		buf := new(bytes.Buffer)
		tx.Serialize(buf)
		return ResponsePack(Success, common.BytesToHexString(buf.Bytes()))
	}
}

func (s *HttpService) GetNeighbors(param Params) map[string]interface{} {
	peers := s.server.ConnectedPeers()
	neighborAddrs := make([]string, 0, len(peers))
	for _, peer := range peers {
		neighborAddrs = append(neighborAddrs, peer.ToPeer().String())
	}
	return ResponsePack(Success, neighborAddrs)
}

func (s *HttpService) GetNodeState(param Params) map[string]interface{} {
	peers := s.server.ConnectedPeers()
	states := make([]*peer.StatsSnap, 0, len(peers))
	for _, peer := range peers {
		states = append(states, peer.ToPeer().StatsSnapshot())
	}
	return ResponsePack(Success, states)
}

func (s *HttpService) SetLogLevel(param Params) map[string]interface{} {
	level, ok := param["level"].(float64)
	if !ok || level < 0 {
		return ResponsePack(InvalidParams, "level must be an integer in 0-6")
	}

	if err := s.logger.SetPrintLevel(int(level)); err != nil {
		return ResponsePack(InvalidParams, err.Error())
	}
	return ResponsePack(Success, fmt.Sprint("log level has been set to ", level))
}

func (s *HttpService) SubmitSideAuxBlock(param Params) map[string]interface{} {
	blockHash, ok := param.String("blockhash")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}
	if _, ok := s.pow.MsgBlock.BlockData[blockHash]; !ok {
		log.Trace("[json-rpc:SubmitSideAuxBlock] receive invalid block hash value:", blockHash)
		return ResponsePack(InvalidParams, "")
	}

	sideAuxPow, ok := param.String("sideauxpow")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}

	buf, _ := common.HexStringToBytes(sideAuxPow)
	err := s.pow.MsgBlock.BlockData[blockHash].Header.SideAuxPow.Deserialize(bytes.NewReader(buf))
	if err != nil {
		log.Trace(err)
		return ResponsePack(InternalError, "[json-rpc:SubmitSideAuxBlock] deserialize side aux pow failed")
	}

	inMainChain, isOrphan, err := s.chain.AddBlock(s.pow.MsgBlock.BlockData[blockHash])
	if err != nil {
		log.Trace(err)
		return ResponsePack(InternalError, "")
	}

	if isOrphan || !inMainChain {
		return ResponsePack(InternalError, "")
	}
	s.pow.BroadcastBlock(s.pow.MsgBlock.BlockData[blockHash])

	s.pow.MsgBlock.Mutex.Lock()
	for key := range s.pow.MsgBlock.BlockData {
		delete(s.pow.MsgBlock.BlockData, key)
	}
	s.pow.MsgBlock.Mutex.Unlock()
	log.Trace("AddBlock called finished and s.pow.MsgBlock.BlockData has been deleted completely")

	log.Info(sideAuxPow, blockHash)
	return ResponsePack(Success, blockHash)
}

func (s *HttpService) generateAuxBlock(addr string) (*core.Block, string, bool) {
	msgBlock := &core.Block{}
	if s.chain.GetBestHeight() == 0 || s.preChainHeight != s.chain.GetBestHeight() ||
		time.Now().Unix()-s.preTime > AuxBlockGenerateInterval {
		if s.preChainHeight != s.chain.GetBestHeight() {
			s.preChainHeight = s.chain.GetBestHeight()
			s.preTime = time.Now().Unix()
			s.preTxCount = s.pow.GetTransactionCount()
		}

		currentTxsCount := s.pow.CollectTransactions(msgBlock)
		if 0 == currentTxsCount {
			// return nil, "currentTxs is nil", false
		}

		msgBlock, err := s.pow.GenerateBlock(addr)
		if nil != err {
			return nil, "msgBlock generate err", false
		}

		curHash := msgBlock.Hash()
		curHashStr := common.BytesToHexString(curHash.Bytes())

		s.pow.MsgBlock.Mutex.Lock()
		s.pow.MsgBlock.BlockData[curHashStr] = msgBlock
		s.pow.MsgBlock.Mutex.Unlock()

		s.preChainHeight = s.chain.GetBestHeight()
		s.preTime = time.Now().Unix()
		s.preTxCount = currentTxsCount // Don't Call GetTransactionCount()

		return msgBlock, curHashStr, true
	}
	return nil, "", false
}

func (s *HttpService) CreateAuxBlock(param Params) map[string]interface{} {
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

	s.pow.SetMinerAddr(addr)

	genesisHash, err := s.chain.GetBlockHash(uint32(0))
	if err != nil {
		return ResponsePack(Error, "Get genesis hash failed")
	}
	genesisHashStr := common.BytesToHexString(genesisHash.Bytes())

	preHash := s.chain.CurrentBlockHash()
	preHashStr := common.BytesToHexString(preHash.Bytes())

	SendToAux := SideAuxBlock{
		GenesisHash:       genesisHashStr,
		Height:            s.chain.GetBestHeight(),
		Bits:              fmt.Sprintf("%x", msgBlock.Bits), //difficulty
		Hash:              curHashStr,
		PreviousBlockHash: preHashStr,
	}
	return ResponsePack(Success, &SendToAux)
}

func (s *HttpService) GetInfo(param Params) map[string]interface{} {
	RetVal := struct {
		Version        int    `json:"version"`
		Balance        int    `json:"balance"`
		Blocks         uint32 `json:"blocks"`
		Timeoffset     int    `json:"timeoffset"`
		Connections    int32  `json:"connections"`
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
		Blocks:         s.chain.GetBestHeight(),
		Timeoffset:     0,
		Connections:    s.server.ConnectedCount(),
		Testnet:        config.Parameters.PowConfiguration.TestNet,
		Keypoololdest:  0,
		Keypoolsize:    0,
		Unlocked_until: 0,
		Paytxfee:       0,
		Relayfee:       0,
		Errors:         "Tobe written"}
	return ResponsePack(Success, &RetVal)
}

func (s *HttpService) AuxHelp(param Params) map[string]interface{} {

	//TODO  and description for this rpc-interface
	return ResponsePack(Success, "createauxblock==submitsideauxblock")
}

func (s *HttpService) ToggleMining(param Params) map[string]interface{} {
	mining, ok := param.Bool("mining")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}

	var message string
	if mining {
		go s.pow.Start()
		message = "mining started"
	} else {
		go s.pow.Halt()
		message = "mining stopped"
	}

	return ResponsePack(Success, message)
}

func (s *HttpService) DiscreteMining(param Params) map[string]interface{} {
	count, ok := param.Uint("count")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}

	ret := make([]string, count)

	blockHashes, err := s.pow.DiscreteMining(uint32(count))
	if err != nil {
		return ResponsePack(Error, err)
	}

	for i, hash := range blockHashes {
		ret[i] = ToReversedString(*hash)
	}

	return ResponsePack(Success, ret)
}

func (s *HttpService) GetConnectionCount(param Params) map[string]interface{} {
	return ResponsePack(Success, s.server.ConnectedCount())
}

func (s *HttpService) GetTransactionPool(param Params) map[string]interface{} {
	txs := make([]*TransactionInfo, 0)
	for _, t := range s.txMemPool.GetTxsInPool() {
		txs = append(txs, s.getTransactionInfo(nil, t))
	}
	return ResponsePack(Success, txs)
}

func (s *HttpService) getBlockInfo(block *core.Block, verbose bool) BlockInfo {
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
	binary.BigEndian.PutUint32(chainWork[:], s.chain.GetBestHeight()-block.Header.Height)

	nextBlockHash, _ := s.chain.GetBlockHash(block.Header.Height + 1)

	auxPow := new(bytes.Buffer)
	block.Header.SideAuxPow.Serialize(auxPow)

	return BlockInfo{
		Hash:              ToReversedString(block.Hash()),
		Confirmations:     s.chain.GetBestHeight() - block.Header.Height + 1,
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
		Difficulty:        blockchain.CalcCurrentDifficulty(block.Header.Bits),
		ChainWork:         common.BytesToHexString(chainWork[:]),
		PreviousBlockHash: ToReversedString(block.Header.Previous),
		NextBlockHash:     ToReversedString(nextBlockHash),
		AuxPow:            common.BytesToHexString(auxPow.Bytes()),
	}
}

func (s *HttpService) getBlock(hash common.Uint256, format uint32) (interface{}, ErrorCode) {
	block, err := s.chain.GetBlockByHash(hash)
	if err != nil {
		return "", UnknownBlock
	}
	switch format {
	case 0:
		w := new(bytes.Buffer)
		block.Serialize(w)
		return common.BytesToHexString(w.Bytes()), Success
	case 2:
		return s.getBlockInfo(block, true), Success
	}
	return s.getBlockInfo(block, false), Success
}

func (s *HttpService) GetBlockByHash(param Params) map[string]interface{} {
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

	result, error := s.getBlock(hash, verbosity)

	return ResponsePack(error, result)
}

func (s *HttpService) SendTransactionInfo(param Params) map[string]interface{} {

	infoStr, ok := param.String("Info")
	if !ok {
		return ResponsePack(InvalidParams, "Info not found")
	}

	infoBytes, err := common.HexStringToBytes(infoStr)
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
	var hash common.Uint256
	hash = txn.Hash()
	if err := s.verifyAndSendTx(txn); err != nil {
		return ResponsePack(InvalidTransaction, err.Error())
	}
	return ResponsePack(Success, hash.String())
}

func (s *HttpService) SendRawTransaction(param Params) map[string]interface{} {
	str, ok := param.String("data")
	if !ok {
		return ResponsePack(InvalidParams, "need a string parameter named data")
	}

	bys, err := common.HexStringToBytes(str)
	if err != nil {
		return ResponsePack(InvalidParams, "hex string to bytes error")
	}
	var txn core.Transaction
	if err := txn.Deserialize(bytes.NewReader(bys)); err != nil {
		return ResponsePack(InvalidTransaction, "transaction deserialize error")
	}

	if err := s.verifyAndSendTx(&txn); err != nil {
		return ResponsePack(InvalidTransaction, err.Error())
	}

	return ResponsePack(Success, ToReversedString(txn.Hash()))
}

func (s *HttpService) GetBlockHeight(param Params) map[string]interface{} {
	return ResponsePack(Success, s.chain.GetBestHeight())
}

func (s *HttpService) GetBestBlockHash(param Params) map[string]interface{} {
	height, ok := param.Uint("height")
	if !ok {
		return ResponsePack(InvalidParams, "height parameter should be a positive integer")
	}

	hash, err := s.chain.GetBlockHash(height)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}
	return ResponsePack(Success, ToReversedString(hash))
}

func (s *HttpService) GetBlockCount(param Params) map[string]interface{} {
	return ResponsePack(Success, s.chain.GetBestHeight()+1)
}

func (s *HttpService) GetBlockHash(param Params) map[string]interface{} {
	height, ok := param.Uint("height")
	if !ok {
		return ResponsePack(InvalidParams, "height parameter should be a positive integer")
	}

	hash, err := s.chain.GetBlockHash(height)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}
	return ResponsePack(Success, ToReversedString(hash))
}

func (s *HttpService) getBlockTransactions(block *core.Block) interface{} {
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

func (s *HttpService) GetTransactionsByHeight(param Params) map[string]interface{} {
	height, ok := param.Uint("index")
	if !ok {
		return ResponsePack(InvalidParams, "index parameter should be a positive integer")
	}

	hash, err := s.chain.GetBlockHash(uint32(height))
	if err != nil {
		return ResponsePack(UnknownBlock, "")

	}
	block, err := s.chain.GetBlockByHash(hash)
	if err != nil {
		return ResponsePack(UnknownBlock, "")
	}
	return ResponsePack(Success, s.getBlockTransactions(block))
}

func (s *HttpService) GetBlockByHeight(param Params) map[string]interface{} {
	height, ok := param.Uint("height")
	if !ok {
		return ResponsePack(InvalidParams, "height parameter should be a positive integer")
	}

	hash, err := s.chain.GetBlockHash(uint32(height))
	if err != nil {
		return ResponsePack(UnknownBlock, "")
	}

	result, errCode := s.getBlock(hash, 2)

	return ResponsePack(errCode, result)
}

//Asset
func (s *HttpService) GetAssetByHash(param Params) map[string]interface{} {
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
	asset, err := s.chain.GetAsset(hash)
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

func (s *HttpService) GetBalanceByAddr(param Params) map[string]interface{} {
	str, ok := param.String("addr")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}

	programHash, err := common.Uint168FromAddress(str)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}
	unspends, err := s.chain.GetUnspents(*programHash)
	var balance common.Fixed64 = 0
	for _, u := range unspends {
		for _, v := range u {
			balance = balance + v.Value
		}
	}
	return ResponsePack(Success, balance.String())
}

func (s *HttpService) GetBalanceByAsset(param Params) map[string]interface{} {
	addr, ok := param.String("addr")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}

	programHash, err := common.Uint168FromAddress(addr)
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
	assetId, err := common.Uint256FromBytes(assetIdBytes)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}

	unspents, err := s.chain.GetUnspents(*programHash)
	var balance common.Fixed64 = 0
	for k, u := range unspents {
		for _, v := range u {
			if assetId.IsEqual(k) {
				balance = balance + v.Value
			}
		}
	}
	return ResponsePack(Success, balance.String())
}

func (s *HttpService) GetUnspendsByAddr(param Params) map[string]interface{} {
	addr, ok := param.String("addr")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}

	programHash, err := common.Uint168FromAddress(addr)
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
	unspends, err := s.chain.GetUnspents(*programHash)

	for k, u := range unspends {
		asset, err := s.chain.GetAsset(k)
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

func (s *HttpService) GetUnspendsByAsset(param Params) map[string]interface{} {
	addr, ok := param.String("addr")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}
	programHash, err := common.Uint168FromAddress(addr)
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

	var assetHash common.Uint256
	if err := assetHash.Deserialize(bytes.NewReader(bys)); err != nil {
		return ResponsePack(InvalidParams, "")
	}
	type UTXOUnspentInfo struct {
		Txid  string
		Index uint32
		Value string
	}
	infos, err := s.chain.GetAssetUnspents(*programHash, assetHash)
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
func (s *HttpService) GetTransactionByHash(param Params) map[string]interface{} {
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
	txn, height, err := s.chain.GetTransaction(hash)
	if err != nil {
		return ResponsePack(UnknownTransaction, "")
	}
	bHash, err := s.chain.GetBlockHash(height)
	if err != nil {
		return ResponsePack(UnknownBlock, "")
	}
	header, err := s.chain.GetHeader(bHash)
	if err != nil {
		return ResponsePack(UnknownBlock, "")
	}

	return ResponsePack(Success, s.getTransactionInfo(header, txn))
}

func (s *HttpService) GetExistDepositTransactions(param Params) map[string]interface{} {
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
		inStore := s.chain.IsDuplicateMainchainTx(*hash)
		inTxPool := s.txMemPool.IsDuplicateMainchainTx(*hash)
		if inTxPool || inStore {
			resultTxHashes = append(resultTxHashes, txHash)
		}
	}

	return ResponsePack(Success, resultTxHashes)
}

func (s *HttpService) getBlockTransactionsDetail(block *core.Block, filter func(*core.Transaction) bool) interface{} {
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

func (s *HttpService) GetDestroyedTransactionsByHeight(param Params) map[string]interface{} {
	height, ok := param.Uint("height")
	if !ok {
		return ResponsePack(InvalidParams, "height parameter should be a positive integer")
	}

	hash, err := s.chain.GetBlockHash(uint32(height))
	if err != nil {
		return ResponsePack(UnknownBlock, "")

	}
	block, err := s.chain.GetBlockByHash(hash)
	if err != nil {
		return ResponsePack(UnknownBlock, "")
	}

	destroyHash := common.Uint168{}
	return ResponsePack(Success, s.getBlockTransactionsDetail(block, func(tran *core.Transaction) bool {
		_, ok := tran.Payload.(*core.PayloadTransferCrossChainAsset)
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

func (s *HttpService) getPayload(pInfo PayloadInfo) (core.Payload, error) {

	switch object := pInfo.(type) {
	case *RegisterAssetInfo:
		obj := new(core.PayloadRegisterAsset)
		obj.Asset = object.Asset
		amount, err := common.StringToFixed64(object.Amount)
		if err != nil {
			return nil, err
		}
		obj.Amount = *amount
		bytes, err := common.HexStringToBytes(object.Controller)
		if err != nil {
			return nil, err
		}
		controller, err := common.Uint168FromBytes(common.BytesReverse(bytes))
		obj.Controller = *controller
		return obj, nil
	case *RechargeToSideChainInfo:
		obj := new(core.PayloadRechargeToSideChain)
		proofBytes, err := common.HexStringToBytes(object.Proof)
		if err != nil {
			return nil, err
		}
		obj.MerkleProof = proofBytes
		transactionBytes, err := common.HexStringToBytes(object.MainChainTransaction)
		if err != nil {
			return nil, err
		}
		obj.MainChainTransaction = transactionBytes
		return obj, nil
	case *TransferCrossChainAssetInfo:
		obj := new(core.PayloadTransferCrossChainAsset)
		obj.CrossChainAddresses = object.CrossChainAddresses
		obj.OutputIndexes = object.OutputIndexes
		obj.CrossChainAmounts = object.CrossChainAmounts
		return obj, nil
	}

	return nil, errors.New("Invalid payload type.")
}

func (s *HttpService) getPayloadInfo(p core.Payload) PayloadInfo {
	switch object := p.(type) {
	case *core.PayloadCoinBase:
		obj := new(CoinbaseInfo)
		obj.CoinbaseData = string(object.CoinbaseData)
		return obj
	case *core.PayloadRegisterAsset:
		obj := new(RegisterAssetInfo)
		obj.Asset = object.Asset
		obj.Amount = object.Amount.String()
		obj.Controller = common.BytesToHexString(common.BytesReverse(object.Controller.Bytes()))
		return obj
	case *core.PayloadTransferCrossChainAsset:
		obj := new(TransferCrossChainAssetInfo)
		obj.CrossChainAddresses = object.CrossChainAddresses
		obj.OutputIndexes = object.OutputIndexes
		obj.CrossChainAmounts = object.CrossChainAmounts
		return obj
	case *core.PayloadTransferAsset:
	case *core.PayloadRecord:
	case *core.PayloadRechargeToSideChain:
		obj := new(RechargeToSideChainInfo)
		obj.MainChainTransaction = common.BytesToHexString(object.MainChainTransaction)
		obj.Proof = common.BytesToHexString(object.MerkleProof)
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

func (s *HttpService) getTransactionInfoFromBytes(txInfoBytes []byte) (*TransactionInfo, error) {
	var txInfo TransactionInfo
	err := json.Unmarshal(txInfoBytes, &txInfo)
	if err != nil {
		return nil, errors.New("InvalidParameter")
	}

	var assetInfo PayloadInfo
	switch txInfo.TxType {
	case core.CoinBase:
		assetInfo = &CoinbaseInfo{}
	case core.RegisterAsset:
		assetInfo = &RegisterAssetInfo{}
	case core.SideChainPow:
		assetInfo = &SideChainPowInfo{}
	case core.RechargeToSideChain:
		assetInfo = &RechargeToSideChainInfo{}
	case core.TransferCrossChainAsset:
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

func (s *HttpService) verifyAndSendTx(tx *core.Transaction) error {
	// if transaction is verified unsucessfully then will not put it into transaction pool
	if err := s.txMemPool.AppendToTxPool(tx); err != nil {
		if e, ok := err.(*RuleError); ok {
			log.Infof("rule error when adding transaction pool, "+
				"error %s, desc %s", e.ErrorCode, e.Description)
		}
		return err
	}
	s.server.BroadcastMessage(msg.NewTx(tx))
	return nil
}

func ResponsePack(errCode ErrorCode, result interface{}) map[string]interface{} {
	if errCode != 0 && (result == "" || result == nil) {
		result = errCode.String()
	}
	return map[string]interface{}{"Result": result, "Error": errCode}
}
