package blocks

import (
	"errors"
	"math"
	"sort"

	"github.com/elastos/Elastos.ELA/blockchain/interfaces"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/version/verconf"
)

// Ensure blockV2 implement the BlockVersion interface.
var _ BlockVersion = (*blockV2)(nil)

// blockV2 represent the current block version.
type blockV2 struct {
	*blockV1
}

func (b *blockV2) GetVersion() uint32 {
	return 2
}

func (b *blockV2) GetNormalArbitratorsDesc(arbitratorsCount uint32, producers []interfaces.Producer) ([][]byte,
	error) {
	if uint32(len(producers)) < arbitratorsCount/2+1 {
		return nil, errors.New("producers count less than min arbitrators count")
	}

	sort.Slice(producers, func(i, j int) bool {
		return producers[i].Votes() > producers[j].Votes()
	})

	result := make([][]byte, 0)
	for i := uint32(0); i < arbitratorsCount && i < uint32(len(producers)); i++ {
		result = append(result, producers[i].NodePublicKey())
	}
	return result, nil
}

func (b *blockV2) AssignCoinbaseTxRewards(block *types.Block, totalReward common.Fixed64) error {
	rewardCyberRepublic := common.Fixed64(math.Ceil(float64(totalReward) * 0.3))
	rewardDposArbiter := common.Fixed64(float64(totalReward) * 0.35)

	var dposChange common.Fixed64
	var err error
	if dposChange, err = b.distributeDposReward(block.Transactions[0], rewardDposArbiter); err != nil {
		return err
	}
	rewardMergeMiner := common.Fixed64(totalReward) - rewardCyberRepublic - rewardDposArbiter + dposChange
	block.Transactions[0].Outputs[0].Value = rewardCyberRepublic
	block.Transactions[0].Outputs[1].Value = rewardMergeMiner
	return nil
}

func (b *blockV2) distributeDposReward(coinBaseTx *types.Transaction, reward common.Fixed64) (common.Fixed64, error) {
	arbitratorsHashes :=
		b.cfg.Arbitrators.GetArbitratorsProgramHashes()
	if len(arbitratorsHashes) == 0 {
		return 0, errors.New("not found arbiters when distributeDposReward")
	}
	candidatesHashes := b.cfg.Arbitrators.GetCandidatesProgramHashes()

	totalBlockConfirmReward := float64(reward) * 0.25
	totalTopProducersReward := float64(reward) * 0.75
	individualBlockConfirmReward := common.Fixed64(math.Floor(totalBlockConfirmReward / float64(len(arbitratorsHashes))))
	individualProducerReward := common.Fixed64(math.Floor(totalTopProducersReward / float64(int(config.Parameters.ArbiterConfiguration.NormalArbitratorsCount)+len(candidatesHashes))))

	realDposReward := common.Fixed64(0)
	for _, v := range arbitratorsHashes {
		reward := individualBlockConfirmReward + individualProducerReward
		if b.cfg.Arbitrators.IsCRCArbitratorProgramHash(v) {
			reward = individualBlockConfirmReward
		}

		coinBaseTx.Outputs = append(coinBaseTx.Outputs, &types.Output{
			AssetID:     config.ELAAssetID,
			Value:       reward,
			ProgramHash: *v,
			Type:        types.OTNone,
			Payload:     &outputpayload.DefaultOutput{},
		})

		realDposReward += reward
	}

	for _, v := range candidatesHashes {

		coinBaseTx.Outputs = append(coinBaseTx.Outputs, &types.Output{
			AssetID:     config.ELAAssetID,
			Value:       individualProducerReward,
			ProgramHash: *v,
			Type:        types.OTNone,
			Payload:     &outputpayload.DefaultOutput{},
		})

		realDposReward += individualProducerReward
	}

	change := reward - realDposReward
	if change < 0 {
		return 0, errors.New("real dpos reward more than reward limit")
	}
	return change, nil
}

func NewBlockV2(cfg *verconf.Config) *blockV2 {
	return &blockV2{NewBlockV1(cfg)}
}
