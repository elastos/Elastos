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
