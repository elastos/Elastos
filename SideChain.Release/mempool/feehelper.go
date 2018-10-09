package mempool

import (
	"bytes"
	"errors"
	"github.com/elastos/Elastos.ELA.SideChain/blockchain"

	"github.com/elastos/Elastos.ELA.SideChain/types"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/core"
)

type FeeHelper struct {
	exchangeRate float64
	chainStore   *blockchain.ChainStore
}

func NewFeeHelper(cfg *Config) *FeeHelper {
	return &FeeHelper{
		chainStore:   cfg.ChainStore,
		exchangeRate: cfg.ExchangeRage,
	}
}

func (h *FeeHelper) GetTxFee(tx *types.Transaction, assetId common.Uint256) common.Fixed64 {
	feeMap, err := h.GetTxFeeMap(tx)
	if err != nil {
		return 0
	}

	return feeMap[assetId]
}

func (h *FeeHelper) GetTxFeeMap(tx *types.Transaction) (map[common.Uint256]common.Fixed64, error) {
	feeMap := make(map[common.Uint256]common.Fixed64)

	if tx.IsRechargeToSideChainTx() {
		depositPayload := tx.Payload.(*types.PayloadRechargeToSideChain)
		mainChainTransaction := new(core.Transaction)
		reader := bytes.NewReader(depositPayload.MainChainTransaction)
		if err := mainChainTransaction.Deserialize(reader); err != nil {
			return nil, errors.New("GetTxFeeMap mainChainTransaction deserialize failed")
		}

		crossChainPayload := mainChainTransaction.Payload.(*core.PayloadTransferCrossChainAsset)

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
						feeMap[v.AssetID] = amount + common.Fixed64(float64(mcAmount)*h.exchangeRate) - v.Value
					} else {
						feeMap[v.AssetID] = common.Fixed64(float64(mcAmount)*h.exchangeRate) - v.Value
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
