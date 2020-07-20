// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package mempool

import (
	"crypto/rand"
	mrand "math/rand"
	"testing"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/stretchr/testify/assert"
)

func TestConflictManager_DPoS_OwnerPublicKey(t *testing.T) {
	conflictTestProc(func(db *UtxoCacheDB) {
		pk := randomPublicKey()
		txs := []*types.Transaction{
			{
				TxType: types.RegisterProducer,
				Payload: &payload.ProducerInfo{
					OwnerPublicKey: pk,
					NodePublicKey:  randomPublicKey(),
					NickName:       randomNickname(),
				},
			},
			{
				TxType: types.UpdateProducer,
				Payload: &payload.ProducerInfo{
					OwnerPublicKey: pk,
					NodePublicKey:  randomPublicKey(),
					NickName:       randomNickname(),
				},
			},
			{
				TxType: types.CancelProducer,
				Payload: &payload.ProcessProducer{
					OwnerPublicKey: pk,
				},
			},
			{
				TxType: types.RegisterCR,
				Payload: &payload.CRInfo{
					Code: redeemScriptFromPk(pk),
				},
			},
		}

		verifyTxListWithConflictManager(txs, db, true, t)
	})
}

func TestConflictManager_DPoS_NodePublicKey(t *testing.T) {
	conflictTestProc(func(db *UtxoCacheDB) {
		pk := randomPublicKey()
		txs := []*types.Transaction{
			{
				TxType: types.RegisterProducer,
				Payload: &payload.ProducerInfo{
					OwnerPublicKey: randomPublicKey(),
					NodePublicKey:  pk,
					NickName:       randomNickname(),
				},
			},
			{
				TxType: types.UpdateProducer,
				Payload: &payload.ProducerInfo{
					OwnerPublicKey: randomPublicKey(),
					NodePublicKey:  pk,
					NickName:       randomNickname(),
				},
			},
			{
				TxType: types.ActivateProducer,
				Payload: &payload.ActivateProducer{
					NodePublicKey: pk,
				},
			},
			{
				TxType: types.RegisterCR,
				Payload: &payload.CRInfo{
					Code: redeemScriptFromPk(pk),
				},
			},
		}

		verifyTxListWithConflictManager(txs, db, true, t)
	})
}

func TestConflictManager_DPoS_Nickname(t *testing.T) {
	conflictTestProc(func(db *UtxoCacheDB) {
		name := randomNickname()
		txs := []*types.Transaction{
			{
				TxType: types.RegisterProducer,
				Payload: &payload.ProducerInfo{
					OwnerPublicKey: randomPublicKey(),
					NodePublicKey:  randomPublicKey(),
					NickName:       name,
				},
			},
			{
				TxType: types.UpdateProducer,
				Payload: &payload.ProducerInfo{
					OwnerPublicKey: randomPublicKey(),
					NodePublicKey:  randomPublicKey(),
					NickName:       name,
				},
			},
		}

		verifyTxListWithConflictManager(txs, db, true, t)
	})
}

func TestConflictManager_CR_DID(t *testing.T) {
	conflictTestProc(func(db *UtxoCacheDB) {
		cid := *randomProgramHash()
		txs := []*types.Transaction{
			{
				TxType: types.RegisterCR,
				Payload: &payload.CRInfo{
					CID:      cid,
					Code:     redeemScriptFromPk(randomPublicKey()),
					NickName: randomNickname(),
				},
			},
			{
				TxType: types.UpdateCR,
				Payload: &payload.CRInfo{
					CID:      cid,
					Code:     redeemScriptFromPk(randomPublicKey()),
					NickName: randomNickname(),
				},
			},
			{
				TxType: types.UnregisterCR,
				Payload: &payload.UnregisterCR{
					CID: cid,
				},
			},
		}

		verifyTxListWithConflictManager(txs, db, true, t)
	})
}

func TestConflictManager_CR_Nickname(t *testing.T) {
	conflictTestProc(func(db *UtxoCacheDB) {
		name := randomNickname()
		txs := []*types.Transaction{
			{
				TxType: types.RegisterCR,
				Payload: &payload.CRInfo{
					DID:      *randomProgramHash(),
					Code:     redeemScriptFromPk(randomPublicKey()),
					NickName: name,
				},
			},
			{
				TxType: types.UpdateCR,
				Payload: &payload.CRInfo{
					DID:      *randomProgramHash(),
					Code:     redeemScriptFromPk(randomPublicKey()),
					NickName: name,
				},
			},
		}

		verifyTxListWithConflictManager(txs, db, true, t)
	})
}

func TestConflictManager_ProgramCode(t *testing.T) {
	conflictTestProc(func(db *UtxoCacheDB) {
		code := redeemScriptFromPk(randomPublicKey())
		txs := []*types.Transaction{
			{
				TxType:  types.ReturnDepositCoin,
				Payload: &payload.ReturnDepositCoin{},
				Programs: []*program.Program{
					{
						Code: code,
					},
				},
			},
			{
				TxType:  types.ReturnCRDepositCoin,
				Payload: &payload.ReturnDepositCoin{},
				Programs: []*program.Program{
					{
						Code: code,
					},
				},
			},
		}

		verifyTxListWithConflictManager(txs, db, true, t)
	})
}

func TestConflictManager_CR_DraftHash(t *testing.T) {
	conflictTestProc(func(db *UtxoCacheDB) {
		hash := *randomHash()
		txs := []*types.Transaction{
			{
				TxType: types.CRCProposal,
				Payload: &payload.CRCProposal{
					DraftHash:          hash,
					CRCouncilMemberDID: *randomProgramHash(),
				},
			},
			{
				TxType: types.CRCProposal,
				Payload: &payload.CRCProposal{
					DraftHash:          hash,
					CRCouncilMemberDID: *randomProgramHash(),
				},
			},
		}

		verifyTxListWithConflictManager(txs, db, true, t)
	})
}

func TestConflictManager_CR_SponsorDID(t *testing.T) {
	did := *randomProgramHash()
	conflictTestProc(func(db *UtxoCacheDB) {
		txs := []*types.Transaction{
			{
				TxType: types.CRCProposal,
				Payload: &payload.CRCProposal{
					DraftHash:          *randomHash(),
					CRCouncilMemberDID: did,
				},
			},
			{
				TxType: types.CRCProposal,
				Payload: &payload.CRCProposal{
					DraftHash:          *randomHash(),
					CRCouncilMemberDID: did,
				},
			},
		}

		verifyTxListWithConflictManager(txs, db, true, t)
	})
}

func TestConflictManager_CR_ProposalHash(t *testing.T) {
	conflictTestProc(func(db *UtxoCacheDB) {
		hash := *randomHash()
		txs := []*types.Transaction{
			{
				TxType: types.CRCProposalWithdraw,
				Payload: &payload.CRCProposalWithdraw{
					ProposalHash: hash,
				},
			},
		}

		verifyTxListWithConflictManager(txs, db, true, t)
	})
}

func TestConflictManager_CR_ProposalTrackHash(t *testing.T) {
	conflictTestProc(func(db *UtxoCacheDB) {
		hash := *randomHash()
		txs := []*types.Transaction{
			{
				TxType: types.CRCProposalTracking,
				Payload: &payload.CRCProposalTracking{
					ProposalHash: hash,
				},
			},
		}

		verifyTxListWithConflictManager(txs, db, true, t)
	})
}

func TestConflictManager_CR_ProposalReviewKey(t *testing.T) {
	conflictTestProc(func(db *UtxoCacheDB) {
		hash := *randomHash()
		did := *randomProgramHash()
		txs := []*types.Transaction{
			{
				TxType: types.CRCProposalReview,
				Payload: &payload.CRCProposalReview{
					ProposalHash: hash,
					DID:          did,
				},
			},
		}

		verifyTxListWithConflictManager(txs, db, true, t)
	})
}

func TestConflictManager_CR_AppropriationKey(t *testing.T) {
	conflictTestProc(func(db *UtxoCacheDB) {
		txs := []*types.Transaction{
			{
				TxType:  types.CRCAppropriation,
				Payload: &payload.CRCAppropriation{},
			},
		}

		verifyTxListWithConflictManager(txs, db, true, t)
	})
}

func TestConflictManager_SpecialTxHashes(t *testing.T) {
	conflictTestProc(func(db *UtxoCacheDB) {
		txs := []*types.Transaction{
			{
				TxType: types.IllegalProposalEvidence,
				Payload: &payload.DPOSIllegalProposals{
					Evidence: payload.ProposalEvidence{
						BlockHeader: randomHash().Bytes(),
					},
					CompareEvidence: payload.ProposalEvidence{
						BlockHeader: randomHash().Bytes(),
					},
				},
			},
		}

		verifyTxListWithConflictManager(txs, db, true, t)
	})

	conflictTestProc(func(db *UtxoCacheDB) {
		txs := []*types.Transaction{
			{
				TxType: types.IllegalVoteEvidence,
				Payload: &payload.DPOSIllegalVotes{
					Evidence: payload.VoteEvidence{
						ProposalEvidence: payload.ProposalEvidence{
							BlockHeader: randomHash().Bytes(),
						},
					},
					CompareEvidence: payload.VoteEvidence{
						ProposalEvidence: payload.ProposalEvidence{
							BlockHeader: randomHash().Bytes(),
						},
					},
				},
			},
		}

		verifyTxListWithConflictManager(txs, db, true, t)
	})

	conflictTestProc(func(db *UtxoCacheDB) {
		txs := []*types.Transaction{
			{
				TxType: types.IllegalBlockEvidence,
				Payload: &payload.DPOSIllegalBlocks{
					Evidence: payload.BlockEvidence{
						Header: randomHash().Bytes(),
					},
					CompareEvidence: payload.BlockEvidence{
						Header: randomHash().Bytes(),
					},
				},
			},
		}

		verifyTxListWithConflictManager(txs, db, true, t)
	})

	conflictTestProc(func(db *UtxoCacheDB) {
		txs := []*types.Transaction{
			{
				TxType: types.IllegalSidechainEvidence,
				Payload: &payload.SidechainIllegalData{
					Evidence: payload.SidechainIllegalEvidence{
						DataHash: *randomHash(),
					},
					CompareEvidence: payload.SidechainIllegalEvidence{
						DataHash: *randomHash(),
					},
				},
			},
		}

		verifyTxListWithConflictManager(txs, db, true, t)
	})

	conflictTestProc(func(db *UtxoCacheDB) {
		txs := []*types.Transaction{
			{
				TxType: types.InactiveArbitrators,
				Payload: &payload.InactiveArbitrators{
					Arbitrators: [][]byte{
						randomPublicKey(),
						randomPublicKey(),
						randomPublicKey(),
					},
				},
			},
		}

		verifyTxListWithConflictManager(txs, db, true, t)
	})
}

func TestConflictManager_Sidechain_TxHashes(t *testing.T) {
	conflictTestProc(func(db *UtxoCacheDB) {
		hash := *randomHash()
		txs := []*types.Transaction{
			{
				TxType: types.WithdrawFromSideChain,
				Payload: &payload.WithdrawFromSideChain{
					SideChainTransactionHashes: []common.Uint256{
						hash,
						*randomHash(),
						*randomHash(),
					},
				},
			},
			{
				TxType: types.WithdrawFromSideChain,
				Payload: &payload.WithdrawFromSideChain{
					SideChainTransactionHashes: []common.Uint256{
						hash,
						*randomHash(),
						*randomHash(),
					},
				},
			},
		}

		verifyTxListWithConflictManager(txs, db, true, t)
	})
}

func TestConflictManager_InputInferKeys(t *testing.T) {
	conflictTestProc(func(db *UtxoCacheDB) {
		txs := []*types.Transaction{
			{
				TxType: types.RegisterProducer,
				Payload: &payload.ProducerInfo{
					OwnerPublicKey: randomPublicKey(),
					NodePublicKey:  randomPublicKey(),
					NickName:       randomNickname(),
				},
			},
			{
				TxType: types.UpdateProducer,
				Payload: &payload.ProducerInfo{
					OwnerPublicKey: randomPublicKey(),
					NodePublicKey:  randomPublicKey(),
					NickName:       randomNickname(),
				},
			},
			{
				TxType: types.CancelProducer,
				Payload: &payload.ProcessProducer{
					OwnerPublicKey: randomPublicKey(),
				},
			},
			{
				TxType: types.RegisterCR,
				Payload: &payload.CRInfo{
					DID:      *randomProgramHash(),
					Code:     redeemScriptFromPk(randomPublicKey()),
					NickName: randomNickname(),
				},
			},
			{
				TxType: types.UpdateCR,
				Payload: &payload.CRInfo{
					DID:      *randomProgramHash(),
					Code:     redeemScriptFromPk(randomPublicKey()),
					NickName: randomNickname(),
				},
			},
			{
				TxType: types.UnregisterCR,
				Payload: &payload.UnregisterCR{
					CID: *randomProgramHash(),
				},
			},
			{
				TxType:  types.ReturnDepositCoin,
				Payload: &payload.ReturnDepositCoin{},
				Programs: []*program.Program{
					{
						Code: redeemScriptFromPk(randomPublicKey()),
					},
				},
			},
			{
				TxType:  types.ReturnCRDepositCoin,
				Payload: &payload.ReturnDepositCoin{},
				Programs: []*program.Program{
					{
						Code: redeemScriptFromPk(randomPublicKey()),
					},
				},
			},
			{
				TxType: types.CRCProposal,
				Payload: &payload.CRCProposal{
					DraftHash: *randomHash(),
				},
			},
			{
				TxType: types.CRCProposalWithdraw,
				Payload: &payload.CRCProposalWithdraw{
					ProposalHash: *randomHash(),
				},
			},
			{
				TxType: types.CRCProposalTracking,
				Payload: &payload.CRCProposalTracking{
					ProposalHash: *randomHash(),
				},
			},
			{
				TxType: types.CRCProposalReview,
				Payload: &payload.CRCProposalReview{
					ProposalHash: *randomHash(),
					DID:          *randomProgramHash(),
				},
			},
			{
				TxType:  types.CRCAppropriation,
				Payload: &payload.CRCAppropriation{},
			},
			{
				TxType: types.WithdrawFromSideChain,
				Payload: &payload.WithdrawFromSideChain{
					SideChainTransactionHashes: []common.Uint256{
						*randomHash(),
						*randomHash(),
					},
				},
			},
		}

		verifyTxListWithConflictManager(txs, db, false, t)
	})
}

func conflictTestProc(action func(*UtxoCacheDB)) {
	origin := blockchain.DefaultLedger
	utxoCacheDB := NewUtxoCacheDB()
	blockchain.DefaultLedger = &blockchain.Ledger{
		Blockchain: &blockchain.BlockChain{
			UTXOCache: blockchain.NewUTXOCache(utxoCacheDB,
				&config.DefaultParams),
		},
	}
	action(utxoCacheDB)
	blockchain.DefaultLedger = origin
}

func setPreviousTransactionIndividually(txs []*types.Transaction,
	utxoCacheDB *UtxoCacheDB) {
	for _, tx := range txs {
		prevTx := newPreviousTx(utxoCacheDB)
		tx.Inputs = []*types.Input{
			{
				Previous: types.OutPoint{
					TxID:  prevTx.Hash(),
					Index: 0,
				},
				Sequence: 100,
			},
		}
	}
}

func setSamePreviousTransaction(txs []*types.Transaction,
	utxoCacheDB *UtxoCacheDB) {
	prevTx := newPreviousTx(utxoCacheDB)
	for _, tx := range txs {
		tx.Inputs = []*types.Input{
			{
				Previous: types.OutPoint{
					TxID:  prevTx.Hash(),
					Index: 0,
				},
				Sequence: 100,
			},
		}
	}
}

func newPreviousTx(utxoCacheDB *UtxoCacheDB) *types.Transaction {
	prevTx := &types.Transaction{
		TxType:  types.TransferAsset,
		Payload: &payload.TransferAsset{},
		Outputs: []*types.Output{
			{
				Value:       common.Fixed64(mrand.Int63()),
				ProgramHash: *randomProgramHash(),
			},
		},
	}
	utxoCacheDB.PutTransaction(prevTx)
	return prevTx
}

func verifyTxListWithConflictManager(txs []*types.Transaction,
	utxoCacheDB *UtxoCacheDB, individualPreTx bool, t *testing.T) {
	if individualPreTx {
		setPreviousTransactionIndividually(txs, utxoCacheDB)
	} else {
		setSamePreviousTransaction(txs, utxoCacheDB)
	}

	manager := newConflictManager()
	for _, addedTx := range txs {
		assert.NoError(t, manager.VerifyTx(addedTx))
		assert.NoError(t, manager.AppendTx(addedTx))
		for _, candidate := range txs {
			assert.True(t, manager.VerifyTx(candidate) != nil)
		}

		assert.NoError(t, manager.removeTx(addedTx))
		assert.True(t, manager.Empty())
		for _, candidate := range txs {
			assert.NoError(t, manager.VerifyTx(candidate))
		}
	}
}

func randomHash() *common.Uint256 {
	a := make([]byte, 32)
	rand.Read(a)
	hash, _ := common.Uint256FromBytes(a)
	return hash
}

func redeemScriptFromPk(pk []byte) []byte {
	pubKey, _ := crypto.DecodePoint(pk)
	rtn, _ := contract.CreateStandardRedeemScript(pubKey)
	return rtn
}

func randomPublicKey() []byte {
	_, pub, _ := crypto.GenerateKeyPair()
	result, _ := pub.EncodePoint(true)
	return result
}

func randomNickname() string {
	var name [20]byte
	rand.Read(name[:])
	return string(name[:])
}

func randomProgramHash() *common.Uint168 {
	a := make([]byte, 21)
	rand.Read(a)
	hash, _ := common.Uint168FromBytes(a)
	return hash
}
