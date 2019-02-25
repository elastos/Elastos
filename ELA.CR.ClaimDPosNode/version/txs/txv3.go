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
var _ TxVersion = (*txV3)(nil)

// txV2 represent the current transaction version.
type txV3 struct {
	*txV2
}

func (v *txV3) GetVersion() byte {
	return 10
}

func (v *txV2) CheckCoinbaseMinerReward(tx *types.Transaction,
	totalReward common.Fixed64) error {
	minerReward := tx.Outputs[1].Value
	if common.Fixed64(minerReward) < common.Fixed64(float64(totalReward)*0.35) {
		return errors.New("reward to miner in coinbase < 35%")
	}

	return nil
}

func (v *txV2) CheckCoinbaseArbitratorsReward(coinbase *types.Transaction,
	rewardInCoinbase common.Fixed64) error {
	outputAddressMap := make(map[common.Uint168]common.Fixed64)

	for i := 2; i < len(coinbase.Outputs); i++ {
		outputAddressMap[coinbase.Outputs[i].ProgramHash] = coinbase.Outputs[i].Value
	}

	arbitratorsHashes :=
		v.cfg.Arbitrators.GetArbitratorsProgramHashes()
	candidatesHashes := v.cfg.Arbitrators.GetCandidatesProgramHashes()
	if len(arbitratorsHashes)+len(candidatesHashes) != len(coinbase.Outputs)-2 {
		return errors.New("coinbase output count not match")
	}

	dposTotalReward := common.Fixed64(float64(rewardInCoinbase) * 0.35)
	totalBlockConfirmReward := float64(dposTotalReward) * 0.25
	totalTopProducersReward := float64(dposTotalReward) * 0.75
	individualBlockConfirmReward := common.Fixed64(
		math.Floor(totalBlockConfirmReward / float64(len(arbitratorsHashes))))
	individualProducerReward := common.Fixed64(math.Floor(
		totalTopProducersReward / float64(int(config.Parameters.
			ArbiterConfiguration.NormalArbitratorsCount) + len(candidatesHashes))))

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

func NewTxV3(cfg *verconf.Config) *txV3 {
	return &txV3{NewTxV2(cfg)}
}
