package state

import (
	"bytes"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/utils/test"

	"github.com/stretchr/testify/assert"
)

var arbiters *Arbitrators
var arbitratorList [][]byte

func TestHeightVersionInit(t *testing.T) {
	test.SkipShort(t)
	config.Parameters = config.ConfigParams{Configuration: &config.Template}
	log.NewDefault(
		config.Parameters.PrintLevel,
		config.Parameters.MaxPerLogSize,
		config.Parameters.MaxLogsSize,
	)

	arbitratorsStr := []string{
		"023a133480176214f88848c6eaa684a54b316849df2b8570b57f3a917f19bbc77a",
		"030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9",
		"0288e79636e41edce04d4fa95d8f62fed73a76164f8631ccc42f5425f960e4a0c7",
		"03e281f89d85b3a7de177c240c4961cb5b1f2106f09daa42d15874a38bbeae85dd",
		"0393e823c2087ed30871cbea9fa5121fa932550821e9f3b17acef0e581971efab0",
	}
	config.Parameters.ArbiterConfiguration.CRCArbiters = []config.CRCArbitratorConfigItem{
		{PublicKey: "023a133480176214f88848c6eaa684a54b316849df2b8570b57f3a917f19bbc77a"},
		{PublicKey: "030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9"},
		{PublicKey: "0288e79636e41edce04d4fa95d8f62fed73a76164f8631ccc42f5425f960e4a0c7"},
		{PublicKey: "03e281f89d85b3a7de177c240c4961cb5b1f2106f09daa42d15874a38bbeae85dd"},
		{PublicKey: "0393e823c2087ed30871cbea9fa5121fa932550821e9f3b17acef0e581971efab0"},
	}

	arbitratorList = make([][]byte, 0)
	for _, v := range arbitratorsStr {
		a, _ := common.HexStringToBytes(v)
		arbitratorList = append(arbitratorList, a)
	}

	activeNetParams := &config.MainNetParams
	var err error
	arbiters, err = NewArbitrators(&ArbitratorsConfig{
		ArbitratorsCount: config.Parameters.ArbiterConfiguration.
			NormalArbitratorsCount + uint32(len(activeNetParams.CRCArbiters)),
		CandidatesCount: config.Parameters.ArbiterConfiguration.CandidatesCount,
		CRCArbitrators:  activeNetParams.CRCArbiters,
		OriginArbiters:  activeNetParams.OriginArbiters,
		GetBestHeight: func() uint32 {
			return 0
		},
	})
	assert.NoError(t, err)
	arbiters.State = NewState(arbiters, activeNetParams)

}

func TestArbitrators_GetNormalArbitratorsDescV0(t *testing.T) {
	test.SkipShort(t)
	arbitrators := make([][]byte, 0)
	for _, v := range config.MainNetParams.OriginArbiters {
		a, _ := common.HexStringToBytes(v)
		arbitrators = append(arbitrators, a)
	}

	// V0
	producers, err := arbiters.GetNormalArbitratorsDesc(0, 5, arbiters.State.getProducers())
	assert.NoError(t, err)
	for i := range producers {
		assert.Equal(t, arbitrators[i], producers[i])
	}
}

func TestArbitrators_GetNormalArbitratorsDesc(t *testing.T) {

	test.SkipShort(t)
	currentHeight := uint32(1)
	block1 := &types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			{
				TxType: types.RegisterProducer,
				Payload: &payload.ProducerInfo{
					OwnerPublicKey: arbitratorList[0],
					NodePublicKey:  arbitratorList[0],
				},
			},
			{
				TxType: types.RegisterProducer,
				Payload: &payload.ProducerInfo{
					OwnerPublicKey: arbitratorList[1],
					NodePublicKey:  arbitratorList[1],
				},
			},
			{
				TxType: types.RegisterProducer,
				Payload: &payload.ProducerInfo{
					OwnerPublicKey: arbitratorList[2],
					NodePublicKey:  arbitratorList[2],
				},
			},
			{
				TxType: types.RegisterProducer,
				Payload: &payload.ProducerInfo{
					OwnerPublicKey: arbitratorList[3],
					NodePublicKey:  arbitratorList[3],
				},
			},
		},
	}
	arbiters.ProcessBlock(block1, nil)

	for i := uint32(0); i < 6; i++ {
		currentHeight++
		blockEx := &types.Block{
			Header:       types.Header{Height: currentHeight},
			Transactions: []*types.Transaction{},
		}
		arbiters.ProcessBlock(blockEx, nil)
	}

	// main version
	producers, err := arbiters.GetNormalArbitratorsDesc(arbiters.State.chainParams.HeightVersions[3], 10, arbiters.State.getProducers())
	assert.Error(t, err, "arbitrators count does not match config value")

	currentHeight += 1
	block2 := &types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			{
				TxType: types.RegisterProducer,
				Payload: &payload.ProducerInfo{
					OwnerPublicKey: arbitratorList[4],
					NodePublicKey:  arbitratorList[4],
				},
			},
		},
	}
	arbiters.ProcessBlock(block2, nil)

	for i := uint32(0); i < 6; i++ {
		currentHeight++
		blockEx := &types.Block{
			Header:       types.Header{Height: currentHeight},
			Transactions: []*types.Transaction{},
		}
		arbiters.ProcessBlock(blockEx, nil)
	}

	// main version
	producers, err = arbiters.GetNormalArbitratorsDesc(arbiters.State.chainParams.HeightVersions[3], 5, arbiters.State.getProducers())
	assert.NoError(t, err)
	for i := range producers {
		found := false
		for _, ar := range arbitratorList {
			if bytes.Equal(ar, producers[i]) {
				found = true
				break
			}
		}

		assert.True(t, found)
	}
}
