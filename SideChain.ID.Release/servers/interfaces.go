package servers

import (
	"bytes"
	"encoding/json"
	"errors"

	bc "github.com/elastos/Elastos.ELA.SideChain.ID/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain.ID/core"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	ucore "github.com/elastos/Elastos.ELA.SideChain/core"
	uerr "github.com/elastos/Elastos.ELA.SideChain/errors"
	"github.com/elastos/Elastos.ELA.SideChain/servers"
	. "github.com/elastos/Elastos.ELA.Utility/common"
)

func InitHttpServers() {
	servers.HttpServers = &servers.HttpServersBase{}
	servers.HttpServers.Init()
	servers.HttpServers.GetPayloadInfo = GetPayloadInfo
	servers.HttpServers.GetTransactionInfoFromBytes = GetTransactionInfoFromBytes
}

func GetIdentificationTxByIdAndPath(param servers.Params) map[string]interface{} {
	id, ok := param.String("id")
	if !ok {
		return servers.ResponsePack(uerr.InvalidParams, "id is null")
	}
	_, err := Uint168FromAddress(id)
	if err != nil {
		return servers.ResponsePack(uerr.InvalidParams, "invalid id")
	}
	path, ok := param.String("path")
	if !ok {
		return servers.ResponsePack(uerr.InvalidParams, "path is null")
	}

	buf := new(bytes.Buffer)
	buf.WriteString(id)
	buf.WriteString(path)
	txHashBytes, err := blockchain.DefaultLedger.Store.(*bc.IDChainStore).GetRegisterIdentificationTx(buf.Bytes())
	if err != nil {
		return servers.ResponsePack(uerr.UnknownTransaction, "get identification transaction failed")
	}
	txHash, err := Uint256FromBytes(txHashBytes)
	if err != nil {
		return servers.ResponsePack(uerr.InvalidTransaction, "invalid transaction hash")
	}

	txn, height, err := blockchain.DefaultLedger.Store.GetTransaction(*txHash)
	if err != nil {
		return servers.ResponsePack(uerr.UnknownTransaction, "get transaction failed")
	}
	bHash, err := blockchain.DefaultLedger.Store.GetBlockHash(height)
	if err != nil {
		return servers.ResponsePack(uerr.UnknownBlock, "get block failed")
	}
	header, err := blockchain.DefaultLedger.Store.GetHeader(bHash)
	if err != nil {
		return servers.ResponsePack(uerr.UnknownBlock, "get header failed")
	}

	return servers.ResponsePack(uerr.Success, servers.HttpServers.GetTransactionInfo(header, txn))
}

func GetPayloadInfo(p ucore.Payload) servers.PayloadInfo {
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

func GetTransactionInfoFromBytes(txInfoBytes []byte) (*servers.TransactionInfo, error) {
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
