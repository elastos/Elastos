package txs

import (
	"errors"
	"math"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/version/verconf"
)

// Ensure txV2 implement the TxVersion interface.
var _ TxVersion = (*txV2)(nil)

// txV2 represent the current transaction version.
type txV2 struct {
	*txV1
}

func (v *txV2) GetVersion() byte {
	return 9
}

func (v *txV2) CheckOutputPayload(txType types.TxType, output *types.Output) error {
	// OTVote information can only be placed in TransferAsset transaction.
	if txType == types.TransferAsset {
		switch output.Type {
		case types.OTNone:
		case types.OTVote:
		default:
			return errors.New("transaction type dose not match the output payload type")
		}
	} else {
		switch output.Type {
		case types.OTNone:
		default:
			return errors.New("transaction type dose not match the output payload type")
		}
	}

	return output.Payload.Validate()
}

func (v *txV2) CheckCoinbaseMinerReward(tx *types.Transaction, totalReward common.Fixed64) error {
	minerReward := tx.Outputs[1].Value
	if common.Fixed64(minerReward) < common.Fixed64(float64(totalReward)*0.35) {
		return errors.New("Reward to miner in coinbase < 35%")
	}

	return nil
}

func (v *txV2) CheckCoinbaseArbitratorsReward(coinbase *types.Transaction, rewardInCoinbase common.Fixed64) error {
	outputAddressMap := make(map[common.Uint168]common.Fixed64)

	for i := 2; i < len(coinbase.Outputs); i++ {
		outputAddressMap[coinbase.Outputs[i].ProgramHash] = coinbase.Outputs[i].Value
	}

	arbitratorsHashes := v.cfg.Arbitrators.GetArbitratorsProgramHashes()
	candidatesHashes := v.cfg.Arbitrators.GetCandidatesProgramHashes()
	if len(arbitratorsHashes)+len(candidatesHashes) != len(coinbase.Outputs)-2 {
		return errors.New("coinbase output count not match")
	}

	dposTotalReward := common.Fixed64(float64(rewardInCoinbase) * 0.35)
	totalBlockConfirmReward := float64(dposTotalReward) * 0.25
	totalTopProducersReward := float64(dposTotalReward) * 0.75
	individualBlockConfirmReward := common.Fixed64(math.Floor(totalBlockConfirmReward / float64(len(arbitratorsHashes))))
	individualProducerReward := common.Fixed64(math.Floor(totalTopProducersReward / float64(int(config.Parameters.ArbiterConfiguration.NormalArbitratorsCount)+len(candidatesHashes))))

	for _, hash := range arbitratorsHashes {

		amount, ok := outputAddressMap[*hash]
		if !ok {
			return errors.New("unknown dpos reward address")
		}

		if v.cfg.Arbitrators.IsCRCArbitratorProgramHash(hash) {
			if amount != individualBlockConfirmReward {
				return errors.New("incorrect dpos reward amount")
			}
		} else {
			if amount != individualProducerReward+individualBlockConfirmReward {
				return errors.New("incorrect dpos reward amount")
			}
		}
	}

	for _, v := range candidatesHashes {

		amount, ok := outputAddressMap[*v]
		if !ok {
			return errors.New("unknown dpos reward address")
		}

		if amount != individualProducerReward {
			return errors.New("incorrect dpos reward amount")
		}
	}

	return nil
}

func (v *txV2) CheckTxHasNoPrograms(tx *types.Transaction) error {
	if len(tx.Programs) != 0 {
		return errors.New("Transaction should have no programs.")
	}

	return nil
}

func NewTxV2(cfg *verconf.Config) *txV2 {
	return &txV2{NewTxV1(cfg)}
}
