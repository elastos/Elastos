package blocks

import (
	"testing"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/blockchain/mock"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/dpos/state"
	"github.com/elastos/Elastos.ELA/version/verconf"

	"github.com/stretchr/testify/assert"
)

var version BlockVersion
var cfg *verconf.Config
var arbitratorsMock *mock.ArbitratorsMock
var arbitrators [][]byte
var originLedger *blockchain.Ledger

func TestBlockVersionInit(t *testing.T) {
	config.Parameters = config.ConfigParams{Configuration: &config.Template}
	log.NewDefault(
		config.Parameters.PrintLevel,
		config.Parameters.MaxPerLogSize,
		config.Parameters.MaxLogsSize,
	)

	chainStore, err := blockchain.NewChainStore("blockVersionTestSuite",
		config.DefaultParams.GenesisBlock)
	if err != nil {
		t.Error(err)
	}

	arbitratorsStr := []string{
		"023a133480176214f88848c6eaa684a54b316849df2b8570b57f3a917f19bbc77a",
		"030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9",
		"0288e79636e41edce04d4fa95d8f62fed73a76164f8631ccc42f5425f960e4a0c7",
		"03e281f89d85b3a7de177c240c4961cb5b1f2106f09daa42d15874a38bbeae85dd",
		"0393e823c2087ed30871cbea9fa5121fa932550821e9f3b17acef0e581971efab0",
	}
	config.Parameters.ArbiterConfiguration.CRCArbiters = []config.CRCArbiterInfo{
		{PublicKey: "023a133480176214f88848c6eaa684a54b316849df2b8570b57f3a917f19bbc77a"},
		{PublicKey: "030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9"},
		{PublicKey: "0288e79636e41edce04d4fa95d8f62fed73a76164f8631ccc42f5425f960e4a0c7"},
		{PublicKey: "03e281f89d85b3a7de177c240c4961cb5b1f2106f09daa42d15874a38bbeae85dd"},
		{PublicKey: "0393e823c2087ed30871cbea9fa5121fa932550821e9f3b17acef0e581971efab0"},
	}

	arbitrators = make([][]byte, 0)
	for _, v := range arbitratorsStr {
		a, _ := common.HexStringToBytes(v)
		arbitrators = append(arbitrators, a)
	}
	arbitratorsMock = &mock.ArbitratorsMock{
		CurrentArbitrators: arbitrators,
	}

	chain, err := blockchain.New(chainStore, &config.DefaultParams,
		nil, state.NewState(arbitratorsMock, &config.DefaultParams))
	if err != nil {
		t.Error(err)
	}

	cfg = &verconf.Config{
		Chain:       chain,
		Arbitrators: arbitratorsMock,
	}
	version = NewBlockV2(cfg)

	originLedger = blockchain.DefaultLedger
	blockchain.DefaultLedger = &blockchain.Ledger{
		Arbitrators: arbitratorsMock,
		Blockchain:  &blockchain.BlockChain{},
	}
}

func TestBlockVersionMain_GetNextOnDutyArbitrator(t *testing.T) {
	var currentArbitrator []byte

	currentArbitrator = version.GetNextOnDutyArbitrator(0, 0)
	assert.Equal(t, arbitrators[0], currentArbitrator)

	currentArbitrator = version.GetNextOnDutyArbitrator(1, 0)
	assert.Equal(t, arbitrators[1], currentArbitrator)

	currentArbitrator = version.GetNextOnDutyArbitrator(2, 0)
	assert.Equal(t, arbitrators[2], currentArbitrator)

	currentArbitrator = version.GetNextOnDutyArbitrator(3, 0)
	assert.Equal(t, arbitrators[3], currentArbitrator)

	currentArbitrator = version.GetNextOnDutyArbitrator(4, 0)
	assert.Equal(t, arbitrators[4], currentArbitrator)

	currentArbitrator = version.GetNextOnDutyArbitrator(5, 0)
	assert.Equal(t, arbitrators[0], currentArbitrator)

	currentArbitrator = version.GetNextOnDutyArbitrator(0, 1)
	assert.Equal(t, arbitrators[1], currentArbitrator)

	currentArbitrator = version.GetNextOnDutyArbitrator(0, 2)
	assert.Equal(t, arbitrators[2], currentArbitrator)

	currentArbitrator = version.GetNextOnDutyArbitrator(0, 3)
	assert.Equal(t, arbitrators[3], currentArbitrator)

	currentArbitrator = version.GetNextOnDutyArbitrator(0, 4)
	assert.Equal(t, arbitrators[4], currentArbitrator)

	currentArbitrator = version.GetNextOnDutyArbitrator(0, 5)
	assert.Equal(t, arbitrators[0], currentArbitrator)

	currentArbitrator = version.GetNextOnDutyArbitrator(0, 6)
	assert.Equal(t, arbitrators[1], currentArbitrator)
}

func TestBlockVersionDone(t *testing.T) {
	blockchain.DefaultLedger = originLedger
}
