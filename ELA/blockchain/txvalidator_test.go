package blockchain

import (
	"github.com/elastos/Elastos.ELA/core"
	"testing"
)

func TestCheckDuplicateSidechainTx(t *testing.T) {
	// 1. Generate the ill withdraw transaction which have duplicate sidechain tx
	txn := new(core.Transaction)
	txn.TxType = core.WithdrawAsset
	txn.Payload = &core.PayloadWithdrawAsset{
		BlockHeight:         100,
		GenesisBlockAddress: "eb7adb1fea0dd6185b09a43bdcd4924bb22bff7151f0b1b4e08699840ab1384b",
		SideChainTransactionHash: []string{
			"8a6cb4b5ff1a4f8368c6513a536c663381e3fdeff738e9b437bd8fce3fb30b62",
			"cc62e14f5f9526b7f4ff9d34dcd0643dacb7886707c57f49ec97b95ec5c4edac",
			"8a6cb4b5ff1a4f8368c6513a536c663381e3fdeff738e9b437bd8fce3fb30b62", // duplicate tx hash
		},
	}

	// 2. Run CheckDuplicateSidechainTx
	err := CheckDuplicateSidechainTx(txn)
	if err == nil {
		t.Error("Should find the duplicate sidechain tx")
	}
}
