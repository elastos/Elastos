package mempool

import (
	"errors"
	"math"
	"fmt"

	"github.com/elastos/Elastos.ELA.SideChain/mempool"
	"github.com/elastos/Elastos.ELA.SideChain/spv"
	side "github.com/elastos/Elastos.ELA.SideChain/types"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/types"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/smartcontract/service"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/store"
	nc "github.com/elastos/Elastos.ELA.SideChain.NeoVM/common"
)

type validator struct {
	*mempool.Validator

	systemAssetID common.Uint256
	foundation    common.Uint168
	spvService    *spv.Service
}

func NewValidator(cfg *mempool.Config) *mempool.Validator {
	var val validator
	val.Validator = mempool.NewValidator(cfg)
	val.systemAssetID = cfg.ChainParams.ElaAssetId
	val.foundation = cfg.ChainParams.Foundation
	val.spvService = cfg.SpvService

	val.RegisterSanityFunc(mempool.FuncNames.CheckTransactionOutput, val.checkTransactionOutput)
	val.RegisterSanityFunc(mempool.FuncNames.CheckTransactionPayload, val.checkTransactionPayload)
	val.RegisterContextFunc(mempool.FuncNames.CheckTransactionSignature, val.checkTransactionSignature)
	val.RegisterSanityFunc(mempool.FuncNames.CheckAttributeProgram, val.checkAttributeProgram)

	return val.Validator
}

func (v *validator) checkTransactionPayload(txn *side.Transaction) error {
	switch pld := txn.Payload.(type) {
	case *side.PayloadRegisterAsset:
		if pld.Asset.Precision < side.MinPrecision || pld.Asset.Precision > side.MaxPrecision {
			return errors.New("[NeoVM CheckTransactionPayload] Invalide asset Precision.")
		}
		if !checkAmountPrecise(pld.Amount, pld.Asset.Precision, side.MaxPrecision) {
			return errors.New("[NeoVM CheckTransactionPayload] Invalide asset value,out of precise.")
		}
	case *side.PayloadTransferAsset:
	case *side.PayloadRecord:
	case *side.PayloadCoinBase:
	case *side.PayloadRechargeToSideChain:
	case *side.PayloadTransferCrossChainAsset:
	case *types.PayloadDeploy:
	case *types.PayloadInvoke:
	default:
		return errors.New("[NeoVM CheckTransactionPayload] [txValidator],invalidate transaction payload type.")
	}
	return nil
}

func checkAmountPrecise(amount common.Fixed64, precision byte, assetPrecision byte) bool {
	return amount.IntValue()%int64(math.Pow10(int(assetPrecision-precision))) == 0
}

func (v *validator) checkTransactionOutput(txn *side.Transaction) error {
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
	if programHash.IsEqual(common.Uint168{}) {
		return true
	}

	switch contract.PrefixType(programHash[0]) {
	case contract.PrefixStandard:
		fallthrough
	case contract.PrefixMultiSig:
		fallthrough
	case contract.PrefixCrossChain:
		return true
	case nc.PrefixSmartContract:
		return true
	}
	return false
}

func (v *validator) checkTransactionSignature(txn *side.Transaction) error {
	if txn.IsRechargeToSideChainTx() {
		if err := v.spvService.VerifyTransaction(txn); err != nil {
			return errors.New("[NeoVM checkTransactionSignature] Invalide recharge to side chain tx: " + err.Error())
		}
		return nil
	}

	hashes, err := v.TxProgramHashes(txn)
	if err != nil {
		return errors.New("[NeoVM checkTransactionSignature] Get program hashes error:" + err.Error())
	}

	// Sort first
	common.SortProgramHashByCodeHash(hashes)
	if err := mempool.SortPrograms(txn.Programs); err != nil {
		return errors.New("[NeoVM checkTransactionSignature] Sort program hashes error:" + err.Error())
	}

	err = RunPrograms(txn, hashes, txn.Programs)
	if err != nil {
		return errors.New("[NeoVM checkTransactionSignature] Run program error:" + err.Error())
	}

	return nil
}

func (v *validator) checkAttributeProgram(txn *side.Transaction) error {
	// Check attributes
	for _, attr := range txn.Attributes {
		if !side.IsValidAttributeType(attr.Usage) {
			str := fmt.Sprintf("[checkAttributeProgram] invalid attribute usage %s", attr.Usage.Name())
			return mempool.RuleError{ErrorCode: mempool.ErrAttributeProgram, Description: str}
		}
	}

	// Check programs
	for _, program := range txn.Programs {
		if program.Code == nil {
			str := fmt.Sprint("[checkAttributeProgram] invalid program code nil")
			return mempool.RuleError{ErrorCode: mempool.ErrAttributeProgram, Description: str}
		}
		if program.Parameter == nil {
			str := fmt.Sprint("[checkAttributeProgram] invalid program parameter nil")
			return mempool.RuleError{ErrorCode: mempool.ErrAttributeProgram, Description: str}
		}
		_, err := nc.ToProgramHash(program.Code)

		if err != nil {
			str := fmt.Sprintf("[checkAttributeProgram] invalid program code %x", program.Code)
			return mempool.RuleError{ErrorCode: mempool.ErrAttributeProgram, Description: str}
		}
	}
	return nil
}

func RunPrograms(tx *side.Transaction, hashes []common.Uint168, programs []*side.Program) error {
	if tx == nil {
		return errors.New("invalid data content nil transaction")
	}
	if len(hashes) != len(programs) {
		return errors.New("The number of data hashes is different with number of programs.")
	}

	for i := 0; i < len(programs); i++ {
		programHash, err := nc.ToProgramHash(programs[i].Code)
		if err != nil {
			return err
		}
		if !hashes[i].ToCodeHash().IsEqual(programHash.ToCodeHash()) {
			return errors.New("The data hashes is different with corresponding program code.")
		}
		//execute program on AVM
		dbCache := blockchain.NewDBCache(blockchain.DefaultChain.Store)
		stateMachine := service.NewStateMachine(dbCache, dbCache)
		se := avm.NewExecutionEngine(tx, new(avm.CryptoECDsa), avm.MAXSTEPS, store.NewCacheCodeTable(dbCache),
			stateMachine, 0, avm.Verification, false)
		se.LoadScript(programs[i].Code, false)
		se.LoadScript(programs[i].Parameter, true)
		se.Execute()

		if se.GetState() != avm.HALT {
			return errors.New("[AVM] Finish State not equal to HALT.")
		}
		
		if se.GetEvaluationStack().Count() != 1 {
			return errors.New("[AVM] Execute Engine Stack Count Error.")
		}

		success := se.GetExecuteResult()
		if !success {
			return errors.New("[AVM] Check Sig FALSE.")
		}
	}

	return nil
}