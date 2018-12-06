package version

import (
	"bytes"
	"errors"
	"math"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	errors2 "github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/node"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type BlockVersion interface {
	GetVersion() uint32
	GetProducersDesc() ([][]byte, error)
	AddBlock(block *types.Block) error
	AddBlockConfirm(block *types.BlockConfirm) (bool, error)
	AssignCoinbaseTxRewards(block *types.Block, totalReward common.Fixed64) error
	CheckConfirmedBlockOnFork(block *types.Block) error
	GetNextOnDutyArbitrator(dutyChangedCount, offset uint32) []byte
}

type BlockVersionMain struct {
}

func (b *BlockVersionMain) GetVersion() uint32 {
	return 1
}

func (b *BlockVersionMain) GetNextOnDutyArbitrator(dutyChangedCount, offset uint32) []byte {
	arbitrators := blockchain.DefaultLedger.Arbitrators.GetArbitrators()
	index := (dutyChangedCount + offset) % uint32(len(arbitrators))
	arbitrator := arbitrators[index]

	return arbitrator
}

func (b *BlockVersionMain) CheckConfirmedBlockOnFork(block *types.Block) error {
	if !node.LocalNode.IsCurrent() {
		return nil
	}

	anotherBlock, err := blockchain.DefaultLedger.GetBlockWithHeight(block.Height)
	if err != nil {
		return err
	}

	if block.Hash().IsEqual(anotherBlock.Hash()) {
		return nil
	}

	evidence, err := b.generateBlockEvidence(block)
	if err != nil {
		return err
	}

	compareEvidence, err := b.generateBlockEvidence(anotherBlock)
	if err != nil {
		return err
	}

	illegalBlocks := &types.PayloadIllegalBlock{
		DposIllegalBlocks: types.DposIllegalBlocks{
			CoinType:        types.ELACoin,
			BlockHeight:     block.Height,
			Evidence:        *evidence,
			CompareEvidence: *compareEvidence,
		},
	}

	if err := blockchain.CheckDposIllegalBlocks(&illegalBlocks.DposIllegalBlocks); err != nil {
		return err
	}

	txn := &types.Transaction{
		Version:        types.TransactionVersion(blockchain.DefaultLedger.HeightVersions.GetDefaultTxVersion(block.Height)),
		TxType:         types.IllegalBlockEvidence,
		PayloadVersion: types.PayloadIllegalBlockVersion,
		Payload:        illegalBlocks,
		Attributes:     []*types.Attribute{},
		LockTime:       0,
		Programs:       []*program.Program{},
		Outputs:        []*types.Output{},
		Inputs:         []*types.Input{},
		Fee:            0,
	}
	if code := node.LocalNode.AppendToTxnPool(txn); code == errors2.Success {
		node.LocalNode.AppendToTxnPool(txn)
	}

	return nil
}

func (b *BlockVersionMain) generateBlockEvidence(block *types.Block) (*types.BlockEvidence, error) {
	headerBuf := new(bytes.Buffer)
	if err := block.Header.Serialize(headerBuf); err != nil {
		return nil, err
	}

	confirm, err := blockchain.DefaultLedger.Store.GetConfirm(block.Hash())
	if err != nil {
		return nil, err
	}
	confirmBuf := new(bytes.Buffer)
	if err = confirm.Serialize(confirmBuf); err != nil {
		return nil, err
	}
	confirmSigners, err := b.getConfirmSigners(confirm)
	if err != nil {
		return nil, err
	}

	return &types.BlockEvidence{
		Block:        headerBuf.Bytes(),
		BlockConfirm: confirmBuf.Bytes(),
		Signers:      confirmSigners,
	}, nil
}

func (b *BlockVersionMain) getConfirmSigners(confirm *types.DPosProposalVoteSlot) ([][]byte, error) {
	result := make([][]byte, 0)
	for _, v := range confirm.Votes {
		data, err := common.HexStringToBytes(v.Signer)
		if err != nil {
			return nil, err
		}
		result = append(result, data)
	}
	return result, nil
}

func (b *BlockVersionMain) GetProducersDesc() ([][]byte, error) {
	producersInfo := blockchain.DefaultLedger.Store.GetRegisteredProducers()
	if uint32(len(producersInfo)) < config.Parameters.ArbiterConfiguration.ArbitratorsCount {
		return nil, errors.New("producers count less than min arbitrators count.")
	}

	result := make([][]byte, 0)
	for i := uint32(0); i < uint32(len(producersInfo)); i++ {
		result = append(result, producersInfo[i].PublicKey)
	}
	return result, nil
}

func (b *BlockVersionMain) AddBlock(block *types.Block) error {
	if _, err := node.LocalNode.AppendBlock(&types.BlockConfirm{
		BlockFlag: true,
		Block:     block,
	}); err != nil {
		log.Error("[AddBlock] err:", err.Error())
		return err
	}

	return nil
}

func (b *BlockVersionMain) AddBlockConfirm(blockConfirm *types.BlockConfirm) (bool, error) {
	isConfirmed, err := node.LocalNode.AppendBlock(blockConfirm)
	if err != nil {
		log.Error("[AddBlockConfirm] err:", err.Error())
		return false, err
	}

	return isConfirmed, nil
}

func (b *BlockVersionMain) AssignCoinbaseTxRewards(block *types.Block, totalReward common.Fixed64) error {
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

func (b *BlockVersionMain) distributeDposReward(coinBaseTx *types.Transaction, reward common.Fixed64) (common.Fixed64, error) {
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

		coinBaseTx.Outputs = append(coinBaseTx.Outputs, &types.Output{
			AssetID:       blockchain.DefaultLedger.Blockchain.AssetID,
			Value:         individualBlockConfirmReward + individualProducerReward,
			ProgramHash:   *v,
			OutputType:    types.DefaultOutput,
			OutputPayload: &outputpayload.DefaultOutput{},
		})

		realDposReward += individualBlockConfirmReward + individualProducerReward
	}

	for _, v := range candidatesHashes {

		coinBaseTx.Outputs = append(coinBaseTx.Outputs, &types.Output{
			AssetID:       blockchain.DefaultLedger.Blockchain.AssetID,
			Value:         individualProducerReward,
			ProgramHash:   *v,
			OutputType:    types.DefaultOutput,
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
