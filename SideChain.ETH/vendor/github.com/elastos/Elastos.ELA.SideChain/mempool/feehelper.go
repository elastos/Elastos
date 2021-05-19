package mempool

import (
	"bytes"
	"errors"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/spv"
	"github.com/elastos/Elastos.ELA.SideChain/types"

	"github.com/elastos/Elastos.ELA/common"
	ela "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
)

type FeeHelper struct {
	chainParams *config.Params
	chainStore  *blockchain.ChainStore
	spvService  *spv.Service
}

func NewFeeHelper(cfg *Config) *FeeHelper {
	return &FeeHelper{
		chainStore:  cfg.ChainStore,
		chainParams: cfg.ChainParams,
		spvService:  cfg.SpvService,
	}
}

func (h *FeeHelper) GetTxFee(tx *types.Transaction, assetId common.Uint256) (common.Fixed64, error) {
	feeMap, err := h.GetTxFeeMap(tx)
	if err != nil {
		return 0, err
	}

	return feeMap[assetId], nil
}

func (h *FeeHelper) GetTxFeeMap(tx *types.Transaction) (map[common.Uint256]common.Fixed64, error) {
	feeMap := make(map[common.Uint256]common.Fixed64)

	if tx.IsRechargeToSideChainTx() {
		var mainChainTransaction *ela.Transaction
		depositPayload, ok := tx.Payload.(*types.PayloadRechargeToSideChain)
		if !ok {
			return nil, errors.New("invalid recharge to side chain transaction payload")
		}
		if tx.PayloadVersion == types.RechargeToSideChainPayloadVersion0 {
			mainChainTransaction = new(ela.Transaction)
			reader := bytes.NewReader(depositPayload.MainChainTransaction)
			if err := mainChainTransaction.Deserialize(reader); err != nil {
				return nil, errors.New("main chain transaction deserialize failed")
			}
		} else if tx.PayloadVersion == types.RechargeToSideChainPayloadVersion1 {
			var err error
			mainChainTransaction, err = h.spvService.GetTransaction(&depositPayload.MainChainTransactionHash)
			if err != nil {
				return nil, errors.New("invalid main chain transaction payload")
			}
		}

		crossChainPayload, ok := mainChainTransaction.Payload.(*payload.TransferCrossChainAsset)
		if !ok {
			return nil, errors.New("invalid transfer cross chain asset transaction payload")
		}
		for _, v := range tx.Outputs {
			for i := 0; i < len(crossChainPayload.CrossChainAddresses); i++ {
				targetAddress, err := v.ProgramHash.ToAddress()
				if err != nil {
					return nil, err
				}
				if targetAddress == crossChainPayload.CrossChainAddresses[i] {
					mcAmount := mainChainTransaction.Outputs[crossChainPayload.OutputIndexes[i]].Value

					amount, ok := feeMap[v.AssetID]
					if ok {
						feeMap[v.AssetID] = amount + common.Fixed64(
							float64(mcAmount)*h.chainParams.ExchangeRate) - v.Value
					} else {
						feeMap[v.AssetID] = common.Fixed64(float64(
							mcAmount)*h.chainParams.ExchangeRate) - v.Value
					}
				}
			}
		}

		return feeMap, nil
	}

	reference, err := h.chainStore.GetTxReference(tx)
	if err != nil {
		return nil, err
	}

	var inputs = make(map[common.Uint256]common.Fixed64)
	var outputs = make(map[common.Uint256]common.Fixed64)
	for _, v := range reference {
		amout, ok := inputs[v.AssetID]
		if ok {
			inputs[v.AssetID] = amout + v.Value
		} else {
			inputs[v.AssetID] = v.Value
		}
	}

	for _, v := range tx.Outputs {
		amout, ok := outputs[v.AssetID]
		if ok {
			outputs[v.AssetID] = amout + v.Value
		} else {
			outputs[v.AssetID] = v.Value
		}
	}

	//calc the balance of input vs output
	for outputAssetid, outputValue := range outputs {
		if inputValue, ok := inputs[outputAssetid]; ok {
			feeMap[outputAssetid] = inputValue - outputValue
		} else {
			feeMap[outputAssetid] -= outputValue
		}
	}
	for inputAssetid, inputValue := range inputs {
		if _, exist := feeMap[inputAssetid]; !exist {
			feeMap[inputAssetid] += inputValue
		}
	}
	return feeMap, nil
}
