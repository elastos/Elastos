package servers

import (
	"bytes"
	"encoding/json"
	"errors"

	bc "github.com/elastos/Elastos.ELA.SideChain.ID/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain.ID/core"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	ucore "github.com/elastos/Elastos.ELA.SideChain/core"
	"github.com/elastos/Elastos.ELA.SideChain/servers"
	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type HttpServiceExtend struct {
	*servers.HttpService
	chain  blockchain.IChainStore
}

func NewHttpService(cfg *servers.Config, chain blockchain.IChainStore) *HttpServiceExtend {
	server := &HttpServiceExtend{
		HttpService: servers.NewHttpService(cfg),
		chain: chain,
	}
	server.GetTransactionInfo = server.getTransactionInfoImpl
	return server
}

func (s *HttpServiceExtend) GetIdentificationTxByIdAndPath(param servers.Params) map[string]interface{} {
	id, ok := param.String("id")
	if !ok {
		return servers.ResponsePack(servers.InvalidParams, "id is null")
	}
	_, err := Uint168FromAddress(id)
	if err != nil {
		return servers.ResponsePack(servers.InvalidParams, "invalid id")
	}
	path, ok := param.String("path")
	if !ok {
		return servers.ResponsePack(servers.InvalidParams, "path is null")
	}

	buf := new(bytes.Buffer)
	buf.WriteString(id)
	buf.WriteString(path)
	txHashBytes, err := s.chain.(*bc.IDChainStore).GetRegisterIdentificationTx(buf.Bytes())
	if err != nil {
		return servers.ResponsePack(servers.UnknownTransaction, "get identification transaction failed")
	}
	txHash, err := Uint256FromBytes(txHashBytes)
	if err != nil {
		return servers.ResponsePack(servers.InvalidTransaction, "invalid transaction hash")
	}

	txn, height, err := s.chain.GetTransaction(*txHash)
	if err != nil {
		return servers.ResponsePack(servers.UnknownTransaction, "get transaction failed")
	}
	bHash, err := s.chain.GetBlockHash(height)
	if err != nil {
		return servers.ResponsePack(servers.UnknownBlock, "get block failed")
	}
	header, err := s.chain.GetHeader(bHash)
	if err != nil {
		return servers.ResponsePack(servers.UnknownBlock, "get header failed")
	}

	return servers.ResponsePack(servers.Success, s.GetTransactionInfo(header, txn))
}

func (s *HttpServiceExtend) getTransactionInfoImpl(header *ucore.Header, tx *ucore.Transaction) *servers.TransactionInfo {
	inputs := make([]servers.InputInfo, len(tx.Inputs))
	for i, v := range tx.Inputs {
		inputs[i].TxID = servers.ToReversedString(v.Previous.TxID)
		inputs[i].VOut = v.Previous.Index
		inputs[i].Sequence = v.Sequence
	}

	outputs := make([]servers.OutputInfo, len(tx.Outputs))
	for i, v := range tx.Outputs {
		outputs[i].Value = v.Value.String()
		outputs[i].Index = uint32(i)
		var address string
		destroyHash := Uint168{}
		if v.ProgramHash == destroyHash {
			address = servers.DestroyAddress
		} else {
			address, _ = v.ProgramHash.ToAddress()
		}
		outputs[i].Address = address
		outputs[i].AssetID = servers.ToReversedString(v.AssetID)
		outputs[i].OutputLock = v.OutputLock
	}

	attributes := make([]servers.AttributeInfo, len(tx.Attributes))
	for i, v := range tx.Attributes {
		attributes[i].Usage = v.Usage
		attributes[i].Data = BytesToHexString(v.Data)
	}

	programs := make([]servers.ProgramInfo, len(tx.Programs))
	for i, v := range tx.Programs {
		programs[i].Code = BytesToHexString(v.Code)
		programs[i].Parameter = BytesToHexString(v.Parameter)
	}

	var txHash = tx.Hash()
	var txHashStr = servers.ToReversedString(txHash)
	var size = uint32(tx.GetSize())
	var blockHash string
	var confirmations uint32
	var time uint32
	var blockTime uint32
	if header != nil {
		confirmations = s.chain.GetHeight() - header.Height + 1
		blockHash = servers.ToReversedString(header.Hash())
		time = header.Timestamp
		blockTime = header.Timestamp
	}

	return &servers.TransactionInfo{
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

func (s *HttpServiceExtend) getPayloadInfo(p ucore.Payload) servers.PayloadInfo {
	switch object := p.(type) {
	case *ucore.PayloadCoinBase:
		obj := new(servers.CoinbaseInfo)
		obj.CoinbaseData = string(object.CoinbaseData)
		return obj
	case *ucore.PayloadRegisterAsset:
		obj := new(servers.RegisterAssetInfo)
		obj.Asset = object.Asset
		obj.Amount = object.Amount.String()
		obj.Controller = BytesToHexString(BytesReverse(object.Controller.Bytes()))
		return obj
	case *ucore.PayloadTransferCrossChainAsset:
		obj := new(servers.TransferCrossChainAssetInfo)
		obj.CrossChainAddresses = object.CrossChainAddresses
		obj.OutputIndexes = object.OutputIndexes
		obj.CrossChainAmounts = object.CrossChainAmounts
		return obj
	case *ucore.PayloadTransferAsset:
	case *ucore.PayloadRecord:
	case *ucore.PayloadRechargeToSideChain:
		obj := new(servers.RechargeToSideChainInfo)
		obj.MainChainTransaction = BytesToHexString(object.MainChainTransaction)
		obj.Proof = BytesToHexString(object.MerkleProof)
		return obj
	case *core.PayloadRegisterIdentification:
		obj := new(RegisterIdentificationInfo)
		obj.Id = object.ID
		obj.Sign = BytesToHexString(object.Sign)
		contents := []RegisterIdentificationContentInfo{}
		for _, content := range object.Contents {
			values := []RegisterIdentificationValueInfo{}
			for _, value := range content.Values {
				values = append(values, RegisterIdentificationValueInfo{
					DataHash: servers.ToReversedString(value.DataHash),
					Proof:    value.Proof,
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

func (s *HttpServiceExtend) GetTransactionInfoFromBytes(txInfoBytes []byte) (*servers.TransactionInfo, error) {
	var txInfo servers.TransactionInfo
	err := json.Unmarshal(txInfoBytes, &txInfo)
	if err != nil {
		return nil, errors.New("InvalidParameter")
	}

	var assetInfo servers.PayloadInfo
	switch txInfo.TxType {
	case ucore.CoinBase:
		assetInfo = &servers.CoinbaseInfo{}
	case ucore.RegisterAsset:
		assetInfo = &servers.RegisterAssetInfo{}
	case ucore.SideChainPow:
		assetInfo = &servers.SideChainPowInfo{}
	case ucore.RechargeToSideChain:
		assetInfo = &servers.RechargeToSideChainInfo{}
	case ucore.TransferCrossChainAsset:
		assetInfo = &servers.TransferCrossChainAssetInfo{}
	case core.RegisterIdentification:
		assetInfo = &RegisterIdentificationInfo{}
	default:
		return nil, errors.New("GetBlockTransactions: Unknown payload type")
	}
	err = servers.Unmarshal(&txInfo.Payload, assetInfo)
	if err == nil {
		txInfo.Payload = assetInfo
	}

	return &txInfo, nil
}
