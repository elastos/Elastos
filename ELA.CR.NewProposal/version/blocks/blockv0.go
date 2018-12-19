package blocks

import (
	"errors"
	"github.com/elastos/Elastos.ELA/version/verconf"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
)

var originalArbitrators = []string{
	"0248df6705a909432be041e0baa25b8f648741018f70d1911f2ed28778db4b8fe4",
	"02771faf0f4d4235744b30972d5f2c470993920846c761e4d08889ecfdc061cddf",
	"0342196610e57d75ba3afa26e030092020aec56822104e465cba1d8f69f8d83c8e",
	"02fa3e0d14e0e93ca41c3c0f008679e417cf2adb6375dd4bbbee9ed8e8db606a56",
	"03ab3ecd1148b018d480224520917c6c3663a3631f198e3b25cf4c9c76786b7850",
}

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
	arbitrators, _ := b.GetProducersDesc()
	height := b.cfg.ChainStore.GetHeight()
	index := (height + offset) % uint32(len(arbitrators))
	arbitrator := arbitrators[index]

	return arbitrator
}

func (b *blockV0) CheckConfirmedBlockOnFork(block *types.Block) error {
	return nil
}

func (b *blockV0) GetProducersDesc() ([][]byte, error) {
	if len(originalArbitrators) == 0 {
		return nil, errors.New("arbiters not configured")
	}

	arbitersByte := make([][]byte, 0)
	for _, arbiter := range originalArbitrators {
		arbiterByte, err := common.HexStringToBytes(arbiter)
		if err != nil {
			return nil, err
		}
		arbitersByte = append(arbitersByte, arbiterByte)
	}

	return arbitersByte, nil
}

func (b *blockV0) AddDposBlock(dposBlock *types.DposBlock) (bool, bool, error) {
	return b.cfg.Chain.AddBlock(dposBlock.Block)
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
