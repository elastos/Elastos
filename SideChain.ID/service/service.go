package service

import (
	"bytes"
	"encoding/json"
	"errors"

	"github.com/elastos/Elastos.ELA.SideChain.ID/blockchain"
	id "github.com/elastos/Elastos.ELA.SideChain.ID/types"

	"github.com/elastos/Elastos.ELA.SideChain/service"
	"github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/http/util"
)

type HttpServiceExtend struct {
	*service.HttpService

	Config *service.Config
	store  *blockchain.IDChainStore
}

func NewHttpService(cfg *service.Config, store *blockchain.IDChainStore) *HttpServiceExtend {
	server := &HttpServiceExtend{
		HttpService: service.NewHttpService(cfg),
		store:       store,
		Config:      cfg,
	}
	return server
}

func (s *HttpServiceExtend) GetIdentificationTxByIdAndPath(param util.Params) (interface{}, error) {
	id, ok := param.String("id")
	if !ok {
		return nil, util.NewError(int(service.InvalidParams), "id is null")
	}
	_, err := common.Uint168FromAddress(id)
	if err != nil {
		return nil, util.NewError(int(service.InvalidParams), "invalid id")
	}
	path, ok := param.String("path")
	if !ok {
		return nil, util.NewError(int(service.InvalidParams), "path is null")
	}

	buf := new(bytes.Buffer)
	buf.WriteString(id)
	buf.WriteString(path)
	txHashBytes, err := s.store.GetRegisterIdentificationTx(buf.Bytes())
	if err != nil {
		return nil, util.NewError(int(service.UnknownTransaction), "get identification transaction failed")
	}
	txHash, err := common.Uint256FromBytes(txHashBytes)
	if err != nil {
		return nil, util.NewError(int(service.InvalidTransaction), "invalid transaction hash")
	}

	txn, height, err := s.store.GetTransaction(*txHash)
	if err != nil {
		return nil, util.NewError(int(service.UnknownTransaction), "get transaction failed")
	}
	bHash, err := s.store.GetBlockHash(height)
	if err != nil {
		return nil, util.NewError(int(service.UnknownBlock), "get block failed")
	}
	header, err := s.store.GetHeader(bHash)
	if err != nil {
		return nil, util.NewError(int(service.UnknownBlock), "get header failed")
	}

	return s.Config.GetTransactionInfo(s.Config, header, txn), nil
}

func (s *HttpServiceExtend) ListUnspent(param util.Params) (interface{}, error) {
	bestHeight := s.Config.Store.GetHeight()
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
		differentAssets, err := s.Config.Chain.GetUnspents(*programHash)
		if err != nil {
			return nil, errors.New("cannot get asset with program")
		}
		for _, asset := range differentAssets {
			for _, unspent := range asset {
				tx, height, err := s.Config.Chain.GetTransaction(unspent.TxId)
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

func GetTransactionInfo(cfg *service.Config, header *types.Header, tx *types.Transaction) *service.TransactionInfo {
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
		confirmations = cfg.Chain.GetBestHeight() - header.Height + 1
		blockHash = service.ToReversedString(header.Hash())
		time = header.Timestamp
		blockTime = header.Timestamp
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
	}
	return nil
}
