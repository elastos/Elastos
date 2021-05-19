package service

import (
	"bytes"
	"encoding/json"
	"errors"
	"time"

	"github.com/elastos/Elastos.ELA.SideChain.ID/blockchain"
	id "github.com/elastos/Elastos.ELA.SideChain.ID/types"
	"github.com/elastos/Elastos.ELA.SideChain/interfaces"
	"github.com/elastos/Elastos.ELA.SideChain/service"
	"github.com/elastos/Elastos.ELA.SideChain/types"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/elanet/pact"
	"github.com/elastos/Elastos.ELA/utils/http"
)

type Config struct {
	service.Config
	Compile  string
	NodePort uint16
	RPCPort  uint16
	RestPort uint16
	WSPort   uint16
	Store    *blockchain.IDChainStore
}

type HttpService struct {
	*service.HttpService
	cfg   *Config
	store *blockchain.IDChainStore
}

type DidDocState uint8

const (
	Valid = iota
	Expired
	Deactivated
	NonExist
)

func (c DidDocState) String() string {
	switch c {
	case Valid:
		return "Valid"
	case Expired:
		return "Expired"
	case Deactivated:
		return "Deactivated"
	case NonExist:
		return "NonExist"
	default:
		return "Unknown"
	}
}

func NewHttpService(cfg *Config) *HttpService {
	server := &HttpService{
		HttpService: service.NewHttpService(&cfg.Config),
		cfg:         cfg,
		store:       cfg.Store,
	}
	return server
}

func (s *HttpService) GetNodeState(param http.Params) (interface{}, error) {
	peers := s.cfg.Server.ConnectedPeers()
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
	return ServerInfo{
		Compile:   s.cfg.Compile,
		Height:    s.cfg.Chain.GetBestHeight(),
		Version:   pact.DPOSStartVersion,
		Services:  s.cfg.Server.Services().String(),
		Port:      s.cfg.NodePort,
		RPCPort:   s.cfg.RPCPort,
		RestPort:  s.cfg.RestPort,
		WSPort:    s.cfg.WSPort,
		Neighbors: states,
	}, nil
}

// payload of DID transaction
type RpcPayloadDIDInfo struct {
	DID        string                `json:"did"`
	Status     int                   `json:"status"`
	RpcTXDatas []RpcTranasactionData `json:"transaction,omitempty"`
}

type RpcOperation struct {
	Header  id.DIDHeaderInfo `json:"header"`
	Payload string           `json:"payload"`
	Proof   id.DIDProofInfo  `json:"proof"`
}

type RpcTranasactionData struct {
	TXID      string       `json:"txid"`
	Timestamp string       `json:"timestamp"`
	Operation RpcOperation `json:"operation"`
}

func (rpcTxData *RpcTranasactionData) FromTranasactionData(txData id.
	TranasactionData) bool {
	hash, err := common.Uint256FromHexString(txData.TXID)
	if err != nil {
		return false
	}

	rpcTxData.TXID = service.ToReversedString(*hash)
	rpcTxData.Timestamp = txData.Timestamp
	rpcTxData.Operation.Header = txData.Operation.Header
	rpcTxData.Operation.Payload = txData.Operation.Payload
	rpcTxData.Operation.Proof = txData.Operation.Proof
	return true
}

func (s *HttpService) getTxTime(txid string) (error, uint32) {
	hash, err := common.Uint256FromHexString(txid)
	if err != nil {
		return errors.New("txid error"), 0
	}

	var header interfaces.Header
	_, height, err := s.cfg.Chain.GetTransaction(*hash)

	bHash, err := s.cfg.Chain.GetBlockHash(height)
	if err != nil {
		return errors.New("unkown block"), 0
	}
	header, err = s.cfg.Chain.GetHeader(bHash)
	if err != nil {
		return errors.New("unkown block header"), 0

	}
	return nil, header.GetTimeStamp()
}

func (s *HttpService) ResolveDID(param http.Params) (interface{}, error) {
	var didDocState DidDocState
	didDocState = NonExist
	idParam, ok := param.String("did")
	if !ok {
		return nil, http.NewError(int(service.InvalidParams), "did is null")
	}

	var did string
	//remove DID_ELASTOS_PREFIX
	if id.IsURIHasPrefix(idParam) {
		did = id.GetDIDFromUri(idParam)
	} else {
		did = idParam
	}

	//check is valid address
	_, err := common.Uint168FromAddress(did)
	if err != nil {
		return nil, http.NewError(int(service.InvalidParams), "invalid did")
	}
	isGetAll, ok := param.Bool("all")
	if !ok {
		return nil, http.NewError(int(service.InvalidParams), "all is null")
	}

	buf := new(bytes.Buffer)
	buf.WriteString(did)

	var rpcPayloadDid RpcPayloadDIDInfo

	expiresHeight, err := s.store.GetExpiresHeight(buf.Bytes())
	if err != nil {
		rpcPayloadDid.DID = idParam
		rpcPayloadDid.Status = NonExist
		return rpcPayloadDid, nil

	}

	var txsData []id.TranasactionData
	if isGetAll {
		txsData, err = s.store.GetAllDIDTxTxData(buf.Bytes())
		if err != nil {
			return nil, http.NewError(int(service.InternalError),
				"get did transaction failed")
		}

	} else {
		txData, err := s.store.GetLastDIDTxData(buf.Bytes())
		if err != nil {
			return nil, http.NewError(int(service.InternalError),
				"get did transactions failed")
		}
		if txData != nil {
			txsData = append(txsData, *txData)
		}
	}
	for index, txData := range txsData {
		rpcPayloadDid.DID = txData.Operation.PayloadInfo.ID
		err, timestamp := s.getTxTime(txData.TXID)
		if err != nil {
			continue
		}
		tempTXData := new(RpcTranasactionData)
		succe := tempTXData.FromTranasactionData(txData)
		if succe == false {
			continue
		}

		tempTXData.Timestamp = time.Unix(int64(timestamp), 0).UTC().Format(time.RFC3339)
		rpcPayloadDid.RpcTXDatas = append(rpcPayloadDid.RpcTXDatas, *tempTXData)
		if index == 0 {
			if txData.Operation.Header.Operation == "deactivate" {
				didDocState = Deactivated
			} else {
				didDocState = Valid
				if s.store.ChainStore.GetHeight() > expiresHeight {
					didDocState = Expired
				}
			}
			rpcPayloadDid.Status = int(didDocState)
		}
	}
	return rpcPayloadDid, nil

}

func (s *HttpService) GetIdentificationTxByIdAndPath(param http.Params) (interface{}, error) {
	id, ok := param.String("id")
	if !ok {
		return nil, http.NewError(int(service.InvalidParams), "id is null")
	}
	_, err := common.Uint168FromAddress(id)
	if err != nil {
		return nil, http.NewError(int(service.InvalidParams), "invalid id")
	}
	path, ok := param.String("path")
	if !ok {
		return nil, http.NewError(int(service.InvalidParams), "path is null")
	}

	buf := new(bytes.Buffer)
	buf.WriteString(id)
	buf.WriteString(path)
	txHashBytes, err := s.store.GetRegisterIdentificationTx(buf.Bytes())
	if err != nil {
		return nil, http.NewError(int(service.UnknownTransaction), "get identification transaction failed")
	}
	txHash, err := common.Uint256FromBytes(txHashBytes)
	if err != nil {
		return nil, http.NewError(int(service.InvalidTransaction), "invalid transaction hash")
	}

	txn, height, err := s.store.GetTransaction(*txHash)
	if err != nil {
		return nil, http.NewError(int(service.UnknownTransaction), "get transaction failed")
	}
	bHash, err := s.store.GetBlockHash(height)
	if err != nil {
		return nil, http.NewError(int(service.UnknownBlock), "get block failed")
	}
	header, err := s.store.GetHeader(bHash)
	if err != nil {
		return nil, http.NewError(int(service.UnknownBlock), "get header failed")
	}

	return s.cfg.GetTransactionInfo(&s.cfg.Config, header, txn), nil
}

func (s *HttpService) ListUnspent(param http.Params) (interface{}, error) {
	bestHeight := s.cfg.Store.GetHeight()
	type UTXOInfo struct {
		AssetId       string `json:"assetid"`
		Txid          string `json:"txid"`
		VOut          uint32 `json:"vout"`
		Address       string `json:"address"`
		Amount        string `json:"amount"`
		Confirmations uint32 `json:"confirmations"`
		OutputLock    uint32 `json:"outputlock"`
	}

	var results []UTXOInfo

	if _, ok := param["addresses"]; !ok {
		return nil, errors.New("need a param called address")
	}
	var addressStrings []string
	switch addresses := param["addresses"].(type) {
	case []interface{}:
		for _, v := range addresses {
			str, ok := v.(string)
			if !ok {
				return nil, errors.New("please send a string")
			}
			addressStrings = append(addressStrings, str)
		}
	default:
		return nil, errors.New("wrong type")
	}

	for _, address := range addressStrings {
		programHash, err := common.Uint168FromAddress(address)
		if err != nil {
			return nil, errors.New("Invalid address: " + address)
		}
		differentAssets, err := s.cfg.Chain.GetUnspents(*programHash)
		if err != nil {
			return nil, errors.New("cannot get asset with program")
		}
		for _, asset := range differentAssets {
			for _, unspent := range asset {
				tx, height, err := s.cfg.Chain.GetTransaction(unspent.TxId)
				if err != nil {
					return nil, errors.New("unknown transaction " + unspent.TxId.String() + " from persisted utxo")
				}
				elaAssetID := types.GetSystemAssetId()
				results = append(results, UTXOInfo{
					Amount:        unspent.Value.String(),
					AssetId:       common.BytesToHexString(common.BytesReverse(elaAssetID[:])),
					Txid:          common.BytesToHexString(common.BytesReverse(unspent.TxId[:])),
					VOut:          unspent.Index,
					Address:       address,
					Confirmations: bestHeight - height + 1,
					OutputLock:    tx.Outputs[unspent.Index].OutputLock,
				})
			}
		}
	}

	return results, nil
}

func GetTransactionInfoFromBytes(txInfoBytes []byte) (*service.TransactionInfo, error) {
	var txInfo service.TransactionInfo
	err := json.Unmarshal(txInfoBytes, &txInfo)
	if err != nil {
		return nil, errors.New("InvalidParameter")
	}

	var assetInfo service.PayloadInfo
	switch txInfo.TxType {
	case types.CoinBase:
		assetInfo = &service.CoinbaseInfo{}
	case types.RegisterAsset:
		assetInfo = &service.RegisterAssetInfo{}
	case types.SideChainPow:
		assetInfo = &service.SideChainPowInfo{}
	case types.RechargeToSideChain:
		if txInfo.PayloadVersion == types.RechargeToSideChainPayloadVersion0 {
			assetInfo = &service.RechargeToSideChainInfoV0{}
		} else if txInfo.PayloadVersion == types.RechargeToSideChainPayloadVersion1 {
			assetInfo = &service.RechargeToSideChainInfoV1{}
		}
	case types.TransferCrossChainAsset:
		assetInfo = &service.TransferCrossChainAssetInfo{}
	case id.RegisterIdentification:
		assetInfo = &RegisterIdentificationInfo{}
	default:
		return nil, errors.New("GetBlockTransactions: Unknown payload type")
	}
	err = service.Unmarshal(&txInfo.Payload, assetInfo)
	if err == nil {
		txInfo.Payload = assetInfo
	}

	return &txInfo, nil
}

func GetTransactionInfo(cfg *service.Config, header interfaces.Header, tx *types.Transaction) *service.TransactionInfo {
	inputs := make([]service.InputInfo, len(tx.Inputs))
	for i, v := range tx.Inputs {
		inputs[i].TxID = service.ToReversedString(v.Previous.TxID)
		inputs[i].VOut = v.Previous.Index
		inputs[i].Sequence = v.Sequence
	}

	outputs := make([]service.OutputInfo, len(tx.Outputs))
	for i, v := range tx.Outputs {
		outputs[i].Value = v.Value.String()
		outputs[i].Index = uint32(i)
		address, _ := v.ProgramHash.ToAddress()
		outputs[i].Address = address
		outputs[i].AssetID = service.ToReversedString(v.AssetID)
		outputs[i].OutputLock = v.OutputLock
	}

	attributes := make([]service.AttributeInfo, len(tx.Attributes))
	for i, v := range tx.Attributes {
		attributes[i].Usage = v.Usage
		attributes[i].Data = common.BytesToHexString(v.Data)
	}

	programs := make([]service.ProgramInfo, len(tx.Programs))
	for i, v := range tx.Programs {
		programs[i].Code = common.BytesToHexString(v.Code)
		programs[i].Parameter = common.BytesToHexString(v.Parameter)
	}

	var txHash = tx.Hash()
	var txHashStr = service.ToReversedString(txHash)
	var size = uint32(tx.GetSize())
	var blockHash string
	var confirmations uint32
	var time uint32
	var blockTime uint32
	if header != nil {
		confirmations = cfg.Chain.GetBestHeight() - header.GetHeight() + 1
		blockHash = service.ToReversedString(header.Hash())
		time = header.GetTimeStamp()
		blockTime = header.GetTimeStamp()
	}

	return &service.TransactionInfo{
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

func GetPayloadInfo(p types.Payload, pVersion byte) service.PayloadInfo {
	switch object := p.(type) {
	case *types.PayloadCoinBase:
		obj := new(service.CoinbaseInfo)
		obj.CoinbaseData = string(object.CoinbaseData)
		return obj
	case *types.PayloadRegisterAsset:
		obj := new(service.RegisterAssetInfo)
		obj.Asset = object.Asset
		obj.Amount = object.Amount.String()
		obj.Controller = common.BytesToHexString(common.BytesReverse(object.Controller.Bytes()))
		return obj
	case *types.PayloadTransferCrossChainAsset:
		obj := new(service.TransferCrossChainAssetInfo)
		obj.CrossChainAssets = make([]service.CrossChainAssetInfo, 0)
		for i := 0; i < len(object.CrossChainAddresses); i++ {
			assetInfo := service.CrossChainAssetInfo{
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
			obj := new(service.RechargeToSideChainInfoV0)
			obj.MainChainTransaction = common.BytesToHexString(object.MainChainTransaction)
			obj.Proof = common.BytesToHexString(object.MerkleProof)
			return obj
		} else if pVersion == types.RechargeToSideChainPayloadVersion1 {
			obj := new(service.RechargeToSideChainInfoV1)
			obj.MainChainTransactionHash = service.ToReversedString(object.MainChainTransactionHash)
			return obj
		}
	case *id.PayloadRegisterIdentification:
		obj := new(RegisterIdentificationInfo)
		obj.Id = object.ID
		obj.Sign = common.BytesToHexString(object.Sign)
		contents := []RegisterIdentificationContentInfo{}
		for _, content := range object.Contents {
			values := []RegisterIdentificationValueInfo{}
			for _, value := range content.Values {
				values = append(values, RegisterIdentificationValueInfo{
					DataHash: service.ToReversedString(value.DataHash),
					Proof:    value.Proof,
					Info:     value.Info,
				})
			}

			contents = append(contents, RegisterIdentificationContentInfo{
				Path:   content.Path,
				Values: values,
			})
		}
		obj.Contents = contents
		return obj
	case *id.Operation:
		operation := new(RpcOperation)
		operation.Header = object.Header
		operation.Payload = object.Payload
		operation.Proof = object.Proof
		return operation
	}
	return nil
}
