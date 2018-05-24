package blockchain

import (
	"github.com/elastos/Elastos.ELA/core"
	"testing"
)

var txPool TxPool

func TestTxPool_VerifyDuplicateSidechainTx(t *testing.T) {
	txPool.Init()

	// 1. Generate a withdraw transaction
	txn1 := new(core.Transaction)
	txn1.TxType = core.WithdrawAsset
	txn1.Payload = &core.PayloadWithdrawAsset{
		BlockHeight:         100,
		GenesisBlockAddress: "eb7adb1fea0dd6185b09a43bdcd4924bb22bff7151f0b1b4e08699840ab1384b",
		SideChainTransactionHash: []string{
			"8a6cb4b5ff1a4f8368c6513a536c663381e3fdeff738e9b437bd8fce3fb30b62",
			"cc62e14f5f9526b7f4ff9d34dcd0643dacb7886707c57f49ec97b95ec5c4edac",
		},
	}

	// 2. Add sidechain Tx to pool
	witPayload := txn1.Payload.(*core.PayloadWithdrawAsset)
	for _, hash := range witPayload.SideChainTransactionHash {
		success := txPool.addSidechainTx(hash)
		if !success {
			t.Error("Add sidechain Tx to pool failed")
		}
	}

	// 3. Generate a withdraw transaction with duplicate sidechain Tx which already in the pool
	txn2 := new(core.Transaction)
	txn2.TxType = core.WithdrawAsset
	txn2.Payload = &core.PayloadWithdrawAsset{
		BlockHeight:         100,
		GenesisBlockAddress: "eb7adb1fea0dd6185b09a43bdcd4924bb22bff7151f0b1b4e08699840ab1384b",
		SideChainTransactionHash: []string{
			"8a6cb4b5ff1a4f8368c6513a536c663381e3fdeff738e9b437bd8fce3fb30b62", // duplicate sidechain Tx
		},
	}

	// 4. Run verifyDuplicateSidechainTx
	err := txPool.verifyDuplicateSidechainTx(txn2)
	if err == nil {
		t.Error("Should find the duplicate sidechain tx")
	}
}

func TestTxPool_CleanSidechainTx(t *testing.T) {
	// 1. Generate some withdraw transactions
	txn1 := new(core.Transaction)
	txn1.TxType = core.WithdrawAsset
	txn1.Payload = &core.PayloadWithdrawAsset{
		BlockHeight:         100,
		GenesisBlockAddress: "eb7adb1fea0dd6185b09a43bdcd4924bb22bff7151f0b1b4e08699840ab1384b",
		SideChainTransactionHash: []string{
			"300db7783393a6f60533c1223108445df57de4fb4842f84f55d07df57caa0c7d",
			"d6c2cb8345a8fe4af0d103cc4e40dbb0654bb169a85bb8cc57923d0c72f3658f",
		},
	}

	txn2 := new(core.Transaction)
	txn2.TxType = core.WithdrawAsset
	txn2.Payload = &core.PayloadWithdrawAsset{
		BlockHeight:         100,
		GenesisBlockAddress: "eb7adb1fea0dd6185b09a43bdcd4924bb22bff7151f0b1b4e08699840ab1384b",
		SideChainTransactionHash: []string{
			"326218253e6feaa21e3521eff27418b942a5fbd45347505f3e5aca0463baffe2",
		},
	}

	txn3 := new(core.Transaction)
	txn3.TxType = core.WithdrawAsset
	txn3.Payload = &core.PayloadWithdrawAsset{
		BlockHeight:         100,
		GenesisBlockAddress: "eb7adb1fea0dd6185b09a43bdcd4924bb22bff7151f0b1b4e08699840ab1384b",
		SideChainTransactionHash: []string{
			"645b614eaaa0a1bfd7015d88f3c1343048343924fc105e403b735ba754caa8db",
			"9dcad6d4ec2851bf522ddd301c7567caf98554a82a0bcce866de80b503909642",
		},
	}
	txns := []*core.Transaction{txn1, txn2, txn3}

	// 2. Add to sidechain txs pool
	for _, txn := range txns {
		witPayload := txn.Payload.(*core.PayloadWithdrawAsset)
		for _, hash := range witPayload.SideChainTransactionHash {
			success := txPool.addSidechainTx(hash)
			if !success {
				t.Error("Add to sidechain tx pool failed")
			}
		}
	}

	// Verify sidechain tx pool state
	for _, txn := range txns {
		err := txPool.verifyDuplicateSidechainTx(txn)
		if err == nil {
			t.Error("Should find the duplicate sidechain tx")
		}
	}

	// 3. Run cleanSidechainTx
	txPool.cleanSidechainTx(txns)

	// Verify sidechian tx pool state
	for _, txn := range txns {
		err := txPool.verifyDuplicateSidechainTx(txn)
		if err != nil {
			t.Error("Should not find the duplicate sidechain tx")
		}
	}
}
