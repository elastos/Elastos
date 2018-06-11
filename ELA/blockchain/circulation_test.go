package blockchain

import (
	"testing"
	"time"

	"github.com/elastos/Elastos.ELA/config"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/stretchr/testify/assert"
)

var (
	TargetTimePerBlock = int64(config.Parameters.ChainParam.TargetTimePerBlock / time.Second)

	OrginAmountOfEla = 3300 * 10000 * 100000000
	SubsidyInterval  = 365 * 24 * 60 * 60 / TargetTimePerBlock
	RetargetPersent  = 25
)

func TestGetBlockRewardAmount(t *testing.T) {
	var blocks uint32 = 10000 * 10000
	for i := uint32(0); i <= blocks; i++ {
		if !assert.Equal(t, GetBlockRewardAmount(i), calcBlockSubsidy(i)) {
			break
		}
	}
	t.Logf("Circulation test finished with %d blocks", blocks)
}

func calcBlockSubsidy(currentHeight uint32) common.Fixed64 {
	ToTalAmountOfEla := int64(OrginAmountOfEla)
	for i := uint32(0); i < (currentHeight / uint32(SubsidyInterval)); i++ {
		incr := float64(ToTalAmountOfEla) / float64(RetargetPersent)
		subsidyPerBlock := int64(float64(incr) / float64(SubsidyInterval))
		ToTalAmountOfEla += subsidyPerBlock * int64(SubsidyInterval)
	}
	incr := float64(ToTalAmountOfEla) / float64(RetargetPersent)
	subsidyPerBlock := common.Fixed64(float64(incr) / float64(SubsidyInterval))

	return subsidyPerBlock
}
