package blockchain

import (
	"errors"

	"github.com/elastos/Elastos.ELA.SideChain.ID/core"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	ucore "github.com/elastos/Elastos.ELA.SideChain/core"
	"github.com/elastos/Elastos.ELA.SideChain/spv"
	"github.com/elastos/Elastos.ELA.Utility/common"
)

func InitTransactionValidtor() {
	blockchain.TransactionValidator = &blockchain.TransactionValidateBase{}
	blockchain.TransactionValidator.Init()
	blockchain.TransactionValidator.CheckTransactionPayload = CheckTransactionPayload
	blockchain.TransactionValidator.CheckTransactionSignature = CheckTransactionSignature

}

func CheckTransactionPayload(txn *ucore.Transaction) error {
	switch pld := txn.Payload.(type) {
	case *ucore.PayloadRegisterAsset:
		if pld.Asset.Precision < ucore.MinPrecision || pld.Asset.Precision > ucore.MaxPrecision {
			return errors.New("Invalide asset Precision.")
		}
		if !blockchain.TransactionValidator.CheckAmountPrecise(pld.Amount, pld.Asset.Precision, ucore.MaxPrecision) {
			return errors.New("Invalide asset value,out of precise.")
		}
	case *ucore.PayloadTransferAsset:
	case *ucore.PayloadRecord:
	case *ucore.PayloadCoinBase:
	case *ucore.PayloadRechargeToSideChain:
	case *ucore.PayloadTransferCrossChainAsset:
	case *core.PayloadRegisterIdentification:
	default:
		return errors.New("[txValidator],invalidate transaction payload type.")
	}
	return nil
}

func CheckTransactionSignature(txn *ucore.Transaction) error {
	if txn.IsRechargeToSideChainTx() {
		if err := spv.VerifyTransaction(txn); err != nil {
			return err
		}
		return nil
	}

	hashes, err := blockchain.GetTxProgramHashes(txn)
	if err != nil {
		return err
	}

	// Add ID program hash to hashes
	if core.IsRegisterIdentificationTx(txn) {
		for _, output := range txn.Outputs {
			if output.ProgramHash[0] == common.PrefixRegisterId {
				hashes = append(hashes, output.ProgramHash)
				break
			}
		}
	}

	// Sort first
	common.SortProgramHashes(hashes)
	if err := blockchain.SortPrograms(txn.Programs); err != nil {
		return err
	}

	return blockchain.RunPrograms(txn, hashes, txn.Programs)
}
