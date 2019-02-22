package blockchain

import (
	"bytes"
	"encoding/hex"
	"testing"

	"github.com/elastos/Elastos.ELA/blockchain/mock"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/dpos/state"
	"github.com/stretchr/testify/assert"
)

const (
	TestBlockHex = "000000007b3a8b2032301d0f9fafadee3bddba8d798a3ce1ed1574063ae3bb55628cec763a45dffe0f38d9efb5" +
		"0a41dbe6b7f4186ba9b4861ad624fdde6e1e775a81b0d3687f4c5add01561d000000001027000001000000010000000000000" +
		"000000000000000000000000000000000000000000000000000000000002cfabe6d6d6d126217acca4ed3b3aa40de6d1dad67" +
		"61a7bba4ebdb67c88714455cea580084010000000000000000000000000000000000000000000000000000000000000000000" +
		"0000000000000000000000000000000000000000000000000ffffff7f00000000000000000000000000000000000000000000" +
		"000000000000000000009fba1be4874f22da581831eb1a5243e53b51e57f3021222943a6a2919d19c19d687f4c5a000000001" +
		"28c95000102000000000403454c4101000847cfc35085f3aec001000000000000000000000000000000000000000000000000" +
		"0000000000000000ffffffffffff02b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3b54afb0" +
		"80000000000000000129e9cf1c5f336fcf3a6c954444ed482c5d916e506b037db964a231458d2d6ffd5ea18944c4f90e63d54" +
		"7c5d3b9874df66a4ead0a3a803f5140000000000000000129e9cf1c5f336fcf3a6c954444ed482c5d916e5061027000000020" +
		"000016c3a8d6db4d3b4ccad1712a29c5e90e2e7bc26c603995fc18a37c85a5420ad445600ffffffff02b037db964a231458d2" +
		"d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3047823a7170100000000000021190ff3b12919c17f232db55431832" +
		"2a6b43ba372b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a300b864d9450000000000000021" +
		"fa402bfaecabefacb6379c08edb5224fd95e25f700000000014140c72db63b7fdf90b8bf34e91f0a6394e25d1340f178a1776" +
		"bdc344fecf8ced8e4db627fb9ffa7068c51d3d15b92a749ffa407e2593833ec836d4cdaae1062abe52321035e1529938d1a36" +
		"bef97806557bdb4faec8c83a8fc557c1afb287b07bd923c589ac"
)

func TestCheckBlockSanity(t *testing.T) {
	config.Parameters = config.ConfigParams{Configuration: &config.Template}
	log.NewDefault(
		config.Parameters.PrintLevel,
		config.Parameters.MaxPerLogSize,
		config.Parameters.MaxLogsSize,
	)
	foundation, err := common.Uint168FromAddress("8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta")
	if !assert.NoError(t, err) {
		return
	}
	FoundationAddress = *foundation
	chainStore, err := NewChainStore("Chain_UnitTest", config.MainNetParams.GenesisBlock)
	if err != nil {
		t.Error(err.Error())
	}
	defer chainStore.Close()

	heightVersions := &mock.HeightVersionsMock{}
	chain, _ := New(chainStore, &config.MainNetParams, heightVersions, state.NewState(nil, &config.MainNetParams))
	if DefaultLedger == nil {
		DefaultLedger = &Ledger{
			Blockchain:     chain,
			HeightVersions: heightVersions,
			Store:          chainStore,
		}
	}

	if err != nil {
		t.Error(err.Error())
	}

	timeSource := NewMedianTime()
	blockData, err := hex.DecodeString(TestBlockHex)
	if err != nil {
		t.Errorf("Decode block hex error %s", err.Error())
	}

	var block types.Block
	block.Deserialize(bytes.NewReader(blockData))
	err = chain.CheckBlockSanity(&block)
	if err != nil {
		t.Error(err.Error())
	}

	// change of time stamp, this will change the block hash
	// and the proof check would fail
	block.Timestamp = uint32(timeSource.AdjustedTime().Unix())
	err = chain.CheckBlockSanity(&block)
	assert.Error(t, err, "[Error] block passed check with invalid hash")
	assert.EqualError(t, err, "[PowCheckBlockSanity] block check aux pow failed")
}
