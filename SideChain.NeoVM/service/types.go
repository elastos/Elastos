package service

import (
	"github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA.SideChain/service"

	"github.com/elastos/Elastos.ELA.Utility/common"

	nt "github.com/elastos/Elastos.ELA.SideChain.NeoVM/types"
)

type DeployInfo struct {
	Code        nt.FunctionCode
	Name        string
	CodeVersion string
	Author      string
	Email       string
	Description string
	ProgramHash string
	Gas         string
}

type InvokeInfo struct {
	CodeHash    string
	Code        string
	ProgramHash string
	Gas         string
}

type UTXOInfo struct {
	AssetId       string `json:"assetid"`
	Txid          string `json:"txid"`
	VOut          uint32 `json:"vout"`
	Address       string `json:"address"`
	Amount        string `json:"amount"`
	Confirmations uint32 `json:"confirmations"`
}

type HeadInfo struct {
	Version          uint32
	PrevBlockHash    string
	TransactionsRoot string
	Timestamp        uint32
	Height           uint32
	Nonce            uint32
	Hash             string
}

type BlockInfo struct {
	Version          uint32
	PrevBlockHash    string
	MerkleRoot string
	Timestamp        uint32
	Height           uint32
	Nonce            uint32
	Hash             string
	tx               []string
}

func GetHeaderInfo(header *types.Header) *HeadInfo {
	h := header.Hash()
	return &HeadInfo{
		Version:          header.Version,
		PrevBlockHash:    common.BytesToHexString(common.BytesReverse(header.Previous.Bytes())),
		TransactionsRoot: common.BytesToHexString(common.BytesReverse(header.MerkleRoot.Bytes())),
		Timestamp:        header.Timestamp,
		Height:           header.Height,
		Nonce:            header.Nonce,
		Hash:             common.BytesToHexString(common.BytesReverse(h.Bytes())),
	}
}

func GetBlockInfo(block *types.Block) *BlockInfo {
	var txs []string
	for _, tx := range block.Transactions {
		txs = append(txs, tx.String())
	}

	return &BlockInfo{
		Version:          block.Version,
		PrevBlockHash:    common.BytesToHexString(common.BytesReverse(block.Previous.Bytes())),
		MerkleRoot: common.BytesToHexString(common.BytesReverse(block.MerkleRoot.Bytes())),
		Timestamp:        block.Timestamp,
		Height:           block.Height,
		Nonce:            block.Nonce,
		Hash:             common.BytesToHexString(common.BytesReverse(block.Hash().Bytes())),
		tx:               txs,
	}
}

func GetTXInfo(tx *types.Transaction) *service.TransactionInfo {

	var txHashStr = common.BytesToHexString(common.BytesReverse(tx.Hash().Bytes()))
	var size = uint32(tx.GetSize())

	inputs := make([]service.InputInfo, len(tx.Inputs))
	for i, v := range tx.Inputs {
		inputs[i].TxID = common.BytesToHexString(common.BytesReverse(v.Previous.TxID.Bytes()))
		inputs[i].VOut = v.Previous.Index
		inputs[i].Sequence = v.Sequence
	}

	outputs := make([]service.OutputInfo, len(tx.Outputs))
	for i, v := range tx.Outputs {
		outputs[i].Value = v.Value.String()
		outputs[i].Index = uint32(i)
		address, err := v.ProgramHash.ToAddress()
		if err != nil {
			return nil
		}
		outputs[i].Address = address
		outputs[i].AssetID = common.BytesToHexString(common.BytesReverse(v.AssetID.Bytes()))
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

	return &service.TransactionInfo{
		TxId:           txHashStr,
		Hash:           txHashStr,
		Size:           size,
		VSize:          size,
		Version:        0x00,
		LockTime:       tx.LockTime,
		Inputs:         inputs,
		Outputs:        outputs,
		BlockHash:      "",
		Confirmations:  0,
		Time:           0,
		BlockTime:      0,
		TxType:         tx.TxType,
		PayloadVersion: tx.PayloadVersion,
		Payload:        common.BytesToHexString(tx.Payload.Data(0)),
		Attributes:     attributes,
		Programs:       programs,
	}
}

func GetAssetInfo(asset *types.Asset) *service.AssetInfo {
	return &service.AssetInfo{
		ID: common.BytesToHexString(asset.Hash().Bytes()),
		Name: asset.Name,
		Description: asset.Description,
		Precision: int(asset.Precision),
		AssetType: int(asset.AssetType),
		RecordType: int(asset.RecordType),
	}
}