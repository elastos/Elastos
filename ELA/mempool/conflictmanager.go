package mempool

import (
	"fmt"

	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/errors"
)

const (
	slotDPoSOwnerPublicKey      = "DPoSOwnerPublicKey"
	slotDPoSNodePublicKey       = "DPoSNodePublicKey"
	slotDPoSNickname            = "DPoSNickname"
	slotCRDID                   = "CrDID"
	slotCRNickname              = "CrNickname"
	slotProgramCode             = "ProgramCode"
	slotCRCProposalDraftHash    = "CRCProposalDraftHash"
	slotCRCProposalHash         = "CRCProposalHash"
	slotCRCProposalTrackingHash = "CRCProposalTrackingHash"
	slotCRCProposalReviewKey    = "CRCProposalReviewKey"
	slotCRCAppropriationKey     = "CRCAppropriationKey"
	slotSpecialTxHash           = "SpecialTxHash"
	slotSidechainTxHashes       = "SidechainTxHashes"
	slotTxInputsReferKeys       = "TxInputsReferKeys"
)

// conflictManager hold a set of conflict slots, and refer some query methods.
type conflictManager struct {
	conflictSlots map[string]*conflictSlot
}

func (m *conflictManager) VerifyTx(tx *types.Transaction) errors.ELAError {
	for k, v := range m.conflictSlots {
		if err := v.VerifyTx(tx); err != nil {
			return errors.SimpleWithMessage(errors.ErrTxPoolFailure, err,
				fmt.Sprintf("slot %s verify tx error", k))
		}
	}
	return nil
}

func (m *conflictManager) AppendTx(tx *types.Transaction) errors.ELAError {
	for k, v := range m.conflictSlots {
		if err := v.AppendTx(tx); err != nil {
			return errors.SimpleWithMessage(errors.ErrTxPoolFailure, err,
				fmt.Sprintf("slot %s append tx error", k))
		}
	}
	return nil
}

func (m *conflictManager) RemoveTx(tx *types.Transaction) errors.ELAError {
	for k, v := range m.conflictSlots {
		if err := v.RemoveTx(tx); err != nil {
			return errors.SimpleWithMessage(errors.ErrTxPoolFailure, err,
				fmt.Sprintf("slot %s remove tx error", k))
		}
	}
	return nil
}

func (m *conflictManager) GetTx(key interface{},
	slotName string) *types.Transaction {
	slot, ok := m.conflictSlots[slotName]
	if !ok {
		return nil
	}
	return slot.GetTx(key)
}

func (m *conflictManager) ContainsKey(key interface{}, slotName string) bool {
	slot, ok := m.conflictSlots[slotName]
	if !ok {
		return false
	}
	return slot.Contains(key)
}

func (m *conflictManager) RemoveKey(key interface{},
	slotName string) errors.ELAError {
	slot, ok := m.conflictSlots[slotName]
	if !ok {
		return errors.SimpleWithMessage(errors.ErrTxPoolFailure, nil,
			fmt.Sprintf("slot %s not exist", slotName))
	}
	return slot.removeKey(key)
}

func (m *conflictManager) Empty() bool {
	for _, v := range m.conflictSlots {
		if !v.Empty() {
			return false
		}
	}
	return true
}

func newConflictManager() conflictManager {
	return conflictManager{
		conflictSlots: map[string]*conflictSlot{
			// DPoS owner public key
			slotDPoSOwnerPublicKey: newConflictSlot(str,
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
			// DPoS node public key
			slotDPoSNodePublicKey: newConflictSlot(str,
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
			// DPoS nickname
			slotDPoSNickname: newConflictSlot(str,
				keyTypeFuncPair{
					Type: types.RegisterProducer,
					Func: strProducerInfoNickname,
				},
				keyTypeFuncPair{
					Type: types.UpdateProducer,
					Func: strProducerInfoNickname,
				},
			),
			// CR DID
			slotCRDID: newConflictSlot(programHash,
				keyTypeFuncPair{
					Type: types.RegisterCR,
					Func: addrCRInfoCRDID,
				},
				keyTypeFuncPair{
					Type: types.UpdateCR,
					Func: addrCRInfoCRDID,
				},
				keyTypeFuncPair{
					Type: types.UnregisterCR,
					Func: addrUnregisterCRDID,
				},
			),
			// CR nickname
			slotCRNickname: newConflictSlot(str,
				keyTypeFuncPair{
					Type: types.RegisterCR,
					Func: strCRInfoNickname,
				},
				keyTypeFuncPair{
					Type: types.UpdateCR,
					Func: strCRInfoNickname,
				},
			),
			// CR and DPoS program code
			slotProgramCode: newConflictSlot(str,
				keyTypeFuncPair{
					Type: types.ReturnDepositCoin,
					Func: strTxProgramCode,
				},
				keyTypeFuncPair{
					Type: types.ReturnCRDepositCoin,
					Func: strTxProgramCode,
				},
			),
			// CRC proposal draft hash
			slotCRCProposalDraftHash: newConflictSlot(hash,
				keyTypeFuncPair{
					Type: types.CRCProposal,
					Func: hashCRCProposalDraftHash,
				},
			),
			// CRC proposal hash
			slotCRCProposalHash: newConflictSlot(hash,
				keyTypeFuncPair{
					Type: types.CRCProposalWithdraw,
					Func: hashCRCProposalWithdrawProposalHash,
				},
			),
			// CRC proposal tracking hash
			slotCRCProposalTrackingHash: newConflictSlot(hash,
				keyTypeFuncPair{
					Type: types.CRCProposalTracking,
					Func: hashCRCProposalTrackingProposalHash,
				},
			),
			// CRC proposal review key
			slotCRCProposalReviewKey: newConflictSlot(str,
				keyTypeFuncPair{
					Type: types.CRCProposalReview,
					Func: strProposalReviewKey,
				},
			),
			// CRC appropriation key
			slotCRCAppropriationKey: newConflictSlot(str,
				keyTypeFuncPair{
					Type: types.CRCAppropriation,
					Func: strCRCAppropriation,
				},
			),
			// special tx hash
			slotSpecialTxHash: newConflictSlot(hash,
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
			// side chain transaction hashes
			slotSidechainTxHashes: newConflictSlot(hashArray,
				keyTypeFuncPair{
					Type: types.WithdrawFromSideChain,
					Func: hashArraySidechainTransactionHashes,
				},
			),
			// tx inputs refer keys
			slotTxInputsReferKeys: newConflictSlot(strArray,
				keyTypeFuncPair{
					Type: allType,
					Func: strArrayTxReferences,
				},
			),
		},
	}
}
