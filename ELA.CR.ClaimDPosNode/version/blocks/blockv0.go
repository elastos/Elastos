package blocks

import (
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/version/verconf"
)

// Ensure blockV0 implement the BlockVersion interface.
var _ BlockVersion = (*blockV0)(nil)

// blockV0 represent the version 0 block.
type blockV0 struct {
	cfg *verconf.Config
}

func (b *blockV0) GetVersion() uint32 {
	return 0
}

func (b *blockV0) GetNextOnDutyArbitrator(dutyChangedCount, offset uint32) []byte {
	arbitrators, _ := b.GetNormalArbitratorsDesc(0)
	height := b.cfg.ChainStore.GetHeight()
	index := (height + offset) % uint32(len(arbitrators))
	arbitrator := arbitrators[index]

	return arbitrator
}

func (b *blockV0) CheckConfirmedBlockOnFork(block *types.Block) error {
	return nil
}

func (b *blockV0) GetNormalArbitratorsDesc(arbitratorsCount uint32) (
	[][]byte, error) {
	arbitersByte := make([][]byte, 0)
	for _, arbiter := range b.cfg.ChainParams.OriginArbiters {
		arbiterByte, err := common.HexStringToBytes(arbiter)
		if err != nil {
			return nil, err
		}
		arbitersByte = append(arbitersByte, arbiterByte)
	}

	return arbitersByte, nil
}

func (b *blockV0) GetCandidatesDesc(startIndex uint32) ([][]byte, error) {
	return [][]byte{}, nil
}

func (b *blockV0) AddDposBlock(dposBlock *types.DposBlock) (bool, bool, error) {
	return b.cfg.Chain.ProcessBlock(dposBlock.Block)
}

func (b *blockV0) AssignCoinbaseTxRewards(block *types.Block, totalReward common.Fixed64) error {
	// PoW miners and DPoS are each equally allocated 35%. The remaining 30% goes to the Cyber Republic fund
	rewardCyberRepublic := common.Fixed64(float64(totalReward) * 0.3)
	rewardMergeMiner := common.Fixed64(float64(totalReward) * 0.35)
	rewardDposArbiter := common.Fixed64(totalReward) - rewardCyberRepublic - rewardMergeMiner
	block.Transactions[0].Outputs[0].Value = rewardCyberRepublic
	block.Transactions[0].Outputs[1].Value = rewardMergeMiner
	block.Transactions[0].Outputs = append(block.Transactions[0].Outputs, &types.Output{
		AssetID:     config.ELAAssetID,
		Value:       rewardDposArbiter,
		ProgramHash: blockchain.FoundationAddress,
	})

	return nil
}

func NewBlockV0(cfg *verconf.Config) *blockV0 {
	return &blockV0{cfg: cfg}
}
