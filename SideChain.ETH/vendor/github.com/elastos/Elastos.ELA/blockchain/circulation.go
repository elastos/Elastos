package blockchain

import (
	"time"

	"github.com/elastos/Elastos.ELA/config"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

var (
	OriginIssuanceAmount   = 3300 * 10000 * 100000000
	InflationPerYear = OriginIssuanceAmount * 4 / 100
	BlockGenerateInterval  = int64(config.Parameters.ChainParam.TargetTimePerBlock / time.Second)
	GeneratedBlocksPerYear = 365 * 24 * 60 * 60 / BlockGenerateInterval
	RewardAmountPerBlock   = common.Fixed64(float64(InflationPerYear) / float64(GeneratedBlocksPerYear))
)
