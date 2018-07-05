package blockchain

import (
	"time"

	"github.com/elastos/Elastos.ELA/config"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

var (
	OriginIssuanceAmount   = 3300 * 10000 * 100000000
	BlockGenerateInterval  = int64(config.Parameters.ChainParam.TargetTimePerBlock / time.Second)
	GeneratedBlocksPerYear = 365 * 24 * 60 * 60 / BlockGenerateInterval
	IssuancePerYear        = func(totalAmount int64) float64 { return float64(totalAmount) * 4 / 100 }
)

func GetBlockRewardAmount(height uint32) common.Fixed64 {
	var totalAmount = int64(OriginIssuanceAmount)
	var rewardPerBlock int64
	for i := uint32(0); i <= height/uint32(GeneratedBlocksPerYear); i++ {
		rewardPerBlock = int64(IssuancePerYear(totalAmount) / float64(GeneratedBlocksPerYear))
		totalAmount += rewardPerBlock * GeneratedBlocksPerYear
	}

	return common.Fixed64(rewardPerBlock)
}
