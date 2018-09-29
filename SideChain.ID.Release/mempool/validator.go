package mempool

import (
	"math"
	"errors"

	"github.com/elastos/Elastos.ELA.SideChain.ID/core"

	ucore "github.com/elastos/Elastos.ELA.SideChain/core"
	"github.com/elastos/Elastos.ELA.SideChain/spv"
	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.SideChain/mempool"
)

type validator struct {
	*mempool.Validator

	systemAssetID common.Uint256
	foundation    common.Uint168
	spvService 	  *spv.Service
}

func NewValidator(cfg *mempool.Config) *mempool.Validator{
	var val validator
	val.Validator =  mempool.NewValidator(cfg)
	val.systemAssetID = cfg.AssetId
	val.foundation = cfg.FoundationAddress
	val.spvService = cfg.SpvService

	val.RegisterSanityFunc(mempool.FuncNames.CheckTransactionOutput, val.checkTransactionOutput)
	val.RegisterSanityFunc(mempool.FuncNames.CheckTransactionPayload, val.checkTransactionPayload)
	val.RegisterContextFunc(mempool.FuncNames.CheckTransactionSignature, val.checkTransactionSignature)
	return val.Validator
}

func (v *validator) checkTransactionPayload(txn *ucore.Transaction) error {
	switch pld := txn.Payload.(type) {
	case *ucore.PayloadRegisterAsset:
		if pld.Asset.Precision < ucore.MinPrecision || pld.Asset.Precision > ucore.MaxPrecision {
			return errors.New("[ID CheckTransactionPayload] Invalide asset Precision.")
		}
		if !checkAmountPrecise(pld.Amount, pld.Asset.Precision, ucore.MaxPrecision) {
			return errors.New("[ID CheckTransactionPayload] Invalide asset value,out of precise.")
		}
	case *ucore.PayloadTransferAsset:
	case *ucore.PayloadRecord:
	case *ucore.PayloadCoinBase:
	case *ucore.PayloadRechargeToSideChain:
	case *ucore.PayloadTransferCrossChainAsset:
	case *core.PayloadRegisterIdentification:
	default:
		return errors.New("[ID CheckTransactionPayload] [txValidator],invalidate transaction payload type.")
	}
	return nil
}

func checkAmountPrecise(amount common.Fixed64, precision byte, assetPrecision byte) bool {
	return amount.IntValue()%int64(math.Pow10(int(assetPrecision-precision))) == 0
}

func (v *validator) checkTransactionOutput(txn *ucore.Transaction) error {
	if txn.IsCoinBaseTx() {
		if len(txn.Outputs) < 2 {
			return errors.New("[checkTransactionOutput] coinbase output is not enough, at least 2")
		}

		var totalReward = common.Fixed64(0)
		var foundationReward = common.Fixed64(0)
		for _, output := range txn.Outputs {
			if !output.AssetID.IsEqual(v.systemAssetID) {
				return errors.New("[checkTransactionOutput] asset ID in coinbase is invalid")
			}
			totalReward += output.Value
			if output.ProgramHash.IsEqual(v.foundation) {
				foundationReward += output.Value
			}
		}
		if common.Fixed64(foundationReward) < common.Fixed64(float64(totalReward)*0.3) {
			return errors.New("[checkTransactionOutput] Reward to foundation in coinbase < 30%")
		}

		return nil
	}

	if len(txn.Outputs) < 1 {
		return errors.New("[checkTransactionOutput] transaction has no outputs")
	}

	// check if output address is valid
	for _, output := range txn.Outputs {
		if output.AssetID != v.systemAssetID {
		return errors.New("[checkTransactionOutput] asset ID in output is invalid")
		}

		if !checkOutputProgramHash(output.ProgramHash) {
			return errors.New("[checkTransactionOutput] output address is invalid")
		}
	}

	return nil
}

func checkOutputProgramHash(programHash common.Uint168) bool {
	var empty = common.Uint168{}
	prefix := programHash[0]
	if prefix == common.PrefixStandard ||
		prefix == common.PrefixMultisig ||
		prefix == common.PrefixCrossChain ||
		prefix == common.PrefixRegisterId ||
		programHash == empty {
		return true
	}
	return false
}

func (v *validator) checkTransactionSignature(txn *ucore.Transaction) error {
	if txn.IsRechargeToSideChainTx() {
		if err := v.spvService.VerifyTransaction(txn); err != nil {
			return errors.New("[ID checkTransactionSignature] Invalide recharge to side chain tx: " + err.Error())
		}
		return nil
	}

	hashes, err := v.TxProgramHashes(txn)
	if err != nil {
		return errors.New("[ID checkTransactionSignature] Get program hashes error:" + err.Error())
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
	if err := mempool.SortPrograms(txn.Programs); err != nil {
		return errors.New("[ID checkTransactionSignature] Sort program hashes error:" + err.Error())
	}

	err = mempool.RunPrograms(txn, hashes, txn.Programs)
	if err != nil {
		return errors.New("[ID checkTransactionSignature] Run program error:" + err.Error())
	}

	return nil
}
