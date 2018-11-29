package version

import (
	"errors"
	"math"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/core/outputpayload"
	"github.com/elastos/Elastos.ELA/node"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type BlockVersion interface {
	GetVersion() uint32
	GetProducersDesc() ([][]byte, error)
	AddBlock(block *core.Block) error
	AddBlockConfirm(block *core.BlockConfirm) (bool, error)
	AssignCoinbaseTxRewards(block *core.Block, totalReward common.Fixed64) error
}

type BlockVersionMain struct {
}

func (b *BlockVersionMain) GetVersion() uint32 {
	return 1
}

func (b *BlockVersionMain) GetProducersDesc() ([][]byte, error) {
	producersInfo := blockchain.DefaultLedger.Store.GetRegisteredProducers()
	if uint32(len(producersInfo)) < config.Parameters.ArbiterConfiguration.ArbitratorsCount {
		return nil, errors.New("producers count less than min arbitrators count.")
	}

	result := make([][]byte, 0)
	for i := uint32(0); i < uint32(len(producersInfo)); i++ {
		arbiterByte, err := common.HexStringToBytes(producersInfo[i].PublicKey)
		if err != nil {
			return nil, err
		}
		result = append(result, arbiterByte)
	}
	return result, nil
}

func (b *BlockVersionMain) AddBlock(block *core.Block) error {
	if _, err := node.LocalNode.AppendBlock(&core.BlockConfirm{
		BlockFlag: true,
		Block:     block,
	}); err != nil {
		log.Error("[AppendBlock] err:", err.Error())
		return err
	}

	return nil
}

func (b *BlockVersionMain) AddBlockConfirm(blockConfirm *core.BlockConfirm) (bool, error) {
	isConfirmed, err := node.LocalNode.AppendBlock(blockConfirm)
	if err != nil {
		log.Error("[AppendBlock] err:", err.Error())
		return false, err
	}

	return isConfirmed, nil
}

func (b *BlockVersionMain) AssignCoinbaseTxRewards(block *core.Block, totalReward common.Fixed64) error {
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

func (b *BlockVersionMain) distributeDposReward(coinBaseTx *core.Transaction, reward common.Fixed64) (common.Fixed64, error) {
	arbitratorsHashes := blockchain.DefaultLedger.Arbitrators.GetArbitratorsProgramHashes()
	if uint32(len(arbitratorsHashes)) < config.Parameters.ArbiterConfiguration.ArbitratorsCount {
		return 0, errors.New("Current arbitrators count less than required arbitrators count.")
	}
	candidatesHashes := blockchain.DefaultLedger.Arbitrators.GetCandidatesProgramHashes()

	totalBlockConfirmReward := float64(reward) * 0.25
	totalTopProducersReward := float64(reward) * 0.75
	individualBlockConfirmReward := common.Fixed64(math.Floor(totalBlockConfirmReward / float64(len(arbitratorsHashes))))
	individualProducerReward := common.Fixed64(math.Floor(totalTopProducersReward / float64(len(arbitratorsHashes)+len(candidatesHashes))))

	realDposReward := common.Fixed64(0)
	for _, v := range arbitratorsHashes {

		coinBaseTx.Outputs = append(coinBaseTx.Outputs, &core.Output{
			AssetID:       blockchain.DefaultLedger.Blockchain.AssetID,
			Value:         individualBlockConfirmReward + individualProducerReward,
			ProgramHash:   *v,
			OutputType:    core.DefaultOutput,
			OutputPayload: &outputpayload.DefaultOutput{},
		})

		realDposReward += individualBlockConfirmReward + individualProducerReward
	}

	for _, v := range candidatesHashes {

		coinBaseTx.Outputs = append(coinBaseTx.Outputs, &core.Output{
			AssetID:       blockchain.DefaultLedger.Blockchain.AssetID,
			Value:         individualProducerReward,
			ProgramHash:   *v,
			OutputType:    core.DefaultOutput,
			OutputPayload: &outputpayload.DefaultOutput{},
		})

		realDposReward += individualBlockConfirmReward
	}

	change := reward - realDposReward
	if change < 0 {
		return 0, errors.New("Real dpos reward more than reward limit.")
	}
	return change, nil
}
