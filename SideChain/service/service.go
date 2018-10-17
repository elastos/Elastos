package service

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
	"github.com/elastos/Elastos.ELA.SideChain/mempool"
	"github.com/elastos/Elastos.ELA.SideChain/pow"
	"github.com/elastos/Elastos.ELA.SideChain/types"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/elalog"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA.Utility/p2p/peer"
	"github.com/elastos/Elastos.ELA.Utility/p2p/server"
)

const (
	AuxBlockGenerateInterval = 5
	DestroyAddress           = "0000000000000000000000000000000000"
)

type Config struct {
	Server     server.IServer
	Chain      *blockchain.BlockChain
	TxMemPool  *mempool.TxPool
	PowService *pow.Service

	GetBlockInfo                func(cfg *Config, block *types.Block, verbose bool) BlockInfo
	GetTransactionInfo          func(cfg *Config, header *types.Header, tx *types.Transaction) *TransactionInfo
	GetTransactionInfoFromBytes func(txInfoBytes []byte) (*TransactionInfo, error)
	GetTransaction              func(cfg *Config, txInfo *TransactionInfo) (*types.Transaction, error)
	GetPayloadInfo              func(p types.Payload, pVersion byte) PayloadInfo
	GetPayload                  func(pInfo PayloadInfo) (types.Payload, error)
}

type Handler func(Params) map[string]interface{}

type HttpService struct {
	cfg *Config

	// This params are protected by prelock
	preLock        sync.Mutex
	preChainHeight uint32
	preTime        int64
	preTxCount     int
}

func NewHttpService(cfg *Config) *HttpService {
	s := &HttpService{
		cfg: cfg,
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
	tx, height, err := s.cfg.Chain.GetTransaction(hash)
	if err != nil {
		return ResponsePack(UnknownTransaction, "")
	}
	bHash, err := s.cfg.Chain.GetBlockHash(height)
	if err != nil {
		return ResponsePack(UnknownTransaction, "")
	}
	header, err := s.cfg.Chain.GetHeader(bHash)
	if err != nil {
		return ResponsePack(UnknownTransaction, "")
	}

	verbose, ok := param.Bool("verbose")
	if verbose {
		return ResponsePack(Success, s.cfg.GetTransactionInfo(s.cfg, header, tx))
	} else {
		buf := new(bytes.Buffer)
		tx.Serialize(buf)
		return ResponsePack(Success, common.BytesToHexString(buf.Bytes()))
	}
}

func (s *HttpService) GetNeighbors(param Params) map[string]interface{} {
	peers := s.cfg.Server.ConnectedPeers()
	neighborAddrs := make([]string, 0, len(peers))
	for _, peer := range peers {
		neighborAddrs = append(neighborAddrs, peer.ToPeer().String())
	}
	return ResponsePack(Success, neighborAddrs)
}

func (s *HttpService) GetNodeState(param Params) map[string]interface{} {
	peers := s.cfg.Server.ConnectedPeers()
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

	log.SetLevel(elalog.Level(level))
	return ResponsePack(Success, fmt.Sprint("log level has been set to ", level))
}

func (s *HttpService) SubmitSideAuxBlock(param Params) map[string]interface{} {
	blockHash, ok := param.String("blockhash")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}
	if _, ok := s.cfg.PowService.MsgBlock.BlockData[blockHash]; !ok {
		log.Trace("[json-rpc:SubmitSideAuxBlock] receive invalid block hash value:", blockHash)
		return ResponsePack(InvalidParams, "")
	}

	sideAuxPow, ok := param.String("sideauxpow")
	if !ok {
		return ResponsePack(InvalidParams, "")
	}

	buf, _ := common.HexStringToBytes(sideAuxPow)
	err := s.cfg.PowService.MsgBlock.BlockData[blockHash].Header.SideAuxPow.Deserialize(bytes.NewReader(buf))
	if err != nil {
		log.Trace(err)
		return ResponsePack(InternalError, "[json-rpc:SubmitSideAuxBlock] deserialize side aux pow failed")
	}

	inMainChain, isOrphan, err := s.cfg.Chain.AddBlock(s.cfg.PowService.MsgBlock.BlockData[blockHash])
	if err != nil {
		log.Trace(err)
		return ResponsePack(InternalError, "")
	}

	if isOrphan || !inMainChain {
		return ResponsePack(InternalError, "")
	}
	s.cfg.PowService.BroadcastBlock(s.cfg.PowService.MsgBlock.BlockData[blockHash])

	s.cfg.PowService.MsgBlock.Mutex.Lock()
	for key := range s.cfg.PowService.MsgBlock.BlockData {
		delete(s.cfg.PowService.MsgBlock.BlockData, key)
	}
	s.cfg.PowService.MsgBlock.Mutex.Unlock()
	log.Trace("AddBlock called finished and s.pow.MsgBlock.BlockData has been deleted completely")

	log.Info(sideAuxPow, blockHash)
	return ResponsePack(Success, blockHash)
}

func (s *HttpService) generateAuxBlock(addr string) (*types.Block, string, bool) {
	msgBlock := &types.Block{}
	if s.cfg.Chain.GetBestHeight() == 0 || s.preChainHeight != s.cfg.Chain.GetBestHeight() ||
		time.Now().Unix()-s.preTime > AuxBlockGenerateInterval {
		if s.preChainHeight != s.cfg.Chain.GetBestHeight() {
			s.preChainHeight = s.cfg.Chain.GetBestHeight()
			s.preTime = time.Now().Unix()
			s.preTxCount = s.cfg.PowService.GetTransactionCount()
		}

		currentTxsCount := s.cfg.PowService.CollectTransactions(msgBlock)
		if 0 == currentTxsCount {
			// return nil, "currentTxs is nil", false
		}

		msgBlock, err := s.cfg.PowService.Cfg.GenerateBlock(s.cfg.PowService.Cfg)
		if nil != err {
			return nil, "msgBlock generate err", false
		}

		curHash := msgBlock.Hash()
		curHashStr := common.BytesToHexString(curHash.Bytes())

		s.cfg.PowService.MsgBlock.Mutex.Lock()
		s.cfg.PowService.MsgBlock.BlockData[curHashStr] = msgBlock
		s.cfg.PowService.MsgBlock.Mutex.Unlock()

		s.preChainHeight = s.cfg.Chain.GetBestHeight()
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

	s.cfg.PowService.SetMinerAddr(addr)

	genesisHash, err := s.cfg.Chain.GetBlockHash(uint32(0))
	if err != nil {
		return ResponsePack(Error, "Get genesis hash failed")
	}
	genesisHashStr := common.BytesToHexString(genesisHash.Bytes())

	preHash := s.cfg.Chain.CurrentBlockHash()
	preHashStr := common.BytesToHexString(preHash.Bytes())

	SendToAux := SideAuxBlock{
		GenesisHash:       genesisHashStr,
		Height:            s.cfg.Chain.GetBestHeight(),
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
		Blocks:         s.cfg.Chain.GetBestHeight(),
		Timeoffset:     0,
		Connections:    s.cfg.Server.ConnectedCount(),
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
		go s.cfg.PowService.Start()
		message = "mining started"
	} else {
		go s.cfg.PowService.Halt()
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

	blockHashes, err := s.cfg.PowService.DiscreteMining(uint32(count))
	if err != nil {
		return ResponsePack(Error, err)
	}

	for i, hash := range blockHashes {
		ret[i] = ToReversedString(*hash)
	}

	return ResponsePack(Success, ret)
}

func (s *HttpService) GetConnectionCount(param Params) map[string]interface{} {
	return ResponsePack(Success, s.cfg.Server.ConnectedCount())
}

func (s *HttpService) GetTransactionPool(param Params) map[string]interface{} {
	txs := make([]*TransactionInfo, 0)
	for _, t := range s.cfg.TxMemPool.GetTxsInPool() {
		txs = append(txs, s.cfg.GetTransactionInfo(s.cfg, nil, t))
	}
	return ResponsePack(Success, txs)
}

func (s *HttpService) getBlock(hash common.Uint256, format uint32) (interface{}, ErrorCode) {
	block, err := s.cfg.Chain.GetBlockByHash(hash)
	if err != nil {
		return "", UnknownBlock
	}
	switch format {
	case 0:
		w := new(bytes.Buffer)
		block.Serialize(w)
		return common.BytesToHexString(w.Bytes()), Success
	case 2:
		return s.cfg.GetBlockInfo(s.cfg, block, true), Success
	}
	return s.cfg.GetBlockInfo(s.cfg, block, false), Success
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
	infoStr, ok := param["info"]
	if !ok {
		return ResponsePack(InvalidParams, "info not found")
	}

	txInfo := new(TransactionInfo)
	if txInfo.PayloadVersion == types.RechargeToSideChainPayloadVersion0 {
		txInfo.Payload = new(RechargeToSideChainInfoV0)
	} else if txInfo.PayloadVersion == types.RechargeToSideChainPayloadVersion1 {
		txInfo.Payload = new(RechargeToSideChainInfoV1)
	}
	err := Unmarshal(&infoStr, txInfo)
	if err != nil {
		return ResponsePack(InvalidParams, "info type error")
	}

	txn, err := s.cfg.GetTransaction(s.cfg, txInfo)
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
	var txn types.Transaction
	if err := txn.Deserialize(bytes.NewReader(bys)); err != nil {
		return ResponsePack(InvalidTransaction, "transaction deserialize error")
	}

	if err := s.verifyAndSendTx(&txn); err != nil {
		return ResponsePack(InvalidTransaction, err.Error())
	}

	return ResponsePack(Success, ToReversedString(txn.Hash()))
}

func (s *HttpService) GetBlockHeight(param Params) map[string]interface{} {
	return ResponsePack(Success, s.cfg.Chain.GetBestHeight())
}

func (s *HttpService) GetBlockCount(param Params) map[string]interface{} {
	return ResponsePack(Success, s.cfg.Chain.GetBestHeight()+1)
}

func (s *HttpService) GetBlockHash(param Params) map[string]interface{} {
	height, ok := param.Uint("height")
	if !ok {
		return ResponsePack(InvalidParams, "height parameter should be a positive integer")
	}

	hash, err := s.cfg.Chain.GetBlockHash(height)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}
	return ResponsePack(Success, ToReversedString(hash))
}

func (s *HttpService) GetBestBlockHash(param Params) map[string]interface{} {
	height := s.cfg.Chain.GetBestHeight()
	hash, err := s.cfg.Chain.GetBlockHash(height)
	if err != nil {
		return ResponsePack(InvalidParams, "")
	}
	return ResponsePack(Success, ToReversedString(hash))
}

func (s *HttpService) getBlockTransactions(block *types.Block) interface{} {
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
	height, ok := param.Uint("height")
	if !ok {
		return ResponsePack(InvalidParams, "height parameter should be a positive integer")
	}

	hash, err := s.cfg.Chain.GetBlockHash(uint32(height))
	if err != nil {
		return ResponsePack(UnknownBlock, "")

	}
	block, err := s.cfg.Chain.GetBlockByHash(hash)
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

	hash, err := s.cfg.Chain.GetBlockHash(uint32(height))
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
	asset, err := s.cfg.Chain.GetAsset(hash)
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
	unspends, err := s.cfg.Chain.GetUnspents(*programHash)
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

	unspents, err := s.cfg.Chain.GetUnspents(*programHash)
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
	unspends, err := s.cfg.Chain.GetUnspents(*programHash)

	for k, u := range unspends {
		asset, err := s.cfg.Chain.GetAsset(k)
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
	infos, err := s.cfg.Chain.GetAssetUnspents(*programHash, assetHash)
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
	txn, height, err := s.cfg.Chain.GetTransaction(hash)
	if err != nil {
		return ResponsePack(UnknownTransaction, "")
	}
	bHash, err := s.cfg.Chain.GetBlockHash(height)
	if err != nil {
		return ResponsePack(UnknownBlock, "")
	}
	header, err := s.cfg.Chain.GetHeader(bHash)
	if err != nil {
		return ResponsePack(UnknownBlock, "")
	}

	return ResponsePack(Success, s.cfg.GetTransactionInfo(s.cfg, header, txn))
}

func (s *HttpService) GetExistDepositTransactions(param Params) map[string]interface{} {
	txs, ok := param.ArrayString("txs")
	if !ok {
		return ResponsePack(InvalidParams, "txs not found")
	}

	var resultTxHashes []string
	for _, txHash := range txs {
		txHashBytes, err := common.HexStringToBytes(txHash)
		if err != nil {
			return ResponsePack(InvalidParams, "")
		}
		hash, err := common.Uint256FromBytes(txHashBytes)
		if err != nil {
			return ResponsePack(InvalidParams, "")
		}
		inStore := s.cfg.Chain.IsDuplicateMainchainTx(*hash)
		inTxPool := s.cfg.TxMemPool.IsDuplicateMainchainTx(*hash)
		if inTxPool || inStore {
			resultTxHashes = append(resultTxHashes, txHash)
		}
	}

	return ResponsePack(Success, resultTxHashes)
}

func (s *HttpService) getBlockTransactionsDetail(block *types.Block, filter func(*types.Transaction) bool) interface{} {
	var trans []*TransactionInfo
	for _, tx := range block.Transactions {
		if !filter(tx) {
			continue
		}

		trans = append(trans, s.cfg.GetTransactionInfo(s.cfg, &block.Header, tx))
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

	hash, err := s.cfg.Chain.GetBlockHash(uint32(height))
	if err != nil {
		return ResponsePack(UnknownBlock, "")

	}
	block, err := s.cfg.Chain.GetBlockByHash(hash)
	if err != nil {
		return ResponsePack(UnknownBlock, "")
	}

	destroyHash := common.Uint168{}
	return ResponsePack(Success, s.getBlockTransactionsDetail(block, func(tran *types.Transaction) bool {
		_, ok := tran.Payload.(*types.PayloadTransferCrossChainAsset)
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

func (s *HttpService) GetTransactionInfoByHash(param Params) map[string]interface{} {
	str, ok := param.String("txid")
	if !ok {
		return ResponsePack(InvalidParams, "txid not found")
	}
	hex, err := FromReversedString(str)
	if err != nil {
		return ResponsePack(InvalidParams, "txid reverse failed")
	}
	var hash common.Uint256
	err = hash.Deserialize(bytes.NewReader(hex))
	if err != nil {
		return ResponsePack(InvalidTransaction, "txid deserialize failed")
	}
	tx, height, err := s.cfg.Chain.GetTransaction(hash)
	if err != nil {
		return ResponsePack(UnknownTransaction, "get tx by txid failed")
	}
	bHash, err := s.cfg.Chain.GetBlockHash(height)
	if err != nil {
		return ResponsePack(UnknownTransaction, "get block by height failed")
	}
	header, err := s.cfg.Chain.GetHeader(bHash)
	if err != nil {
		return ResponsePack(UnknownTransaction, "get header by block hash failed")
	}
	return ResponsePack(Success, s.cfg.GetTransactionInfo(s.cfg, header, tx))
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

func (s *HttpService) verifyAndSendTx(tx *types.Transaction) error {
	// if transaction is verified unsucessfully then will not put it into transaction pool
	if err := s.cfg.TxMemPool.AppendToTxPool(tx); err != nil {
		if e, ok := err.(*RuleError); ok {
			log.Infof("rule error when adding transaction pool, "+
				"error %s, desc %s", e.ErrorCode, e.Description)
		}
		return err
	}
	s.cfg.Server.BroadcastMessage(msg.NewTx(tx))
	return nil
}

func ResponsePack(errCode ErrorCode, result interface{}) map[string]interface{} {
	if errCode != 0 && (result == "" || result == nil) {
		result = errCode.String()
	}
	return map[string]interface{}{"Result": result, "Error": errCode}
}

func GetBlockInfo(cfg *Config, block *types.Block, verbose bool) BlockInfo {
	var txs []interface{}
	if verbose {
		for _, tx := range block.Transactions {
			txs = append(txs, cfg.GetTransactionInfo(cfg, &block.Header, tx))
		}
	} else {
		for _, tx := range block.Transactions {
			txs = append(txs, ToReversedString(tx.Hash()))
		}
	}
	var versionBytes [4]byte
	binary.BigEndian.PutUint32(versionBytes[:], block.Header.Version)

	var chainWork [4]byte
	binary.BigEndian.PutUint32(chainWork[:], cfg.Chain.GetBestHeight()-block.Header.Height)

	nextBlockHash, _ := cfg.Chain.GetBlockHash(block.Header.Height + 1)

	auxPow := new(bytes.Buffer)
	block.Header.SideAuxPow.Serialize(auxPow)

	return BlockInfo{
		Hash:              ToReversedString(block.Hash()),
		Confirmations:     cfg.Chain.GetBestHeight() - block.Header.Height + 1,
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

func GetTransactionInfo(cfg *Config, header *types.Header, tx *types.Transaction) *TransactionInfo {
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
		confirmations = cfg.Chain.GetBestHeight() - header.Height + 1
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
		Payload:        cfg.GetPayloadInfo(tx.Payload, tx.PayloadVersion),
		Attributes:     attributes,
		Programs:       programs,
	}
}

func GetTransactionInfoFromBytes(txInfoBytes []byte) (*TransactionInfo, error) {
	var txInfo TransactionInfo
	err := json.Unmarshal(txInfoBytes, &txInfo)
	if err != nil {
		return nil, errors.New("InvalidParameter")
	}

	var assetInfo PayloadInfo
	switch txInfo.TxType {
	case types.CoinBase:
		assetInfo = &CoinbaseInfo{}
	case types.RegisterAsset:
		assetInfo = &RegisterAssetInfo{}
	case types.SideChainPow:
		assetInfo = &SideChainPowInfo{}
	case types.RechargeToSideChain:
		if txInfo.PayloadVersion == types.RechargeToSideChainPayloadVersion0 {
			assetInfo = &RechargeToSideChainInfoV0{}
		} else if txInfo.PayloadVersion == types.RechargeToSideChainPayloadVersion1 {
			assetInfo = &RechargeToSideChainInfoV1{}
		}
	case types.TransferCrossChainAsset:
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

func GetTransaction(cfg *Config, txInfo *TransactionInfo) (*types.Transaction, error) {
	txPaload, err := cfg.GetPayload(txInfo.Payload)
	if err != nil {
		return nil, err
	}

	var txAttribute []*types.Attribute
	for _, att := range txInfo.Attributes {
		var attData []byte
		if att.Usage == types.Nonce {
			attData = []byte(att.Data)
		} else {
			attData, err = common.HexStringToBytes(att.Data)
			if err != nil {
				return nil, err
			}
		}
		txAttr := &types.Attribute{
			Usage: att.Usage,
			Data:  attData,
		}
		txAttribute = append(txAttribute, txAttr)
	}

	var txUTXOTxInput []*types.Input
	for _, input := range txInfo.Inputs {
		txID, err := FromReversedString(input.TxID)
		if err != nil {
			return nil, err
		}
		referID, err := common.Uint256FromBytes(txID)
		if err != nil {
			return nil, err
		}
		utxoInput := &types.Input{
			Previous: types.OutPoint{
				TxID:  *referID,
				Index: input.VOut,
			},
			Sequence: input.Sequence,
		}
		txUTXOTxInput = append(txUTXOTxInput, utxoInput)
	}

	var txOutputs []*types.Output
	for _, output := range txInfo.Outputs {
		assetIdBytes, err := FromReversedString(output.AssetID)
		if err != nil {
			return nil, err
		}
		assetId, err := common.Uint256FromBytes(assetIdBytes)
		if err != nil {
			asset := types.GetSystemAssetId()
			assetId = &asset
		}
		value, err := common.StringToFixed64(output.Value)
		if err != nil {
			return nil, err
		}
		programHash, err := common.Uint168FromAddress(output.Address)
		if err != nil {
			return nil, err
		}
		output := &types.Output{
			AssetID:     *assetId,
			Value:       *value,
			OutputLock:  output.OutputLock,
			ProgramHash: *programHash,
		}
		txOutputs = append(txOutputs, output)
	}

	var txPrograms []*types.Program
	for _, pgrm := range txInfo.Programs {
		code, err := common.HexStringToBytes(pgrm.Code)
		if err != nil {
			return nil, err
		}
		parameter, err := common.HexStringToBytes(pgrm.Parameter)
		if err != nil {
			return nil, err
		}
		txProgram := &types.Program{
			Code:      code,
			Parameter: parameter,
		}
		txPrograms = append(txPrograms, txProgram)
	}

	txTransaction := &types.Transaction{
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

func GetPayloadInfo(p types.Payload, pVersion byte) PayloadInfo {
	switch object := p.(type) {
	case *types.PayloadCoinBase:
		obj := new(CoinbaseInfo)
		obj.CoinbaseData = string(object.CoinbaseData)
		return obj
	case *types.PayloadRegisterAsset:
		obj := new(RegisterAssetInfo)
		obj.Asset = object.Asset
		obj.Amount = object.Amount.String()
		obj.Controller = common.BytesToHexString(common.BytesReverse(object.Controller.Bytes()))
		return obj
	case *types.PayloadTransferCrossChainAsset:
		obj := new(TransferCrossChainAssetInfo)
		obj.CrossChainAssets = make([]CrossChainAssetInfo, 0)
		for i := 0; i < len(object.CrossChainAddresses); i++ {
			assetInfo := CrossChainAssetInfo{
				CrossChainAddress: object.CrossChainAddresses[i],
				OutputIndex:       object.OutputIndexes[i],
				CrossChainAmount:  object.CrossChainAmounts[i].String(),
			}
			obj.CrossChainAssets = append(obj.CrossChainAssets, assetInfo)
		}
		return obj
	case *types.PayloadTransferAsset:
	case *types.PayloadRecord:
	case *types.PayloadRechargeToSideChain:
		if pVersion == types.RechargeToSideChainPayloadVersion0 {
			obj := new(RechargeToSideChainInfoV0)
			obj.MainChainTransaction = common.BytesToHexString(object.MainChainTransaction)
			obj.Proof = common.BytesToHexString(object.MerkleProof)
			return obj
		} else if pVersion == types.RechargeToSideChainPayloadVersion1 {
			obj := new(RechargeToSideChainInfoV1)
			obj.MainChainTransactionHash = ToReversedString(object.MainChainTransactionHash)
			return obj
		}
	}
	return nil
}

func GetPayload(pInfo PayloadInfo) (types.Payload, error) {
	switch object := pInfo.(type) {
	case *RegisterAssetInfo:
		obj := new(types.PayloadRegisterAsset)
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
	case *RechargeToSideChainInfoV0:
		obj := new(types.PayloadRechargeToSideChain)
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
	case *RechargeToSideChainInfoV1:
		obj := new(types.PayloadRechargeToSideChain)
		hashBytes, err := common.HexStringToBytes(object.MainChainTransactionHash)
		if err != nil {
			return nil, err
		}
		hash, err := common.Uint256FromBytes(common.BytesReverse(hashBytes))
		if err != nil {
			return nil, err
		}
		obj.MainChainTransactionHash = *hash
		return obj, nil
	case *TransferCrossChainAssetInfo:
		obj := new(types.PayloadTransferCrossChainAsset)
		obj.CrossChainAddresses = make([]string, 0)
		obj.OutputIndexes = make([]uint64, 0)
		obj.CrossChainAmounts = make([]common.Fixed64, 0)
		for _, assetInfo := range object.CrossChainAssets {
			obj.CrossChainAddresses = append(obj.CrossChainAddresses, assetInfo.CrossChainAddress)
			obj.OutputIndexes = append(obj.OutputIndexes, assetInfo.OutputIndex)
			amount, err := common.StringToFixed64(assetInfo.CrossChainAmount)
			if err != nil {
				return nil, err
			}
			obj.CrossChainAmounts = append(obj.CrossChainAmounts, *amount)
		}
		return obj, nil
	}

	return nil, errors.New("Invalid payload type.")
}
