package blocks

import (
	"bytes"
	"errors"
	"math"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/version/verconf"
)

// Ensure blockV1 implement the BlockVersion interface.
var _ BlockVersion = (*blockV1)(nil)

// blockV1 represent the current block version.
type blockV1 struct {
	cfg *verconf.Config
}

func (b *blockV1) GetVersion() uint32 {
	return 1
}

func (b *blockV1) GetNextOnDutyArbitrator(dutyChangedCount, offset uint32) []byte {
	arbitrators := b.cfg.Arbitrators.GetArbitrators()
	if len(arbitrators) == 0 {
		return nil
	}
	index := (dutyChangedCount + offset) % uint32(len(arbitrators))
	arbitrator := arbitrators[index]

	return arbitrator
}

func (b *blockV1) CheckConfirmedBlockOnFork(block *types.Block) error {
	if !b.cfg.Server.IsCurrent() {
		return nil
	}

	hash, err := b.cfg.ChainStore.GetBlockHash(block.Height)
	if err != nil {
		return err
	}

	anotherBlock, err := b.cfg.ChainStore.GetBlock(hash)
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

	tx := &types.Transaction{
		Version:        types.TransactionVersion(b.cfg.Versions.GetDefaultTxVersion(block.Height)),
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
	if err := b.cfg.TxMemPool.AppendToTxPool(tx); err == nil {
		err = b.cfg.TxMemPool.AppendToTxPool(tx)
	}

	return nil
}

func (b *blockV1) GetNormalArbitratorsDesc() ([][]byte, error) {
	return [][]byte{}, nil
}

func (b *blockV1) GetCandidatesDesc() ([][]byte, error) {
	return [][]byte{}, nil
}

func (b *blockV1) AddDposBlock(dposBlock *types.DposBlock) (bool, bool, error) {
	return b.cfg.BlockMemPool.AppendDposBlock(dposBlock)
}

func (b *blockV1) AssignCoinbaseTxRewards(block *types.Block, totalReward common.Fixed64) error {
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

func (b *blockV1) distributeDposReward(coinBaseTx *types.Transaction, reward common.Fixed64) (common.Fixed64, error) {
	arbitratorsHashes := b.cfg.Arbitrators.GetArbitratorsProgramHashes()
	if uint32(len(arbitratorsHashes)) < blockchain.DefaultLedger.Arbitrators.GetArbitersCount() {
		return 0, errors.New("current arbitrators count less than required arbitrators count")
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
			AssetID:       config.ELAAssetID,
			Value:         reward,
			ProgramHash:   *v,
			OutputType:    types.DefaultOutput,
			OutputPayload: &outputpayload.DefaultOutput{},
		})

		realDposReward += reward
	}

	for _, v := range candidatesHashes {

		coinBaseTx.Outputs = append(coinBaseTx.Outputs, &types.Output{
			AssetID:       config.ELAAssetID,
			Value:         individualProducerReward,
			ProgramHash:   *v,
			OutputType:    types.DefaultOutput,
			OutputPayload: &outputpayload.DefaultOutput{},
		})

		realDposReward += individualProducerReward
	}

	change := reward - realDposReward
	if change < 0 {
		return 0, errors.New("Real dpos reward more than reward limit.")
	}
	return change, nil
}

func (b *blockV1) generateBlockEvidence(block *types.Block) (*types.BlockEvidence, error) {
	headerBuf := new(bytes.Buffer)
	if err := block.Header.Serialize(headerBuf); err != nil {
		return nil, err
	}

	confirm, err := b.cfg.ChainStore.GetConfirm(block.Hash())
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

func (b *blockV1) getConfirmSigners(confirm *types.DPosProposalVoteSlot) ([][]byte, error) {
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

func NewBlockV1(cfg *verconf.Config) *blockV1 {
	return &blockV1{cfg: cfg}
}
