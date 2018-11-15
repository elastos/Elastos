package service

import (
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/contract"
)

type DeployInfo struct {
	Code        contract.FunctionCode
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