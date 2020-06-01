// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package mempool

import (
	"fmt"

	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/errors"
)

const (
	slotDPoSOwnerPublicKey         = "DPoSOwnerPublicKey"
	slotDPoSNodePublicKey          = "DPoSNodePublicKey"
	slotDPoSNickname               = "DPoSNickname"
	slotCRDID                      = "CrDID"
	slotCRNickname                 = "CrNickname"
	slotProgramCode                = "ProgramCode"
	slotCRCProposalDraftHash       = "CRCProposalDraftHash"
	slotCRCProposalDID             = "CRCProposalDID"
	slotCRCProposalHash            = "CRCProposalHash"
	slotCRCProposalTrackingHash    = "CRCProposalTrackingHash"
	slotCRCProposalReviewKey       = "CRCProposalReviewKey"
	slotCRCAppropriationKey        = "CRCAppropriationKey"
	slotCRCProposalRealWithdrawKey = "SlotCRCProposalRealWithdrawKey"
	slotSpecialTxHash              = "SpecialTxHash"
	slotSidechainTxHashes          = "SidechainTxHashes"
	slotTxInputsReferKeys          = "TxInputsReferKeys"
)

type conflict struct {
	name string
	slot *conflictSlot
}

// conflictManager hold a set of conflict slots, and refer some query methods.
type conflictManager struct {
	conflictSlots []*conflict
}

func (m *conflictManager) VerifyTx(tx *types.Transaction) errors.ELAError {
	for _, v := range m.conflictSlots {
		if err := v.slot.VerifyTx(tx); err != nil {
			return errors.SimpleWithMessage(errors.ErrTxPoolFailure, err,
				fmt.Sprintf("slot %s verify tx error", v.name))
		}
	}
	return nil
}

func (m *conflictManager) AppendTx(tx *types.Transaction) errors.ELAError {
	for _, v := range m.conflictSlots {
		if err := v.slot.AppendTx(tx); err != nil {
			return errors.SimpleWithMessage(errors.ErrTxPoolFailure, err,
				fmt.Sprintf("slot %s append tx error", v.name))
		}
	}
	return nil
}

func (m *conflictManager) removeTx(tx *types.Transaction) errors.ELAError {
	for _, v := range m.conflictSlots {
		if err := v.slot.RemoveTx(tx); err != nil {
			return errors.SimpleWithMessage(errors.ErrTxPoolFailure, err,
				fmt.Sprintf("slot %s remove tx error", v.name))
		}
	}
	return nil
}

func (m *conflictManager) GetTx(key interface{},
	slotName string) *types.Transaction {
	for _, v := range m.conflictSlots {
		if v.name == slotName {
			return v.slot.GetTx(key)
		}
	}
	return nil
}

func (m *conflictManager) ContainsKey(key interface{}, slotName string) bool {
	for _, v := range m.conflictSlots {
		if v.name == slotName {
			return v.slot.Contains(key)
		}
	}
	return false
}

func (m *conflictManager) RemoveKey(key interface{},
	slotName string) errors.ELAError {
	for _, v := range m.conflictSlots {
		if v.name == slotName {
			return v.slot.removeKey(key)
		}
	}
	return errors.SimpleWithMessage(errors.ErrTxPoolFailure, nil,
		fmt.Sprintf("slot %s not exist", slotName))
}

func (m *conflictManager) Empty() bool {
	for _, v := range m.conflictSlots {
		if !v.slot.Empty() {
			return false
		}
	}
	return true
}

func newConflictManager() conflictManager {
	return conflictManager{
		conflictSlots: []*conflict{
			// DPoS owner public key
			{
				name: slotDPoSOwnerPublicKey,
				slot: newConflictSlot(str,
					keyTypeFuncPair{
						Type: types.RegisterProducer,
						Func: strProducerInfoOwnerPublicKey,
					},
					keyTypeFuncPair{
						Type: types.UpdateProducer,
						Func: strProducerInfoOwnerPublicKey,
					},
					keyTypeFuncPair{
						Type: types.CancelProducer,
						Func: strCancelProducerOwnerPublicKey,
					},
					keyTypeFuncPair{
						Type: types.RegisterCR,
						Func: strRegisterCRPublicKey,
					},
				),
			},
			// DPoS node public key
			{
				name: slotDPoSNodePublicKey,
				slot: newConflictSlot(str,
					keyTypeFuncPair{
						Type: types.RegisterProducer,
						Func: strProducerInfoNodePublicKey,
					},
					keyTypeFuncPair{
						Type: types.UpdateProducer,
						Func: strProducerInfoNodePublicKey,
					},
					keyTypeFuncPair{
						Type: types.ActivateProducer,
						Func: strActivateProducerNodePublicKey,
					},
					keyTypeFuncPair{
						Type: types.RegisterCR,
						Func: strRegisterCRPublicKey,
					},
				),
			},
			// DPoS nickname
			{
				name: slotDPoSNickname,
				slot: newConflictSlot(str,
					keyTypeFuncPair{
						Type: types.RegisterProducer,
						Func: strProducerInfoNickname,
					},
					keyTypeFuncPair{
						Type: types.UpdateProducer,
						Func: strProducerInfoNickname,
					},
				),
			},
			// CR CID
			{
				name: slotCRDID,
				slot: newConflictSlot(programHash,
					keyTypeFuncPair{
						Type: types.RegisterCR,
						Func: addrCRInfoCRCID,
					},
					keyTypeFuncPair{
						Type: types.UpdateCR,
						Func: addrCRInfoCRCID,
					},
					keyTypeFuncPair{
						Type: types.UnregisterCR,
						Func: addrUnregisterCRCID,
					},
				),
			},
			// CR nickname
			{
				name: slotCRNickname,
				slot: newConflictSlot(str,
					keyTypeFuncPair{
						Type: types.RegisterCR,
						Func: strCRInfoNickname,
					},
					keyTypeFuncPair{
						Type: types.UpdateCR,
						Func: strCRInfoNickname,
					},
				),
			},
			// CR and DPoS program code
			{
				name: slotProgramCode,
				slot: newConflictSlot(str,
					keyTypeFuncPair{
						Type: types.ReturnDepositCoin,
						Func: strTxProgramCode,
					},
					keyTypeFuncPair{
						Type: types.ReturnCRDepositCoin,
						Func: strTxProgramCode,
					},
				),
			},
			// CRC proposal draft hash
			{
				name: slotCRCProposalDraftHash,
				slot: newConflictSlot(hash,
					keyTypeFuncPair{
						Type: types.CRCProposal,
						Func: hashCRCProposalDraftHash,
					},
				),
			},
			// CRC proposal DID
			{
				name: slotCRCProposalDID,
				slot: newConflictSlot(programHash,
					keyTypeFuncPair{
						Type: types.CRCProposal,
						Func: hashCRCProposalDID,
					},
				),
			},
			// CRC proposal hash
			{
				name: slotCRCProposalHash,
				slot: newConflictSlot(hash,
					keyTypeFuncPair{
						Type: types.CRCProposalWithdraw,
						Func: hashCRCProposalWithdrawProposalHash,
					},
				),
			},
			// CRC proposal tracking hash
			{
				name: slotCRCProposalTrackingHash,
				slot: newConflictSlot(hash,
					keyTypeFuncPair{
						Type: types.CRCProposalTracking,
						Func: hashCRCProposalTrackingProposalHash,
					},
				),
			},
			// CRC proposal review key
			{
				name: slotCRCProposalReviewKey,
				slot: newConflictSlot(str,
					keyTypeFuncPair{
						Type: types.CRCProposalReview,
						Func: strProposalReviewKey,
					},
				),
			},
			// CRC appropriation key
			{
				name: slotCRCAppropriationKey,
				slot: newConflictSlot(str,
					keyTypeFuncPair{
						Type: types.CRCAppropriation,
						Func: strCRCAppropriation,
					},
				),
			},
			// CRC proposal real withdraw transaction key
			{
				name: slotCRCProposalRealWithdrawKey,
				slot: newConflictSlot(hashArray,
					keyTypeFuncPair{
						Type: types.CRCProposalRealWithdraw,
						Func: hashArrayCRCProposalRealWithdrawTransactionHashes,
					},
				),
			},
			// special tx hash
			{
				name: slotSpecialTxHash,
				slot: newConflictSlot(hash,
					keyTypeFuncPair{
						Type: types.IllegalProposalEvidence,
						Func: hashSpecialTxHash,
					},
					keyTypeFuncPair{
						Type: types.IllegalVoteEvidence,
						Func: hashSpecialTxHash,
					},
					keyTypeFuncPair{
						Type: types.IllegalBlockEvidence,
						Func: hashSpecialTxHash,
					},
					keyTypeFuncPair{
						Type: types.IllegalSidechainEvidence,
						Func: hashSpecialTxHash,
					},
					keyTypeFuncPair{
						Type: types.InactiveArbitrators,
						Func: hashSpecialTxHash,
					},
				),
			},
			// side chain transaction hashes
			{
				name: slotSidechainTxHashes,
				slot: newConflictSlot(hashArray,
					keyTypeFuncPair{
						Type: types.WithdrawFromSideChain,
						Func: hashArraySidechainTransactionHashes,
					},
				),
			},
			// tx inputs refer keys
			{
				name: slotTxInputsReferKeys,
				slot: newConflictSlot(strArray,
					keyTypeFuncPair{
						Type: allType,
						Func: strArrayTxReferences,
					},
				),
			},
		},
	}
}
