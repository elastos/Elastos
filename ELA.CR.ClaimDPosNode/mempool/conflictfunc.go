// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package mempool

import (
	"encoding/hex"
	"fmt"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/vm"
)

// hashes related functions
func hashCRCProposalDraftHash(tx *types.Transaction) (interface{}, error) {
	p, ok := tx.Payload.(*payload.CRCProposal)
	if !ok {
		return nil, fmt.Errorf(
			"CRC proposal payload cast failed, tx:%s", tx.Hash())
	}
	return p.DraftHash, nil
}

func hashCRCProposalDID(tx *types.Transaction) (interface{}, error) {
	p, ok := tx.Payload.(*payload.CRCProposal)
	if !ok {
		return nil, fmt.Errorf(
			"CRC proposal payload cast failed, tx:%s", tx.Hash())
	}
	return p.CRCouncilMemberDID, nil
}

func hashCRCProposalWithdrawProposalHash(
	tx *types.Transaction) (interface{}, error) {
	p, ok := tx.Payload.(*payload.CRCProposalWithdraw)
	if !ok {
		return nil, fmt.Errorf(
			"crcProposalWithdraw  payload cast failed, tx:%s", tx.Hash())
	}
	return p.ProposalHash, nil
}

func hashCRCProposalTrackingProposalHash(
	tx *types.Transaction) (interface{}, error) {
	p, ok := tx.Payload.(*payload.CRCProposalTracking)
	if !ok {
		return nil, fmt.Errorf(
			"crcProposalTracking  payload cast failed, tx:%s", tx.Hash())
	}
	return p.ProposalHash, nil
}

func hashSpecialTxHash(tx *types.Transaction) (interface{}, error) {
	illegalData, ok := tx.Payload.(payload.DPOSIllegalData)
	if !ok {
		return nil, fmt.Errorf(
			"special tx payload cast failed, tx:%s", tx.Hash())
	}
	return illegalData.Hash(), nil
}

func hashNextTurnDPOSInfoTxPayloadHash(tx *types.Transaction) (interface{}, error) {
	payload, ok := tx.Payload.(*payload.NextTurnDPOSInfo)
	if !ok {
		return nil, fmt.Errorf(
			"NextTurnDPOSInfo tx payload cast failed, tx:%s", tx.Hash())
	}
	return payload.Hash(), nil
}

// strings related functions
func strCancelProducerOwnerPublicKey(tx *types.Transaction) (interface{},
	error) {
	p, ok := tx.Payload.(*payload.ProcessProducer)
	if !ok {
		err := fmt.Errorf(
			"cancel producer payload cast failed, tx:%s", tx.Hash())
		return nil, errors.Simple(errors.ErrTxPoolFailure, err)
	}
	return common.BytesToHexString(p.OwnerPublicKey), nil
}

func strProducerInfoOwnerPublicKey(tx *types.Transaction) (interface{}, error) {
	p, err := comGetProducerInfo(tx)
	if err != nil {
		return nil, err
	}
	return common.BytesToHexString(p.OwnerPublicKey), nil
}

func strProducerInfoNodePublicKey(tx *types.Transaction) (interface{}, error) {
	p, err := comGetProducerInfo(tx)
	if err != nil {
		return nil, err
	}
	return common.BytesToHexString(p.NodePublicKey), nil
}

func strCRManagementPublicKey(tx *types.Transaction) (interface{}, error) {
	p, ok := tx.Payload.(*payload.CRDPOSManagement)
	if !ok {
		return nil, fmt.Errorf(
			"cr dpos management payload cast failed, tx:%s", tx.Hash())
	}
	return common.BytesToHexString(p.CRManagementPublicKey), nil
}

func strCRManagementDID(tx *types.Transaction) (interface{}, error) {
	p, ok := tx.Payload.(*payload.CRDPOSManagement)
	if !ok {
		return nil, fmt.Errorf(
			"cr dpos management payload cast failed, tx:%s", tx.Hash())
	}
	return p.CRCommitteeDID, nil
}

func strProducerInfoNickname(tx *types.Transaction) (interface{}, error) {
	p, err := comGetProducerInfo(tx)
	if err != nil {
		return nil, err
	}
	return p.NickName, nil
}

func strActivateProducerNodePublicKey(
	tx *types.Transaction) (interface{}, error) {
	p, ok := tx.Payload.(*payload.ActivateProducer)
	if !ok {
		return nil, fmt.Errorf(
			"activate producer payload cast failed, tx:%s", tx.Hash())
	}
	return common.BytesToHexString(p.NodePublicKey), nil
}

func strRegisterCRPublicKey(tx *types.Transaction) (interface{}, error) {
	p, err := comGetCRInfo(tx)
	if err != nil {
		return nil, err
	}

	signType, err := crypto.GetScriptType(p.Code)
	if err != nil {
		return nil, err
	}

	if signType == vm.CHECKSIG {
		return hex.EncodeToString(p.Code[1 : len(p.Code)-1]), nil
	} else {
		return nil, fmt.Errorf("unsupported sign script type: %d", signType)
	}
}

func strCRInfoNickname(tx *types.Transaction) (interface{}, error) {
	p, err := comGetCRInfo(tx)
	if err != nil {
		return nil, err
	}
	return p.NickName, nil
}

func strTxProgramCode(tx *types.Transaction) (interface{}, error) {
	return common.BytesToHexString(tx.Programs[0].Code), nil
}

func strProposalReviewKey(tx *types.Transaction) (interface{}, error) {
	p, ok := tx.Payload.(*payload.CRCProposalReview)
	if !ok {
		return nil, fmt.Errorf(
			"crcProposalReview  payload cast failed, tx:%s",
			tx.Hash())
	}
	return p.DID.String() + p.ProposalHash.String(), nil
}

func strCRCAppropriation(*types.Transaction) (interface{}, error) {
	// const string to ensure only one tx added to the tx pool
	return "CRC Appropriation", nil
}

func hashArrayCRCProposalRealWithdrawTransactionHashes(
	tx *types.Transaction) (interface{}, error) {
	p, ok := tx.Payload.(*payload.CRCProposalRealWithdraw)
	if !ok {
		return nil, fmt.Errorf(
			"real proposal withdraw transaction cast failed, tx: %s",
			tx.Hash())
	}

	return p.WithdrawTransactionHashes, nil
}

// program hashes related functions
func addrCRInfoCRCID(tx *types.Transaction) (interface{}, error) {
	p, err := comGetCRInfo(tx)
	if err != nil {
		return nil, err
	}
	return p.CID, nil
}

func addrUnregisterCRCID(tx *types.Transaction) (interface{}, error) {
	p, ok := tx.Payload.(*payload.UnregisterCR)
	if !ok {
		return nil, fmt.Errorf(
			"unregisterCR CR payload cast failed, tx: %s", tx.Hash())
	}
	return p.CID, nil
}

// hash array related functions
func hashArraySidechainTransactionHashes(
	tx *types.Transaction) (interface{}, error) {
	p, ok := tx.Payload.(*payload.WithdrawFromSideChain)
	if !ok {
		return nil, fmt.Errorf(
			"withdraw from sidechain payload cast failed, tx: %s",
			tx.Hash())
	}

	array := make([]common.Uint256, 0, len(p.SideChainTransactionHashes))
	for _, v := range p.SideChainTransactionHashes {
		array = append(array, v)
	}
	return array, nil
}

// str array related functions
func strArrayTxReferences(tx *types.Transaction) (interface{}, error) {
	reference, err :=
		blockchain.DefaultLedger.Blockchain.UTXOCache.GetTxReference(tx)
	if err != nil {
		return nil, err
	}

	result := make([]string, 0, len(reference))
	for k := range reference {
		result = append(result, k.ReferKey())
	}
	return result, nil
}

// common functions

func comGetProducerInfo(tx *types.Transaction) (*payload.ProducerInfo, error) {
	p, ok := tx.Payload.(*payload.ProducerInfo)
	if !ok {
		return nil, fmt.Errorf(
			"register producer payload cast failed, tx:%s", tx.Hash())
	}
	return p, nil
}

func comGetCRInfo(tx *types.Transaction) (*payload.CRInfo, error) {
	p, ok := tx.Payload.(*payload.CRInfo)
	if !ok {
		return nil, fmt.Errorf(
			"register CR payload cast failed, tx:%s", tx.Hash())
	}
	return p, nil
}
