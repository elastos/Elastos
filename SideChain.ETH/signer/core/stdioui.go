// Copyright 2018 The Elastos.ELA.SideChain.ETH Authors
// This file is part of Elastos.ELA.SideChain.ETH.
//
// Elastos.ELA.SideChain.ETH is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Elastos.ELA.SideChain.ETH is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Elastos.ELA.SideChain.ETH. If not, see <http://www.gnu.org/licenses/>.
//

package core

import (
	"context"
	"sync"

	"github.com/elastos/Elastos.ELA.SideChain.ETH/internal/ethapi"
	"github.com/elastos/Elastos.ELA.SideChain.ETH/log"
	"github.com/elastos/Elastos.ELA.SideChain.ETH/rpc"
)

type StdIOUI struct {
	client rpc.Client
	mu     sync.Mutex
}

func NewStdIOUI() *StdIOUI {
	log.Info("NewStdIOUI")
	client, err := rpc.DialContext(context.Background(), "stdio://")
	if err != nil {
		log.Crit("Could not create stdio client", "err", err)
	}
	return &StdIOUI{client: *client}
}

// dispatch sends a request over the stdio
func (ui *StdIOUI) dispatch(serviceMethod string, args interface{}, reply interface{}) error {
	err := ui.client.Call(&reply, serviceMethod, args)
	if err != nil {
		log.Info("Error", "exc", err.Error())
	}
	return err
}

func (ui *StdIOUI) ApproveTx(request *SignTxRequest) (SignTxResponse, error) {
	var result SignTxResponse
	err := ui.dispatch("ApproveTx", request, &result)
	return result, err
}

func (ui *StdIOUI) ApproveSignData(request *SignDataRequest) (SignDataResponse, error) {
	var result SignDataResponse
	err := ui.dispatch("ApproveSignData", request, &result)
	return result, err
}

func (ui *StdIOUI) ApproveExport(request *ExportRequest) (ExportResponse, error) {
	var result ExportResponse
	err := ui.dispatch("ApproveExport", request, &result)
	return result, err
}

func (ui *StdIOUI) ApproveImport(request *ImportRequest) (ImportResponse, error) {
	var result ImportResponse
	err := ui.dispatch("ApproveImport", request, &result)
	return result, err
}

func (ui *StdIOUI) ApproveListing(request *ListRequest) (ListResponse, error) {
	var result ListResponse
	err := ui.dispatch("ApproveListing", request, &result)
	return result, err
}

func (ui *StdIOUI) ApproveNewAccount(request *NewAccountRequest) (NewAccountResponse, error) {
	var result NewAccountResponse
	err := ui.dispatch("ApproveNewAccount", request, &result)
	return result, err
}

func (ui *StdIOUI) ShowError(message string) {
	err := ui.dispatch("ShowError", &Message{message}, nil)
	if err != nil {
		log.Info("Error calling 'ShowError'", "exc", err.Error(), "msg", message)
	}
}

func (ui *StdIOUI) ShowInfo(message string) {
	err := ui.dispatch("ShowInfo", Message{message}, nil)
	if err != nil {
		log.Info("Error calling 'ShowInfo'", "exc", err.Error(), "msg", message)
	}
}
func (ui *StdIOUI) OnApprovedTx(tx ethapi.SignTransactionResult) {
	err := ui.dispatch("OnApprovedTx", tx, nil)
	if err != nil {
		log.Info("Error calling 'OnApprovedTx'", "exc", err.Error(), "tx", tx)
	}
}

func (ui *StdIOUI) OnSignerStartup(info StartupInfo) {
	err := ui.dispatch("OnSignerStartup", info, nil)
	if err != nil {
		log.Info("Error calling 'OnSignerStartup'", "exc", err.Error(), "info", info)
	}
}
func (ui *StdIOUI) OnInputRequired(info UserInputRequest) (UserInputResponse, error) {
	var result UserInputResponse
	err := ui.dispatch("OnInputRequired", info, &result)
	if err != nil {
		log.Info("Error calling 'OnInputRequired'", "exc", err.Error(), "info", info)
	}
	return result, err
}
