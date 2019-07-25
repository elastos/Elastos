package servers

import (
	"bytes"
	"encoding/binary"
	"encoding/hex"
	"fmt"
	"sort"
	"strings"

	aux "github.com/elastos/Elastos.ELA/auxpow"
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/contract"
	. "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/dpos"
	"github.com/elastos/Elastos.ELA/dpos/state"
	"github.com/elastos/Elastos.ELA/elanet"
	"github.com/elastos/Elastos.ELA/elanet/pact"
	. "github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/mempool"
	"github.com/elastos/Elastos.ELA/p2p/msg"
	"github.com/elastos/Elastos.ELA/pow"
)

var (
	Compile   string
	Config    *config.Configuration
	Chain     *blockchain.BlockChain
	Store     blockchain.IChainStore
	TxMemPool *mempool.TxPool
	Pow       *pow.Service
	Server    elanet.Server
	Arbiter   *dpos.Arbitrator
	Arbiters  state.Arbitrators
)

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
		outputs[i].OutputType = uint32(v.Type)
		outputs[i].OutputPayload = getOutputPayloadInfo(v.Payload)
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
	states := make([]*PeerInfo, 0, len(peers))
	for _, peer := range peers {
		snap := peer.ToPeer().StatsSnapshot()
		states = append(states, &PeerInfo{
			NetAddress:     snap.Addr,
			Services:       pact.ServiceFlag(snap.Services).String(),
			RelayTx:        snap.RelayTx != 0,
			LastSend:       snap.LastSend.String(),
			LastRecv:       snap.LastRecv.String(),
			ConnTime:       snap.ConnTime.String(),
			TimeOffset:     snap.TimeOffset,
			Version:        snap.Version,
			Inbound:        snap.Inbound,
			StartingHeight: snap.StartingHeight,
			LastBlock:      snap.LastBlock,
			LastPingTime:   snap.LastPingTime.String(),
			LastPingMicros: snap.LastPingMicros,
		})
	}
	return ResponsePack(Success, ServerInfo{
		Compile:   Compile,
		Height:    Chain.GetHeight(),
		Version:   pact.DPOSStartVersion,
		Services:  Server.Services().String(),
		Port:      Config.NodePort,
		RPCPort:   uint16(Config.HttpJsonPort),
		RestPort:  uint16(Config.HttpRestPort),
		WSPort:    uint16(Config.HttpWsPort),
		Neighbors: states,
	})
}

func SetLogLevel(param Params) map[string]interface{} {
	level, ok := param.Int("level")
	if !ok || level < 0 {
		return ResponsePack(InvalidParams, "level must be an integer in 0-6")
	}

	log.SetPrintLevel(uint8(level))
	return ResponsePack(Success, fmt.Sprint("log level has been set to ", level))
}

func CreateAuxBlock(param Params) map[string]interface{} {
	payToAddr, ok := param.String("paytoaddress")
	if !ok {
		return ResponsePack(InvalidParams, "parameter paytoaddress not found")
	}

	block, err := Pow.CreateAuxBlock(payToAddr)
	if err != nil {
		return ResponsePack(InternalError, "generate block failed")
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
		CoinBaseValue:     block.Transactions[0].Outputs[1].Value,
		Bits:              fmt.Sprintf("%x", block.Header.Bits),
		Hash:              block.Hash().String(),
		PreviousBlockHash: Chain.CurrentBlockHash().String(),
	}
	return ResponsePack(Success, &SendToAux)
}

func SubmitAuxBlock(param Params) map[string]interface{} {
	blockHashHex, ok := param.String("blockhash")
	if !ok {
		return ResponsePack(InvalidParams, "parameter blockhash not found")
	}
	blockHash, err := common.Uint256FromHexString(blockHashHex)
	if err != nil {
		return ResponsePack(InvalidParams, "bad blockhash")
	}

	auxPow, ok := param.String("auxpow")
	if !ok {
		return ResponsePack(InvalidParams, "parameter auxpow not found")
	}
	var aux aux.AuxPow
	buf, _ := common.HexStringToBytes(auxPow)
	if err := aux.Deserialize(bytes.NewReader(buf)); err != nil {
		log.Debug("[json-rpc:SubmitAuxBlock] auxpow deserialization failed", auxPow)
		return ResponsePack(InternalError, "auxpow deserialization failed")
	}

	err = Pow.SubmitAuxBlock(blockHash, &aux)
	if err != nil {
		log.Debug(err)
		return ResponsePack(InternalError, "adding block failed")
	}

	log.Debug("AddBlock called finished and Pow.MsgBlock.MapNewBlock has been deleted completely")
	log.Info(auxPow, blockHash)
	return ResponsePack(Success, true)
}

func SubmitSidechainIllegalData(param Params) map[string]interface{} {
	if Arbiter == nil {
		return ResponsePack(InternalError, "arbiter disabled")
	}

	rawHex, ok := param.String("illegaldata")
	if !ok {
		return ResponsePack(InvalidParams, "parameter illegaldata not found")
	}

	var data payload.SidechainIllegalData
	buf, _ := common.HexStringToBytes(rawHex)
	if err := data.DeserializeUnsigned(bytes.NewReader(buf),
		payload.SidechainIllegalDataVersion); err != nil {
		log.Debug("[json-rpc:SubmitSidechainIllegalData] illegaldata deserialization failed", rawHex)
		return ResponsePack(InternalError, "illegaldata deserialization failed")
	}

	Arbiter.OnSidechainIllegalEvidenceReceived(&data)

	return ResponsePack(Success, true)
}

func GetArbiterPeersInfo(params Params) map[string]interface{} {
	if Arbiter == nil {
		return ResponsePack(InternalError, "arbiter disabled")
	}

	type peerInfo struct {
		OwnerPublicKey string `json:"ownerpublickey"`
		NodePublicKey  string `json:"nodepublickey"`
		IP             string `json:"ip"`
		ConnState      string `json:"connstate"`
	}

	peers := Arbiter.GetArbiterPeersInfo()

	result := make([]peerInfo, 0)
	for _, p := range peers {
		producer := Arbiters.GetCRCProducer(p.PID[:])
		if producer == nil {
			if producer = blockchain.DefaultLedger.Blockchain.GetState().
				GetProducer(p.PID[:]); producer == nil {
				continue
			}
		}
		result = append(result, peerInfo{
			OwnerPublicKey: common.BytesToHexString(producer.OwnerPublicKey()),
			NodePublicKey:  common.BytesToHexString(producer.NodePublicKey()),
			IP:             p.Addr,
			ConnState:      p.State.String(),
		})
	}
	return ResponsePack(Success, result)
}

func GetArbitersInfo(params Params) map[string]interface{} {
	type arbitersInfo struct {
		Arbiters               []string `json:"arbiters"`
		Candidates             []string `json:"candidates"`
		NextArbiters           []string `json:"nextarbiters"`
		NextCandidates         []string `json:"nextcandidates"`
		OnDutyArbiter          string   `json:"ondutyarbiter"`
		CurrentTurnStartHeight int      `json:"currentturnstartheight"`
		NextTurnStartHeight    int      `json:"nextturnstartheight"`
	}

	dutyIndex := Arbiters.GetDutyIndex()
	result := &arbitersInfo{
		Arbiters:       make([]string, 0),
		Candidates:     make([]string, 0),
		NextArbiters:   make([]string, 0),
		NextCandidates: make([]string, 0),
		OnDutyArbiter:  common.BytesToHexString(Arbiters.GetOnDutyArbitrator()),

		CurrentTurnStartHeight: int(Store.GetHeight()) - dutyIndex,
		NextTurnStartHeight: int(Store.GetHeight()) +
			Arbiters.GetArbitersCount() - dutyIndex,
	}
	for _, v := range Arbiters.GetArbitrators() {
		result.Arbiters = append(result.Arbiters, common.BytesToHexString(v))
	}
	for _, v := range Arbiters.GetCandidates() {
		result.Candidates = append(result.Candidates, common.BytesToHexString(v))
	}
	for _, v := range Arbiters.GetNextArbitrators() {
		result.NextArbiters = append(result.NextArbiters,
			common.BytesToHexString(v))
	}
	for _, v := range Arbiters.GetNextCandidates() {
		result.NextCandidates = append(result.NextCandidates,
			common.BytesToHexString(v))
	}
	return ResponsePack(Success, result)
}

func GetInfo(param Params) map[string]interface{} {
	RetVal := struct {
		Version       uint32 `json:"version"`
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
		Version:       pact.DPOSStartVersion,
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
		return ResponsePack(Error, err.Error())
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
		MinerInfo:         string(block.Transactions[0].Payload.(*payload.CoinBase).Content[:]),
	}
}

func GetConfirmInfo(confirm *payload.Confirm) ConfirmInfo {
	votes := make([]VoteInfo, 0)
	for _, vote := range confirm.Votes {
		votes = append(votes, VoteInfo{
			Signer: common.BytesToHexString(vote.Signer),
			Accept: vote.Accept,
		})
	}

	return ConfirmInfo{
		BlockHash:  ToReversedString(confirm.Proposal.BlockHash),
		Sponsor:    common.BytesToHexString(confirm.Proposal.Sponsor),
		ViewOffset: confirm.Proposal.ViewOffset,
		Votes:      votes,
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

func getConfirm(hash common.Uint256, verbose uint32) (interface{}, ErrCode) {
	confirm, err := Store.GetConfirm(hash)
	if err != nil {
		return "", UnknownBlock
	}
	if verbose == 0 {
		w := new(bytes.Buffer)
		confirm.Serialize(w)
		return common.BytesToHexString(w.Bytes()), Success
	}

	return GetConfirmInfo(confirm), Success
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

func GetConfirmByHeight(param Params) map[string]interface{} {
	height, ok := param.Uint("height")
	if !ok {
		return ResponsePack(InvalidParams, "height parameter should be a positive integer")
	}

	hash, err := Store.GetBlockHash(height)
	if err != nil {
		return ResponsePack(UnknownBlock, err.Error())
	}

	verbosity, ok := param.Uint("verbosity")
	if !ok {
		verbosity = 1
	}

	result, errCode := getConfirm(hash, verbosity)
	return ResponsePack(errCode, result)
}

func GetConfirmByHash(param Params) map[string]interface{} {
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

	result, error := getConfirm(hash, verbosity)
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

	if err := VerifyAndSendTx(&txn); err != nil {
		return ResponsePack(err.(ErrCode), err.Error())
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
		return ResponsePack(UnknownBlock, "not found block hash at given height")
	}

	block, _ := Store.GetBlock(hash)
	if block == nil {
		return ResponsePack(InternalError, "not found block at given height")
	}

	var arbitrators []string
	for _, data := range Arbiters.GetArbitrators() {
		arbitrators = append(arbitrators, common.BytesToHexString(data))
	}

	result := ArbitratorGroupInfo{
		OnDutyArbitratorIndex: Arbiters.GetDutyIndexByHeight(height),
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

func GetUTXOsByAmount(param Params) map[string]interface{} {
	bestHeight := Store.GetHeight()

	result := make([]UTXOInfo, 0)
	address, ok := param.String("address")
	if !ok {
		return ResponsePack(InvalidParams, "need a parameter named address!")
	}
	amountStr, ok := param.String("amount")
	if !ok {
		return ResponsePack(InvalidParams, "need a parameter named amount!")
	}
	amount, err := common.StringToFixed64(amountStr)
	if err != nil {
		return ResponsePack(InvalidParams, "invalid amount!")
	}
	programHash, err := common.Uint168FromAddress(address)
	if err != nil {
		return ResponsePack(InvalidParams, "invalid address: "+address)
	}
	unspents, err := Store.GetUnspentsFromProgramHash(*programHash)
	if err != nil {
		return ResponsePack(InvalidParams, "cannot get asset with program")
	}
	utxoType := "mixed"
	if t, ok := param.String("utxotype"); ok {
		switch t {
		case "mixed", "vote", "normal":
			utxoType = t
		default:
			return ResponsePack(InvalidParams, "invalid utxotype")
		}
	}
	totalAmount := common.Fixed64(0)
	for _, unspent := range unspents[config.ELAAssetID] {
		if totalAmount >= *amount {
			break
		}
		tx, height, err := Store.GetTransaction(unspent.TxID)
		if err != nil {
			return ResponsePack(InternalError, "unknown transaction "+
				unspent.TxID.String()+" from persisted utxo")
		}
		if utxoType == "vote" && (tx.Version < TxVersion09 ||
			tx.Version >= TxVersion09 && tx.Outputs[unspent.Index].Type != OTVote) {
			continue
		}
		if utxoType == "normal" && tx.Version >= TxVersion09 &&
			tx.Outputs[unspent.Index].Type == OTVote {
			continue
		}
		if tx.TxType == CoinBase && bestHeight-height < config.DefaultParams.CoinbaseMaturity {
			continue
		}
		totalAmount += unspent.Value
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

	if totalAmount < *amount {
		return ResponsePack(InternalError, "not enough utxo")
	}

	return ResponsePack(Success, result)
}

func GetAmountByInputs(param Params) map[string]interface{} {
	inputStr, ok := param.String("inputs")
	if !ok {
		return ResponsePack(InvalidParams, "need a parameter named inputs!")
	}

	inputBytes, _ := common.HexStringToBytes(inputStr)
	r := bytes.NewReader(inputBytes)
	count, err := common.ReadVarUint(r, 0)
	if err != nil {
		return ResponsePack(InvalidParams, "invalid inputs")
	}

	amount := common.Fixed64(0)
	for i := uint64(0); i < count; i++ {
		input := new(Input)
		if err := input.Deserialize(r); err != nil {
			return ResponsePack(InvalidParams, "invalid inputs")
		}
		tx, _, err := Store.GetTransaction(input.Previous.TxID)
		if err != nil {
			return ResponsePack(InternalError, "unknown transaction "+
				input.Previous.TxID.String()+" from persisted utxo")
		}
		amount += tx.Outputs[input.Previous.Index].Value
	}

	return ResponsePack(Success, amount.String())
}

func ListUnspent(param Params) map[string]interface{} {
	bestHeight := Store.GetHeight()

	var result []UTXOInfo
	addresses, ok := param.ArrayString("addresses")
	if !ok {
		return ResponsePack(InvalidParams, "need addresses in an array!")
	}
	utxoType := "mixed"
	if t, ok := param.String("utxotype"); ok {
		switch t {
		case "mixed", "vote", "normal":
			utxoType = t
		default:
			return ResponsePack(InvalidParams, "invalid utxotype")
		}
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
			if utxoType == "vote" && (tx.Version < TxVersion09 ||
				tx.Version >= TxVersion09 && tx.Outputs[unspent.Index].Type != OTVote) {
				continue
			}
			if utxoType == "normal" && tx.Version >= TxVersion09 && tx.Outputs[unspent.Index].Type == OTVote {
				continue
			}
			if unspent.Value == 0 {
				continue
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
	txList, ok := param.ArrayString("txs")
	if !ok {
		return ResponsePack(InvalidParams, "txs not found")
	}

	var resultTxHashes []string
	for _, txHash := range txList {
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
	OwnerPublicKey string `json:"ownerpublickey"`
	NodePublicKey  string `json:"nodepublickey"`
	Nickname       string `json:"nickname"`
	Url            string `json:"url"`
	Location       uint64 `json:"location"`
	Active         bool   `json:"active"`
	Votes          string `json:"votes"`
	State          string `json:"state"`
	RegisterHeight uint32 `json:"registerheight"`
	CancelHeight   uint32 `json:"cancelheight"`
	InactiveHeight uint32 `json:"inactiveheight"`
	IllegalHeight  uint32 `json:"illegalheight"`
	Index          uint64 `json:"index"`
}

type Producers struct {
	Producers   []Producer `json:"producers"`
	TotalVotes  string     `json:"totalvotes"`
	TotalCounts uint64     `json:"totalcounts"`
}

func ListProducers(param Params) map[string]interface{} {
	start, _ := param.Int("start")
	limit, ok := param.Int("limit")
	if !ok {
		limit = -1
	}
	s, ok := param.String("state")
	if ok {
		s = strings.ToLower(s)
	}
	var producers []*state.Producer
	switch s {
	case "all":
		producers = Chain.GetState().GetAllProducers()
	case "pending":
		producers = Chain.GetState().GetPendingProducers()
	case "active":
		producers = Chain.GetState().GetActiveProducers()
	case "inactive":
		producers = Chain.GetState().GetInactiveProducers()
	case "canceled":
		producers = Chain.GetState().GetCanceledProducers()
	case "illegal":
		producers = Chain.GetState().GetIllegalProducers()
	case "returned":
		producers = Chain.GetState().GetReturnedDepositProducers()
	default:
		producers = Chain.GetState().GetProducers()
	}

	sort.Slice(producers, func(i, j int) bool {
		if producers[i].Votes() == producers[j].Votes() {
			return bytes.Compare(producers[i].NodePublicKey(),
				producers[j].NodePublicKey()) < 0
		}
		return producers[i].Votes() > producers[j].Votes()
	})

	var ps []Producer
	var totalVotes common.Fixed64
	for i, p := range producers {
		totalVotes += p.Votes()
		producer := Producer{
			OwnerPublicKey: hex.EncodeToString(p.Info().OwnerPublicKey),
			NodePublicKey:  hex.EncodeToString(p.Info().NodePublicKey),
			Nickname:       p.Info().NickName,
			Url:            p.Info().Url,
			Location:       p.Info().Location,
			Active:         p.State() == state.Active,
			Votes:          p.Votes().String(),
			State:          p.State().String(),
			RegisterHeight: p.RegisterHeight(),
			CancelHeight:   p.CancelHeight(),
			InactiveHeight: p.InactiveSince(),
			IllegalHeight:  p.IllegalHeight(),
			Index:          uint64(i),
		}
		ps = append(ps, producer)
	}

	count := int64(len(producers))
	if limit < 0 {
		limit = count
	}
	var resultPs []Producer
	if start < count {
		end := start
		if start+limit <= count {
			end = start + limit
		} else {
			end = count
		}
		resultPs = append(resultPs, ps[start:end]...)
	}

	result := &Producers{
		Producers:   resultPs,
		TotalVotes:  totalVotes.String(),
		TotalCounts: uint64(count),
	}

	return ResponsePack(Success, result)
}

func ProducerStatus(param Params) map[string]interface{} {
	publicKey, ok := param.String("publickey")
	if !ok {
		return ResponsePack(InvalidParams, "public key not found")
	}
	publicKeyBytes, err := common.HexStringToBytes(publicKey)
	if err != nil {
		return ResponsePack(InvalidParams, "invalid public key")
	}
	if _, err = contract.PublicKeyToStandardProgramHash(publicKeyBytes); err != nil {
		return ResponsePack(InvalidParams, "invalid public key bytes")
	}
	producer := Chain.GetState().GetProducer(publicKeyBytes)
	if producer == nil {
		return ResponsePack(InvalidParams, "unknown producer public key")
	}
	return ResponsePack(Success, producer.State().String())
}

func VoteStatus(param Params) map[string]interface{} {
	address, ok := param.String("address")
	if !ok {
		return ResponsePack(InvalidParams, "address not found")
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
	for _, unspent := range unspents[config.ELAAssetID] {
		tx, _, err := Store.GetTransaction(unspent.TxID)
		if err != nil {
			return ResponsePack(InternalError, "unknown transaction "+unspent.TxID.String()+" from persisted utxo")
		}
		if tx.Outputs[unspent.Index].Type == OTVote {
			voting += unspent.Value
		}
		total += unspent.Value
	}

	pending := false
	for _, t := range TxMemPool.GetTxsInPool() {
		for _, i := range t.Inputs {
			tx, _, err := Store.GetTransaction(i.Previous.TxID)
			if err != nil {
				return ResponsePack(InternalError, "unknown transaction "+i.Previous.TxID.String()+" from persisted utxo")
			}
			if tx.Outputs[i.Previous.Index].ProgramHash.IsEqual(*programHash) {
				pending = true
			}
		}
		for _, o := range t.Outputs {
			if o.Type == OTVote && o.ProgramHash.IsEqual(*programHash) {
				pending = true
			}
		}
		if pending {
			break
		}
	}

	type voteInfo struct {
		Total   string `json:"total"`
		Voting  string `json:"voting"`
		Pending bool   `json:"pending"`
	}
	return ResponsePack(Success, &voteInfo{
		Total:   total.String(),
		Voting:  voting.String(),
		Pending: pending,
	})
}

func GetDepositCoin(param Params) map[string]interface{} {
	pk, ok := param.String("ownerpublickey")
	if !ok {
		return ResponsePack(InvalidParams, "need a param called ownerpublickey")
	}
	pkBytes, err := hex.DecodeString(pk)
	if err != nil {
		return ResponsePack(InvalidParams, "invalid publickey")
	}
	programHash, err := contract.PublicKeyToDepositProgramHash(pkBytes)
	if err != nil {
		return ResponsePack(InvalidParams, "invalid publickey to programHash")
	}
	unspends, err := Store.GetUnspentsFromProgramHash(*programHash)
	var balance common.Fixed64 = 0
	for _, u := range unspends {
		for _, v := range u {
			balance = balance + v.Value
		}
	}
	var deducted common.Fixed64 = 0
	//todo get deducted coin

	type depositCoin struct {
		Available string `json:"available"`
		Deducted  string `json:"deducted"`
	}
	return ResponsePack(Success, &depositCoin{
		Available: balance.String(),
		Deducted:  deducted.String(),
	})
}

func EstimateSmartFee(param Params) map[string]interface{} {
	confirm, ok := param.Int("confirmations")
	if !ok {
		return ResponsePack(InvalidParams, "need a param called confirmations")
	}
	if confirm > 25 {
		return ResponsePack(InvalidParams, "support only 25 confirmations at most")
	}
	var FeeRate = 10000 //basic fee rate 10000 sela per KB
	var count = 0

	// TODO just return fixed transaction fee for now, we didn't have that much
	// transactions in a block yet.

	return ResponsePack(Success, GetFeeRate(count, int(confirm))*FeeRate)
}

func GetFeeRate(count int, confirm int) int {
	gap := count - confirm
	if gap < 0 {
		gap = -1
	}
	return gap + 2
}

func getPayloadInfo(p Payload) PayloadInfo {
	switch object := p.(type) {
	case *payload.CoinBase:
		obj := new(CoinbaseInfo)
		obj.CoinbaseData = string(object.Content)
		return obj
	case *payload.RegisterAsset:
		obj := new(RegisterAssetInfo)
		obj.Asset = object.Asset
		obj.Amount = object.Amount.String()
		obj.Controller = common.BytesToHexString(common.BytesReverse(object.Controller.Bytes()))
		return obj
	case *payload.SideChainPow:
		obj := new(SideChainPowInfo)
		obj.BlockHeight = object.BlockHeight
		obj.SideBlockHash = object.SideBlockHash.String()
		obj.SideGenesisHash = object.SideGenesisHash.String()
		obj.Signature = common.BytesToHexString(object.Signature)
		return obj
	case *payload.WithdrawFromSideChain:
		obj := new(WithdrawFromSideChainInfo)
		obj.BlockHeight = object.BlockHeight
		obj.GenesisBlockAddress = object.GenesisBlockAddress
		for _, hash := range object.SideChainTransactionHashes {
			obj.SideChainTransactionHashes = append(obj.SideChainTransactionHashes, hash.String())
		}
		return obj
	case *payload.TransferCrossChainAsset:
		obj := new(TransferCrossChainAssetInfo)
		obj.CrossChainAddresses = object.CrossChainAddresses
		obj.OutputIndexes = object.OutputIndexes
		obj.CrossChainAmounts = object.CrossChainAmounts
		return obj
	case *payload.TransferAsset:
	case *payload.Record:
	case *payload.ProducerInfo:
		obj := new(ProducerInfo)
		obj.OwnerPublicKey = common.BytesToHexString(object.OwnerPublicKey)
		obj.NodePublicKey = common.BytesToHexString(object.NodePublicKey)
		obj.NickName = object.NickName
		obj.Url = object.Url
		obj.Location = object.Location
		obj.NetAddress = object.NetAddress
		obj.Signature = common.BytesToHexString(object.Signature)
		return obj
	case *payload.ProcessProducer:
		obj := new(CancelProducerInfo)
		obj.OwnerPublicKey = common.BytesToHexString(object.OwnerPublicKey)
		obj.Signature = common.BytesToHexString(object.Signature)
		return obj
	case *payload.ActivateProducer:
		obj := new(ActivateProducerInfo)
		obj.NodePublicKey = common.BytesToHexString(object.NodePublicKey)
		obj.Signature = common.BytesToHexString(object.Signature)
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

func VerifyAndSendTx(tx *Transaction) error {
	// if transaction is verified unsuccessfully then will not put it into transaction pool
	if err := TxMemPool.AppendToTxPool(tx); err != nil {
		log.Info("[httpjsonrpc] VerifyTransaction failed when AppendToTxnPool. Errcode:", err)
		return err
	}

	// Relay tx inventory to other peers.
	txHash := tx.Hash()
	iv := msg.NewInvVect(msg.InvTypeTx, &txHash)
	Server.RelayInventory(iv, tx)

	return nil
}

func ResponsePack(errCode ErrCode, result interface{}) map[string]interface{} {
	if errCode != 0 && (result == "" || result == nil) {
		result = ErrMap[errCode]
	}
	return map[string]interface{}{"Result": result, "Error": errCode}
}
