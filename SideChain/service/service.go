package service

import (
	"bytes"
	"encoding/binary"
	"encoding/json"
	"errors"
	"fmt"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/interfaces"
	"github.com/elastos/Elastos.ELA.SideChain/mempool"
	"github.com/elastos/Elastos.ELA.SideChain/pow"
	"github.com/elastos/Elastos.ELA.SideChain/server"
	"github.com/elastos/Elastos.ELA.SideChain/spv"
	"github.com/elastos/Elastos.ELA.SideChain/types"

	"github.com/elastos/Elastos.ELA/common"
	ela "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/p2p/msg"
	"github.com/elastos/Elastos.ELA/utils/elalog"
	"github.com/elastos/Elastos.ELA/utils/http"
)

type Config struct {
	Server         server.Server
	Chain          *blockchain.BlockChain
	Store          *blockchain.ChainStore
	GenesisAddress string
	TxMemPool      *mempool.TxPool
	PowService     *pow.Service
	SpvService     *spv.Service
	SetLogLevel    func(level elalog.Level)

	GetBlockInfo                func(cfg *Config, block *types.Block, verbose bool) BlockInfo
	GetTransactionInfo          func(cfg *Config, header interfaces.Header, tx *types.Transaction) *TransactionInfo
	GetTransactionInfoFromBytes func(txInfoBytes []byte) (*TransactionInfo, error)
	GetTransaction              func(cfg *Config, txInfo *TransactionInfo) (*types.Transaction, error)
	GetPayloadInfo              func(p types.Payload, pVersion byte) PayloadInfo
	GetPayload                  func(pInfo PayloadInfo) (types.Payload, error)
}

type HttpService struct {
	cfg *Config
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

func newError(err ErrorCode) *http.Error {
	return http.NewError(int(err), err.String())
}

// Input JSON string examples for getblock method as following:
func (s *HttpService) GetRawTransaction(param http.Params) (interface{}, error) {
	str, ok := param.String("txid")
	if !ok {
		return nil, newError(InvalidParams)
	}

	hex, err := FromReversedString(str)
	if err != nil {
		return nil, newError(InvalidParams)
	}
	var hash common.Uint256
	err = hash.Deserialize(bytes.NewReader(hex))
	if err != nil {
		return nil, newError(InvalidParams)
	}
	var header interfaces.Header
	tx, height, err := s.cfg.Chain.GetTransaction(hash)
	if err != nil {
		//try to find transaction in transaction pool.
		tx = s.cfg.TxMemPool.GetTransaction(hash)
		if tx == nil {
			return nil, newError(UnknownTransaction)
		}
	} else {
		bHash, err := s.cfg.Chain.GetBlockHash(height)
		if err != nil {
			return nil, newError(UnknownTransaction)
		}
		header, err = s.cfg.Chain.GetHeader(bHash)
		if err != nil {
			return nil, newError(UnknownTransaction)
		}
	}

	verbose, ok := param.Bool("verbose")
	if verbose {
		return s.cfg.GetTransactionInfo(s.cfg, header, tx), nil
	} else {
		buf := new(bytes.Buffer)
		tx.Serialize(buf)
		return common.BytesToHexString(buf.Bytes()), nil
	}
}

func (s *HttpService) GetNeighbors(param http.Params) (interface{}, error) {
	peers := s.cfg.Server.ConnectedPeers()
	neighborAddrs := make([]string, 0, len(peers))
	for _, peer := range peers {
		neighborAddrs = append(neighborAddrs, peer.ToPeer().String())
	}
	return neighborAddrs, nil
}

func (s *HttpService) SetLogLevel(param http.Params) (interface{}, error) {
	level, ok := param["level"].(float64)
	if !ok || level < 0 {
		return nil, http.NewError(int(InvalidParams), "level must be an integer in 0-6")
	}

	s.cfg.SetLogLevel(elalog.Level(level))
	return fmt.Sprint("log level has been set to ", level), nil
}

func (s *HttpService) SubmitAuxBlock(param http.Params) (interface{}, error) {
	blockHash, ok := param.String("blockhash")
	if !ok {
		return nil, newError(InvalidParams)
	}

	sideAuxPow, ok := param.String("sideauxpow")
	if !ok {
		return nil, newError(InvalidParams)
	}

	sideAuxData, _ := common.HexStringToBytes(sideAuxPow)
	err := s.cfg.PowService.SubmitAuxBlock(blockHash, sideAuxData)
	if err != nil {
		log.Warn(err)
		return nil, http.NewError(int(InvalidParams), err.Error())
	}

	return blockHash, nil
}

func (s *HttpService) CreateAuxBlock(param http.Params) (interface{}, error) {
	addr, _ := param.String("paytoaddress")

	msgBlock, curHashStr, _ := s.cfg.PowService.GenerateAuxBlock(addr)
	if nil == msgBlock {
		return nil, newError(UnknownBlock)
	}

	type SideAuxBlock struct {
		GenesisHash       string `json:"genesishash"`
		Height            uint32 `json:"height"`
		Bits              string `json:"bits"`
		Hash              string `json:"hash"`
		PreviousBlockHash string `json:"previousblockhash"`
	}

	genesisHash, err := s.cfg.Chain.GetBlockHash(uint32(0))
	if err != nil {
		return nil, http.NewError(int(InvalidParams), "get genesis hash failed")
	}
	genesisHashStr := common.BytesToHexString(genesisHash.Bytes())

	preHash := s.cfg.Chain.CurrentBlockHash()
	preHashStr := common.BytesToHexString(preHash.Bytes())

	SendToAux := SideAuxBlock{
		GenesisHash:       genesisHashStr,
		Height:            s.cfg.Chain.GetBestHeight(),
		Bits:              fmt.Sprintf("%x", msgBlock.GetBits()), //difficulty
		Hash:              curHashStr,
		PreviousBlockHash: preHashStr,
	}
	return SendToAux, nil
}

func (s *HttpService) ToggleMining(param http.Params) (interface{}, error) {
	mining, ok := param.Bool("mining")
	if !ok {
		return nil, newError(InvalidParams)
	}

	var message string
	if mining {
		go s.cfg.PowService.Start()
		message = "mining started"
	} else {
		go s.cfg.PowService.Halt()
		message = "mining stopped"
	}

	return message, nil
}

func (s *HttpService) DiscreteMining(param http.Params) (interface{}, error) {
	count, ok := param.Uint("count")
	if !ok {
		return nil, newError(InvalidParams)
	}

	ret := make([]string, count)

	blockHashes, err := s.cfg.PowService.DiscreteMining(uint32(count))
	if err != nil {
		return nil, http.NewError(int(InvalidParams), err.Error())
	}

	for i, hash := range blockHashes {
		ret[i] = ToReversedString(*hash)
	}

	return ret, nil
}

func (s *HttpService) GetConnectionCount(param http.Params) (interface{}, error) {
	return s.cfg.Server.ConnectedCount(), nil
}

func (s *HttpService) GetTransactionPool(param http.Params) (interface{}, error) {
	txs := make([]*TransactionInfo, 0)
	for _, t := range s.cfg.TxMemPool.GetTxsInPool() {
		txs = append(txs, s.cfg.GetTransactionInfo(s.cfg, nil, t))
	}
	return txs, nil
}

func (s *HttpService) getBlock(hash common.Uint256, format uint) (interface{}, error) {
	block, err := s.cfg.Chain.GetBlockByHash(hash)
	if err != nil {
		return "", newError(UnknownBlock)
	}
	switch format {
	case 0:
		w := new(bytes.Buffer)
		block.Serialize(w)
		return common.BytesToHexString(w.Bytes()), nil
	case 2:
		return s.cfg.GetBlockInfo(s.cfg, block, true), nil
	}
	return s.cfg.GetBlockInfo(s.cfg, block, false), nil
}

func (s *HttpService) GetBlockByHash(param http.Params) (interface{}, error) {
	str, ok := param.String("blockhash")
	if !ok {
		return nil, http.NewError(int(InvalidParams), "block hash not found")
	}

	var hash common.Uint256
	hashBytes, err := FromReversedString(str)
	if err != nil {
		return nil, http.NewError(int(InvalidParams), "invalid block hash")
	}
	if err := hash.Deserialize(bytes.NewReader(hashBytes)); err != nil {
		return nil, http.NewError(int(InvalidParams), "invalid block hash")
	}

	verbosity, ok := param.Uint("verbosity")
	if !ok {
		verbosity = 1
	}

	return s.getBlock(hash, verbosity)
}

func (s *HttpService) SendRechargeToSideChainTxByHash(param http.Params) (interface{}, error) {
	txid, ok := param.String("txid")
	if !ok {
		return nil, http.NewError(int(InvalidParams), "txid not found")
	}

	txBytes, err := common.HexStringToBytes(txid)
	if err != nil {
		return nil, http.NewError(int(InvalidParams), "invalid txid")
	}

	hash, err := common.Uint256FromBytes(txBytes)
	if err != nil {
		return nil, http.NewError(int(InvalidParams), "to tx hash failed")
	}

	tx, err := s.cfg.SpvService.GetTransaction(hash)
	if err != nil {
		return nil, http.NewError(int(InvalidParams), "invalid tx hash")
	}

	depositTx, err := createRechargeToSideChainTransaction(tx, s.cfg.GenesisAddress)
	if err != nil {
		return nil, http.NewError(int(InvalidParams), "create recharge tx failed")
	}

	if err := s.verifyAndSendTx(depositTx); err != nil {
		return nil, ruleError(err)
	}
	return depositTx.Hash().String(), nil
}

func createRechargeToSideChainTransaction(tx *ela.Transaction, genesisAddress string) (*types.Transaction, error) {
	rechargeInfo, err := parseRechargeToSideChainTransactionInfo(tx, genesisAddress)
	if err != nil {
		return nil, err
	}

	return createRechargeToSideChainTransactionByInfo(rechargeInfo)
}

type RechargeToSideChainAsset struct {
	TargetAddress    string
	Amount           *common.Fixed64
	CrossChainAmount *common.Fixed64
}

type RechargeToSideChainInfo struct {
	MainChainTransactionHash *common.Uint256
	DepositAssets            []*RechargeToSideChainAsset
}

func parseRechargeToSideChainTransactionInfo(txn *ela.Transaction, genesisAddress string) (*RechargeToSideChainInfo, error) {
	result := new(RechargeToSideChainInfo)
	payloadObj, ok := txn.Payload.(*payload.TransferCrossChainAsset)
	if !ok {
		return nil, errors.New("Invalid payload")
	}
	if len(txn.Outputs) == 0 {
		return nil, errors.New("Invalid TransferCrossChainAsset payload, outputs is null")
	}
	programHash, err := common.Uint168FromAddress(genesisAddress)
	if err != nil {
		return nil, errors.New("Invalid genesis address")
	}
	hash := txn.Hash()
	result.MainChainTransactionHash = &hash
	result.DepositAssets = make([]*RechargeToSideChainAsset, 0)
	for i := 0; i < len(payloadObj.CrossChainAddresses); i++ {
		if txn.Outputs[payloadObj.OutputIndexes[i]].ProgramHash.IsEqual(*programHash) {
			result.DepositAssets = append(result.DepositAssets, &RechargeToSideChainAsset{
				TargetAddress:    payloadObj.CrossChainAddresses[i],
				Amount:           &txn.Outputs[payloadObj.OutputIndexes[i]].Value,
				CrossChainAmount: &payloadObj.CrossChainAmounts[i],
			})
		}
	}
	return result, nil
}

func createRechargeToSideChainTransactionByInfo(txInfo *RechargeToSideChainInfo) (*types.Transaction, error) {
	// create payload
	payload := new(types.PayloadRechargeToSideChain)
	payload.MainChainTransactionHash = *txInfo.MainChainTransactionHash

	var txOutputs []*types.Output
	for _, output := range txInfo.DepositAssets {
		asset := types.GetSystemAssetId()
		assetId := &asset

		programHash, err := common.Uint168FromAddress(output.TargetAddress)
		if err != nil {
			return nil, err
		}
		output := &types.Output{
			AssetID:     *assetId,
			Value:       *output.CrossChainAmount,
			OutputLock:  0,
			ProgramHash: *programHash,
		}
		txOutputs = append(txOutputs, output)
	}

	txTransaction := &types.Transaction{
		TxType:         types.RechargeToSideChain,
		PayloadVersion: types.RechargeToSideChainPayloadVersion1,
		Payload:        payload,
		Outputs:        txOutputs,
	}
	return txTransaction, nil
}

func (s *HttpService) SendRawTransaction(param http.Params) (interface{}, error) {
	str, ok := param.String("data")
	if !ok {
		return nil, http.NewError(int(InvalidParams), "need a string parameter named data")
	}

	bys, err := common.HexStringToBytes(str)
	if err != nil {
		return nil, http.NewError(int(InvalidParams), "hex string to bytes error:"+err.Error())
	}
	var txn types.Transaction
	if err := txn.Deserialize(bytes.NewReader(bys)); err != nil {
		return nil, http.NewError(int(InvalidTransaction), "transaction deserialize error:"+err.Error())
	}

	if err := s.verifyAndSendTx(&txn); err != nil {
		return nil, ruleError(err)
	}

	return ToReversedString(txn.Hash()), nil
}

func (s *HttpService) GetBlockHeight(param http.Params) (interface{}, error) {
	return s.cfg.Chain.GetBestHeight(), nil
}

func (s *HttpService) GetBlockCount(param http.Params) (interface{}, error) {
	return s.cfg.Chain.GetBestHeight() + 1, nil
}

func (s *HttpService) GetBlockHash(param http.Params) (interface{}, error) {
	height, ok := param.Uint32("height")
	if !ok {
		return nil, http.NewError(int(InvalidParams), " height parameter should be a positive integer")
	}

	hash, err := s.cfg.Chain.GetBlockHash(height)
	if err != nil {
		return nil, newError(InvalidParams)
	}
	return ToReversedString(hash), nil
}

func (s *HttpService) GetBestBlockHash(param http.Params) (interface{}, error) {
	height := s.cfg.Chain.GetBestHeight()
	hash, err := s.cfg.Chain.GetBlockHash(height)
	if err != nil {
		return nil, newError(InvalidParams)
	}
	return ToReversedString(hash), nil
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
		Height:       block.Header.GetHeight(),
		Transactions: trans,
	}
	return b
}

func (s *HttpService) GetTransactionsByHeight(param http.Params) (interface{}, error) {
	height, ok := param.Uint("height")
	if !ok {
		return nil, http.NewError(int(InvalidParams), "height parameter should be a positive integer")
	}

	hash, err := s.cfg.Chain.GetBlockHash(uint32(height))
	if err != nil {
		return nil, newError(UnknownBlock)

	}
	block, err := s.cfg.Chain.GetBlockByHash(hash)
	if err != nil {
		return nil, newError(UnknownBlock)
	}
	return s.getBlockTransactions(block), nil
}

func (s *HttpService) GetBlockByHeight(param http.Params) (interface{}, error) {
	height, ok := param.Uint("height")
	if !ok {
		return nil, http.NewError(int(InvalidParams), "height parameter should be a positive integer")
	}

	hash, err := s.cfg.Chain.GetBlockHash(uint32(height))
	if err != nil {
		return nil, newError(UnknownBlock)
	}

	return s.getBlock(hash, 2)
}

//Asset
func (s *HttpService) GetAssetByHash(param http.Params) (interface{}, error) {
	str, ok := param.String("hash")
	if !ok {
		return nil, newError(InvalidParams)
	}
	hashBytes, err := FromReversedString(str)
	if err != nil {
		return nil, newError(InvalidParams)
	}
	var hash common.Uint256
	err = hash.Deserialize(bytes.NewReader(hashBytes))
	if err != nil {
		return nil, newError(InvalidAsset)
	}
	asset, err := s.cfg.Chain.GetAsset(hash)
	if err != nil {
		return nil, newError(UnknownAsset)
	}
	if false {
		w := new(bytes.Buffer)
		asset.Serialize(w)
		return common.BytesToHexString(w.Bytes()), nil
	}
	return asset, nil
}

func (s *HttpService) GetBalanceByAddr(param http.Params) (interface{}, error) {
	str, ok := param.String("addr")
	if !ok {
		return nil, newError(InvalidParams)
	}

	programHash, err := common.Uint168FromAddress(str)
	if err != nil {
		return nil, newError(InvalidParams)
	}
	unspends, err := s.cfg.Chain.GetUnspents(*programHash)
	var balance common.Fixed64 = 0
	for _, u := range unspends {
		for _, v := range u {
			balance = balance + v.Value
		}
	}
	return balance.String(), nil
}

func (s *HttpService) GetReceivedByAddress(param http.Params) (interface{}, error) {
	str, ok := param.String("address")
	if !ok {
		return nil, newError(InvalidParams)
	}

	programHash, err := common.Uint168FromAddress(str)
	if err != nil {
		return nil, newError(InvalidParams)
	}
	unspends, err := s.cfg.Chain.GetUnspents(*programHash)
	var balance common.Fixed64 = 0
	for _, u := range unspends {
		for _, v := range u {
			balance = balance + v.Value
		}
	}
	return balance.String(), nil
}

func (s *HttpService) GetBalanceByAsset(param http.Params) (interface{}, error) {
	addr, ok := param.String("addr")
	if !ok {
		return nil, newError(InvalidParams)
	}

	programHash, err := common.Uint168FromAddress(addr)
	if err != nil {
		return nil, newError(InvalidParams)
	}

	assetIdStr, ok := param.String("assetid")
	if !ok {
		return nil, newError(InvalidParams)
	}
	assetIdBytes, err := FromReversedString(assetIdStr)
	if err != nil {
		return nil, newError(InvalidParams)
	}
	assetId, err := common.Uint256FromBytes(assetIdBytes)
	if err != nil {
		return nil, newError(InvalidParams)
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
	return balance.String(), nil
}

func (s *HttpService) GetUnspendsByAddr(param http.Params) (interface{}, error) {
	addr, ok := param.String("addr")
	if !ok {
		return nil, newError(InvalidParams)
	}

	programHash, err := common.Uint168FromAddress(addr)
	if err != nil {
		return nil, newError(InvalidParams)
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
			return nil, newError(InternalError)
		}
		var unspendsInfo []UTXOUnspentInfo
		for _, v := range u {
			unspendsInfo = append(unspendsInfo, UTXOUnspentInfo{ToReversedString(v.TxId), v.Index, v.Value.String()})
		}
		results = append(results, Result{ToReversedString(k), asset.Name, unspendsInfo})
	}
	return results, nil
}

func (s *HttpService) GetUnspendsByAsset(param http.Params) (interface{}, error) {
	addr, ok := param.String("addr")
	if !ok {
		return nil, newError(InvalidParams)
	}
	programHash, err := common.Uint168FromAddress(addr)
	if err != nil {
		return nil, newError(InvalidParams)
	}
	assetId, ok := param.String("assetid")
	if !ok {
		return nil, newError(InvalidParams)
	}
	bys, err := FromReversedString(assetId)
	if err != nil {
		return nil, newError(InvalidParams)
	}

	var assetHash common.Uint256
	if err := assetHash.Deserialize(bytes.NewReader(bys)); err != nil {
		return nil, newError(InvalidParams)
	}
	type UTXOUnspentInfo struct {
		Txid  string
		Index uint32
		Value string
	}
	infos, err := s.cfg.Chain.GetAssetUnspents(*programHash, assetHash)
	if err != nil {
		return nil, newError(InvalidParams)

	}
	var UTXOoutputs []UTXOUnspentInfo
	for _, v := range infos {
		UTXOoutputs = append(UTXOoutputs, UTXOUnspentInfo{Txid: ToReversedString(v.TxId), Index: v.Index, Value: v.Value.String()})
	}
	return UTXOoutputs, nil
}

func (s *HttpService) GetAssetList(params http.Params) (interface{}, error) {
	assets := s.cfg.Chain.GetAssets()

	var assetInfo []AssetInfo
	for hash, asset := range assets {
		assetInfo = append(assetInfo, AssetInfo{
			hash.String(),
			asset.Name,
			asset.Description,
			int(asset.Precision),
			int(asset.AssetType),
			int(asset.RecordType),
		})
	}
	return assets, nil
}

//Transaction
func (s *HttpService) GetTransactionByHash(param http.Params) (interface{}, error) {
	str, ok := param.String("hash")
	if !ok {
		return nil, newError(InvalidParams)
	}

	bys, err := FromReversedString(str)
	if err != nil {
		return nil, newError(InvalidParams)
	}

	var hash common.Uint256
	err = hash.Deserialize(bytes.NewReader(bys))
	if err != nil {
		return nil, newError(InvalidTransaction)
	}
	txn, height, err := s.cfg.Chain.GetTransaction(hash)
	if err != nil {
		return nil, newError(UnknownTransaction)
	}
	bHash, err := s.cfg.Chain.GetBlockHash(height)
	if err != nil {
		return nil, newError(UnknownBlock)
	}
	header, err := s.cfg.Chain.GetHeader(bHash)
	if err != nil {
		return nil, newError(UnknownBlock)
	}

	return s.cfg.GetTransactionInfo(s.cfg, header, txn), nil
}

func GetStringArray(param http.Params, key string) ([]string, bool) {
	value, ok := param[key]
	if !ok {
		return nil, false
	}
	switch v := value.(type) {
	case []interface{}:
		var arrayString []string
		for _, param := range v {
			paramString, ok := param.(string)
			if !ok {
				return nil, false
			}
			arrayString = append(arrayString, paramString)
		}
		return arrayString, true
	default:
		return nil, false
	}
}

func (s *HttpService) GetExistDepositTransactions(param http.Params) (interface{}, error) {
	txs, ok := GetStringArray(param, "txs")
	if !ok {
		return nil, http.NewError(int(InvalidParams), "txs not found")
	}

	var resultTxHashes []string
	for _, txHash := range txs {
		txHashBytes, err := common.HexStringToBytes(txHash)
		if err != nil {
			return nil, newError(InvalidParams)
		}
		hash, err := common.Uint256FromBytes(txHashBytes)
		if err != nil {
			return nil, newError(InvalidParams)
		}
		inStore := s.cfg.Chain.IsDuplicateMainchainTx(*hash)
		inTxPool := s.cfg.TxMemPool.IsDuplicateMainChainTx(*hash)
		if inTxPool || inStore {
			resultTxHashes = append(resultTxHashes, txHash)
		}
	}

	return resultTxHashes, nil
}

func (s *HttpService) GetBlockTransactionsDetail(block *types.Block, filter func(*types.Transaction) bool) []*types.Transaction {
	var trans []*types.Transaction
	for _, tx := range block.Transactions {
		if !filter(tx) {
			continue
		}
		trans = append(trans, tx)
	}

	return trans
}

func (s *HttpService) GetWithdrawTxsInfo(txs []*types.Transaction) interface{} {
	var trans []*WithdrawTxInfo
	for _, tx := range txs {
		payload, ok := tx.Payload.(*types.PayloadTransferCrossChainAsset)
		if !ok {
			continue
		}

		var txOuputsInfo []*WithdrawOutputInfo
		for i := 0; i < len(payload.CrossChainAmounts); i++ {
			txOuputsInfo = append(txOuputsInfo, &WithdrawOutputInfo{
				CrossChainAddress: payload.CrossChainAddresses[i],
				CrossChainAmount:  payload.CrossChainAmounts[i].String(),
				OutputAmount:      tx.Outputs[payload.OutputIndexes[i]].Value.String(),
			})
		}

		txWithdraw := &WithdrawTxInfo{
			TxID:             ToReversedString(tx.Hash()),
			CrossChainAssets: txOuputsInfo,
		}

		trans = append(trans, txWithdraw)
	}

	return trans
}

type WithdrawOutputInfo struct {
	CrossChainAddress string `json:"crosschainaddress"`
	CrossChainAmount  string `json:"crosschainamount"`
	OutputAmount      string `json:"outputamount"`
}

type WithdrawTxInfo struct {
	TxID             string                `json:"txid"`
	CrossChainAssets []*WithdrawOutputInfo `json:"crosschainassets"`
}

func (s *HttpService) GetWithdrawTransactionsByHeight(param http.Params) (interface{}, error) {
	height, ok := param.Uint("height")
	if !ok {
		return nil, http.NewError(int(InvalidParams), "height parameter should be a positive integer")
	}

	hash, err := s.cfg.Chain.GetBlockHash(uint32(height))
	if err != nil {
		return nil, newError(UnknownBlock)

	}
	block, err := s.cfg.Chain.GetBlockByHash(hash)
	if err != nil {
		return nil, newError(UnknownBlock)
	}

	destroyHash := common.Uint168{}
	txs := s.GetBlockTransactionsDetail(block, func(tran *types.Transaction) bool {
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
	})
	return s.GetWithdrawTxsInfo(txs), nil
}

func (s *HttpService) GetWithdrawTransactionByHash(param http.Params) (interface{}, error) {
	str, ok := param.String("txid")
	if !ok {
		return nil, http.NewError(int(InvalidParams), "txid not found")
	}
	hex, err := FromReversedString(str)
	if err != nil {
		return nil, http.NewError(int(InvalidParams), "txid reverse failed")
	}
	var hash common.Uint256
	err = hash.Deserialize(bytes.NewReader(hex))
	if err != nil {
		return nil, http.NewError(int(InvalidTransaction), "txid deserialize failed")
	}
	tx, _, err := s.cfg.Chain.GetTransaction(hash)
	if err != nil {
		return nil, http.NewError(int(UnknownTransaction), "get tx by txid failed")
	}
	payload, ok := tx.Payload.(*types.PayloadTransferCrossChainAsset)
	if !ok {
		return nil, http.NewError(int(UnknownTransaction), "get tx by txid failed")
	}

	var txOuputsInfo []*WithdrawOutputInfo
	for i := 0; i < len(payload.CrossChainAmounts); i++ {
		txOuputsInfo = append(txOuputsInfo, &WithdrawOutputInfo{
			CrossChainAddress: payload.CrossChainAddresses[i],
			CrossChainAmount:  payload.CrossChainAmounts[i].String(),
			OutputAmount:      tx.Outputs[payload.OutputIndexes[i]].Value.String(),
		})
	}

	txWithdraw := WithdrawTxInfo{
		TxID:             ToReversedString(tx.Hash()),
		CrossChainAssets: txOuputsInfo,
	}

	return txWithdraw, nil
}

type SidechainIllegalDataInfo struct {
	IllegalType     uint8  `json:"illegaltype"`
	Height          uint32 `json:"height"`
	IllegalSigner   string `json:"illegalsigner"`
	Evidence        string `json:"evidence"`
	CompareEvidence string `json:"compareevidence"`
}

func (s *HttpService) GetIllegalEvidenceByHeight(param http.Params) (interface{}, error) {
	height, ok := param.Uint("height")
	if !ok {
		return nil, http.NewError(int(InvalidParams), "height parameter should be a positive integer")
	}

	hash, err := s.cfg.Chain.GetBlockHash(uint32(height))
	if err != nil {
		return nil, newError(UnknownBlock)

	}
	_, err = s.cfg.Chain.GetBlockByHash(hash)
	if err != nil {
		return nil, newError(UnknownBlock)
	}

	// todo get illegal evidences in block

	result := make([]*SidechainIllegalDataInfo, 0)
	return result, nil
}

func (s *HttpService) CheckIllegalEvidence(param http.Params) (interface{}, error) {
	evidence, ok := param["evidence"]
	if !ok {
		return nil, http.NewError(int(InvalidParams), "no evidence")
	}
	e := new(SidechainIllegalDataInfo)
	if err := Unmarshal(evidence, e); err != nil {
		log.Error("[CheckIllegalEvidence] received invalid evidence")
		return false, err
	}

	// todo check illegal evidence

	return false, nil
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
	// if transaction is not valid, then it will not put into mem pool.
	if err := s.cfg.TxMemPool.AppendToTxPool(tx); err != nil {
		if e, ok := err.(mempool.RuleError); ok {
			log.Errorf("adding transaction pool error, "+
				"error %s, %s", e.ErrorCode, e.Description)
		}
		return err
	}
	hash := tx.Hash()
	iv := msg.NewInvVect(msg.InvTypeTx, &hash)
	s.cfg.Server.RelayInventory(iv, tx)
	return nil
}

func GetBlockInfo(cfg *Config, block *types.Block, verbose bool) BlockInfo {
	header := block.Header
	var txs []interface{}
	if verbose {
		for _, tx := range block.Transactions {
			txs = append(txs, cfg.GetTransactionInfo(cfg, header, tx))
		}
	} else {
		for _, tx := range block.Transactions {
			txs = append(txs, ToReversedString(tx.Hash()))
		}
	}
	var versionBytes [4]byte
	binary.BigEndian.PutUint32(versionBytes[:], header.GetVersion())

	var chainWork [4]byte
	binary.BigEndian.PutUint32(chainWork[:], cfg.Chain.GetBestHeight()-header.GetHeight())

	nextBlockHash, _ := cfg.Chain.GetBlockHash(header.GetHeight() + 1)

	auxPow := new(bytes.Buffer)
	header.GetAuxPow().Serialize(auxPow)

	var minerInfo string
	if block.GetHeight() == 0 {
		minerInfo = "ELA"
	} else {
		minerInfo = string(block.Transactions[0].Payload.(*types.PayloadCoinBase).CoinbaseData[:])
	}

	return BlockInfo{
		Hash:              ToReversedString(block.Hash()),
		Confirmations:     cfg.Chain.GetBestHeight() - header.GetHeight() + 1,
		StrippedSize:      uint32(block.GetSize()),
		Size:              uint32(block.GetSize()),
		Weight:            uint32(block.GetSize() * 4),
		Height:            header.GetHeight(),
		Version:           header.GetVersion(),
		VersionHex:        common.BytesToHexString(versionBytes[:]),
		MerkleRoot:        ToReversedString(header.GetMerkleRoot()),
		Tx:                txs,
		Time:              header.GetTimeStamp(),
		MedianTime:        header.GetTimeStamp(),
		Nonce:             header.GetNonce(),
		Bits:              header.GetBits(),
		Difficulty:        cfg.Chain.CalcCurrentDifficulty(header.GetBits()),
		ChainWork:         common.BytesToHexString(chainWork[:]),
		PreviousBlockHash: ToReversedString(header.GetPrevious()),
		NextBlockHash:     ToReversedString(nextBlockHash),
		AuxPow:            common.BytesToHexString(auxPow.Bytes()),
		MinerInfo:         minerInfo,
	}
}

func GetTransactionInfo(cfg *Config, header interfaces.Header, tx *types.Transaction) *TransactionInfo {
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
		confirmations = cfg.Chain.GetBestHeight() - header.GetHeight() + 1
		blockHash = ToReversedString(header.Hash())
		time = header.GetTimeStamp()
		blockTime = header.GetTimeStamp()
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

func ruleError(err error) error {
	ruleErr, ok := err.(mempool.RuleError)
	if ok {
		return http.NewError(int(ruleErr.ErrorCode), ruleErr.Error())
	}
	return http.NewError(int(InvalidTransaction), err.Error())
}
